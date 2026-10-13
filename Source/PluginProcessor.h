#pragma once

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <JuceHeader.h>
#include <atomic>
#include <array>
#include <deque>
#include "PresetManager.h"

//==============================================================================
// EchoForge
// Stereo delay with Simple / BBD / Tape / Diffuse engines.
// Primary controls: CENTER, WIDTH, HEIGHT, BUFFER.
// AlphaAudio by William Ashley Music
//==============================================================================

//==============================================================================
// Enumerations
//==============================================================================
enum class DelayEngine
{
    Simple = 0,   // Clean digital delay
    BBD,          // Bucket-brigade delay
    Tape,         // Tape echo with wow/flutter
    Diffuse       // Short diffused multi-tap smear
};

enum class PingPongMode
{
    Off = 0,      // L and R independent
    LToR,         // Feedback crosses L -> R
    RToL,         // Feedback crosses R -> L
    Both          // Full cross-feedback
};

enum class ModShape
{
    Sine = 0,
    Triangle,
    Saw,
    RandomHold
};

//==============================================================================
// DSP building blocks
//==============================================================================
namespace EchoDSP
{
    //==========================================================================
    // Buffer Manager
    //
    // Handles the CENTER / BUFFER abstraction. Allocates at the max size once,
    // then exposes a normalised read position that maps [0..1] to the active
    // portion of the buffer. The active portion is defined by the BUFFER knob,
    // so a small BUFFER means fewer samples touched and better cache locality.
    //
    // The buffer is a flat circular array. Delay lines read from it via this
    // class's helper rather than poking directly, so the "use only the active
    // portion" logic lives in exactly one place.
    //==========================================================================
    class DelayBuffer
    {
    public:
        // maxSeconds is the absolute ceiling. 2 seconds is plenty for a delay.
        void prepare(double sr, float maxSeconds = 2.0f)
        {
            sampleRate = sr;
            maxSamples = static_cast<int>(sr * maxSeconds) + 4;
            // Round up to a power of two so we can use a mask instead of a
            // modulo for the wrap-around — this is the single biggest CPU
            // win in a delay line.
            int pow2 = 1;
            while (pow2 < maxSamples) pow2 <<= 1;
            mask = pow2 - 1;
            buf.assign(static_cast<size_t>(pow2), 0.f);
            writePos = 0;
            activeSamples = maxSamples;
            updateScale();
        }

        void reset()
        {
            std::fill(buf.begin(), buf.end(), 0.f);
            writePos = 0;
        }

        // bufferPercent: 0..1, where 1 = full 2 seconds active, 0 = 250 ms
        // minimum. The engine still holds 2 seconds of memory; only the active
        // portion is ever touched.
        void setActiveBuffer(float bufferPercent)
        {
            const float minSeconds = 0.25f;
            const float maxSeconds = static_cast<float>(maxSamples)
                / static_cast<float>(sampleRate);
            const float secs = minSeconds + juce::jlimit(0.f, 1.f, bufferPercent)
                * (maxSeconds - minSeconds);
            activeSamples = juce::jlimit(16, maxSamples,
                static_cast<int>(secs * static_cast<float>(sampleRate)));
            updateScale();
        }

        // Write a sample at the current write position and advance.
        inline void write(float sample) noexcept
        {
            buf[static_cast<size_t>(writePos) & mask] = sample;
            writePos = (writePos + 1) & mask;
        }

        // Read at a normalised delay position. pos01 = 0 is "just written",
        // pos01 = 1 is "activeSamples back in the buffer". Interpolates
        // between adjacent samples for smooth modulation.
        inline float readNormalised(float pos01) const noexcept
        {
            const float clamped = juce::jlimit(0.f, 1.f, pos01);
            const float delaySamples = clamped * static_cast<float>(activeSamples - 2);

            float rp = static_cast<float>(writePos) - delaySamples;
            if (rp < 0.f) rp += static_cast<float>(mask + 1);

            const int i0 = static_cast<int>(rp);
            const float frac = rp - static_cast<float>(i0);
            const float s0 = buf[static_cast<size_t>(i0) & mask];
            const float s1 = buf[static_cast<size_t>(i0 + 1) & mask];
            return s0 + (s1 - s0) * frac;
        }

        int   getActiveSamples() const noexcept { return activeSamples; }
        int   getMaxSamples()    const noexcept { return maxSamples; }
        float getSampleRate()    const noexcept { return static_cast<float>(sampleRate); }

    private:
        void updateScale() {}

        std::vector<float> buf;
        int writePos = 0;
        int mask = 0;
        int maxSamples = 0;
        int activeSamples = 0;
        double sampleRate = 44100.0;
    };

    //==========================================================================
    // BBD bucket — a single bucket-brigade stage with charge loss and allpass
    // character.
    //==========================================================================
    struct BBDBucket
    {
        float charge = 0.f;
        float preEmphasisState = 0.f;
        float apX = 0.f;
        float apY = 0.f;
        float leakState = 0.f;
    };

    //==========================================================================
    // BBD line
    //
    // Uses a deque of buckets with per-bucket state. Reads a fractional bucket
    // position for interpolated taps. The bucket state is only updated for
    // buckets in the active range.
    //==========================================================================
    class BBDLine
    {
    public:
        void prepare(double sr, int maxSamples)
        {
            sampleRate = sr;
            buckets.clear();
            buckets.resize(static_cast<size_t>(maxSamples), 0.f);
            preEmphasis.assign(static_cast<size_t>(maxSamples), 0.f);
            allpassX.assign(static_cast<size_t>(maxSamples), 0.f);
            allpassY.assign(static_cast<size_t>(maxSamples), 0.f);
            leakState.assign(static_cast<size_t>(maxSamples), 0.f);
            writePos = 0;
            activeSamples = maxSamples;
        }

        void reset()
        {
            std::fill(buckets.begin(), buckets.end(), 0.f);
            std::fill(preEmphasis.begin(), preEmphasis.end(), 0.f);
            std::fill(allpassX.begin(), allpassX.end(), 0.f);
            std::fill(allpassY.begin(), allpassY.end(), 0.f);
            std::fill(leakState.begin(), leakState.end(), 0.f);
            writePos = 0;
        }

        void setActiveSamples(int n) { activeSamples = juce::jlimit(4, static_cast<int>(buckets.size()), n); }

        // Process one sample. pos01 = normalised delay position (0..1).
        // drive, chargeLoss, leak, allpassFreqHz all 0..1 or Hz.
        float process(float input, float pos01,
            float drive, float chargeLoss, float leak, float allpassFreqHz)
        {
            const int N = static_cast<int>(buckets.size());
            const float delaySamples = juce::jlimit(0.f, 1.f, pos01)
                * static_cast<float>(activeSamples - 2);

            // Pre-emphasis (record side): boost highs before the bucket line
            // so the de-emphasis on the playback side can undo it cleanly.
            const float preEmphAmt = 0.3f;
            const float emphasized = input + preEmphAmt * (input - preEmphasis[(size_t)writePos]);
            preEmphasis[(size_t)writePos] = input;

            // Saturation in the bucket line
            const float drv = 1.f + drive * 4.f;
            float toWrite = std::tanh(emphasized * drv) / drv;

            // Charge loss: high amplitudes lose more charge per shift
            const float loss = chargeLoss * (0.3f + 0.3f * std::abs(toWrite));
            toWrite *= (1.f - juce::jlimit(0.f, 0.5f, loss));

            // Leak noise on this bucket
            if (leak > 0.0005f)
            {
                leakState[(size_t)writePos] = leakState[(size_t)writePos] * 0.98f
                    + nextNoise() * leak * 0.02f;
                toWrite += leakState[(size_t)writePos];
            }

            // Store the new charge
            buckets[(size_t)writePos] = toWrite;

            // Read from the interpolated position
            float rp = static_cast<float>(writePos) - delaySamples;
            while (rp < 0.f) rp += static_cast<float>(N);
            const int i0 = static_cast<int>(rp) % N;
            const int i1 = (i0 + 1) % N;
            const float frac = rp - std::floor(rp);

            const float s0 = buckets[(size_t)i0];
            const float s1 = buckets[(size_t)i1];
            float wet = s0 + (s1 - s0) * frac;

            // Allpass character on the playback side
            if (allpassFreqHz > 0.f)
            {
                const float a = (1.f - allpassFreqHz / (float)sampleRate)
                    / (1.f + allpassFreqHz / (float)sampleRate);
                const float aClamped = juce::jlimit(-0.9f, 0.9f, a);
                const int readIdx = i0;
                const float apOut = allpassX[(size_t)readIdx]
                    + aClamped * (wet - allpassY[(size_t)readIdx]);
                allpassY[(size_t)readIdx] = apOut;
                allpassX[(size_t)readIdx] = wet;
                wet = apOut;
            }

            writePos = (writePos + 1) % N;
            return wet;
        }

    private:
        float nextNoise()
        {
            noiseSeed = noiseSeed * 1664525u + 1013904223u;
            return static_cast<float>(static_cast<int32_t>(noiseSeed))
                * (1.f / 2147483648.f);
        }

        std::vector<float> buckets;
        std::vector<float> preEmphasis;
        std::vector<float> allpassX;
        std::vector<float> allpassY;
        std::vector<float> leakState;
        int writePos = 0;
        int activeSamples = 0;
        double sampleRate = 44100.0;
        uint32_t noiseSeed = 0x9E3779B9u;
    };

    //==========================================================================
    // Tape line
    //==========================================================================
    class TapeLine
    {
    public:
        void prepare(double sr, int maxSamples)
        {
            sampleRate = sr;
            int pow2 = 1;
            while (pow2 < maxSamples) pow2 <<= 1;
            mask = pow2 - 1;
            buffer.assign(static_cast<size_t>(pow2), 0.f);
            writePos = 0;
            activeSamples = maxSamples;
            wowPhase = flutterPhase = 0.f;
            satState = 0.f;
        }

        void reset()
        {
            std::fill(buffer.begin(), buffer.end(), 0.f);
            writePos = 0;
            wowPhase = flutterPhase = 0.f;
            satState = 0.f;
        }

        void setActiveSamples(int n) { activeSamples = juce::jlimit(4, static_cast<int>(buffer.size()), n); }

        float process(float input, float pos01, float wowAmt, float flutterAmt,
            float saturation)
        {
            // Saturation in the tape pre-amp
            float preSaturated = input;
            if (saturation > 0.001f)
            {
                const float drv = 1.f + saturation * 3.f;
                satState += 0.05f * (std::tanh(input * drv) / drv - satState);
                preSaturated = satState * saturation + input * (1.f - saturation);
            }

            buffer[static_cast<size_t>(writePos) & mask] = preSaturated;
            writePos = (writePos + 1) & mask;

            // Wow/flutter modulates the read position
            wowPhase += 0.7f / (float)sampleRate;
            if (wowPhase >= 1.f) wowPhase -= 1.f;
            flutterPhase += 6.5f / (float)sampleRate;
            if (flutterPhase >= 1.f) flutterPhase -= 1.f;

            const float wow = std::sin(wowPhase * juce::MathConstants<float>::twoPi);
            const float flutter = std::sin(flutterPhase * juce::MathConstants<float>::twoPi);

            const float baseSamples = juce::jlimit(0.f, 1.f, pos01)
                * static_cast<float>(activeSamples - 2);
            const float modSamples = (wow * wowAmt * 0.02f
                + flutter * flutterAmt * 0.005f)
                * static_cast<float>(activeSamples);

            float rp = static_cast<float>(writePos) - baseSamples - modSamples;
            while (rp < 0.f) rp += static_cast<float>(mask + 1);
            const int i0 = static_cast<int>(rp) & mask;
            const int i1 = (i0 + 1) & mask;
            const float frac = rp - std::floor(rp);
            const float s0 = buffer[(size_t)i0];
            const float s1 = buffer[(size_t)i1];
            return s0 + (s1 - s0) * frac;
        }

    private:
        std::vector<float> buffer;
        int mask = 0;
        int writePos = 0;
        int activeSamples = 0;
        double sampleRate = 44100.0;
        float wowPhase = 0.f;
        float flutterPhase = 0.f;
        float satState = 0.f;
    };

    //==========================================================================
    // Diffuse line
    //==========================================================================
    class DiffuseLine
    {
    public:
        void prepare(double sr, int maxSamples)
        {
            sampleRate = sr;
            int pow2 = 1;
            while (pow2 < maxSamples) pow2 <<= 1;
            mask = pow2 - 1;
            buffer.assign(static_cast<size_t>(pow2), 0.f);
            writePos = 0;
            activeSamples = maxSamples;
            tapDampState.fill(0.f);
        }

        void reset()
        {
            std::fill(buffer.begin(), buffer.end(), 0.f);
            writePos = 0;
            tapDampState.fill(0.f);
        }

        void setActiveSamples(int n) { activeSamples = juce::jlimit(4, static_cast<int>(buffer.size()), n); }

        float process(float input, float pos01, float spread01, float damping,
            int numTaps)
        {
            buffer[static_cast<size_t>(writePos) & mask] = input;
            writePos = (writePos + 1) & mask;

            numTaps = juce::jlimit(2, 8, numTaps);
            const float baseSamples = juce::jlimit(0.f, 1.f, pos01)
                * static_cast<float>(activeSamples - 2);
            const float spreadSamples = spread01 * baseSamples * 0.5f;

            float out = 0.f;
            for (int t = 0; t < numTaps; ++t)
            {
                const float frac = (numTaps > 1)
                    ? static_cast<float>(t) / static_cast<float>(numTaps - 1)
                    : 0.f;
                const float tapOffset = (frac - 0.5f) * spreadSamples;
                const float tapDelay = juce::jlimit(1.f,
                    static_cast<float>(activeSamples - 2),
                    baseSamples + tapOffset);

                float rp = static_cast<float>(writePos) - tapDelay;
                while (rp < 0.f) rp += static_cast<float>(mask + 1);
                const int i0 = static_cast<int>(rp) & mask;
                const int i1 = (i0 + 1) & mask;
                const float f = rp - std::floor(rp);
                float tapVal = buffer[(size_t)i0] + (buffer[(size_t)i1] - buffer[(size_t)i0]) * f;

                // One-pole damping on each tap
                tapDampState[(size_t)t] += damping * 0.3f * (tapVal - tapDampState[(size_t)t]);
                out += tapDampState[(size_t)t];
            }

            return out / static_cast<float>(numTaps);
        }

    private:
        std::vector<float> buffer;
        int mask = 0;
        int writePos = 0;
        int activeSamples = 0;
        double sampleRate = 44100.0;
        std::array<float, 8> tapDampState{};
    };

    //==========================================================================
    // Simple line — clean, just interpolated reads from the delay buffer.
    //==========================================================================
    class SimpleLine
    {
    public:
        void prepare(DelayBuffer* sharedBuffer) { buf = sharedBuffer; }
        void reset() {}

        float process(float input, float pos01)
        {
            if (!buf) return input;
            buf->write(input);
            return buf->readNormalised(pos01);
        }

    private:
        DelayBuffer* buf = nullptr;
    };

    //==========================================================================
    // Modulation LFO with four shapes
    //==========================================================================
    struct ModLFO
    {
        void prepare(double sr)
        {
            sampleRate = sr;
            phase = 0.f;
            holdValue = 0.f;
            rng.setSeed(0xC0FFEE);
        }

        float tick(float rateHz, int shape)
        {
            float out = 0.f;
            switch (shape)
            {
            case 0: out = std::sin(phase * juce::MathConstants<float>::twoPi); break;
            case 1: out = (phase < 0.5f) ? (phase * 4.f - 1.f) : (3.f - phase * 4.f); break;
            case 2: out = 1.f - 2.f * phase; break;
            case 3:
                if (phase < 1.f / 64.f) holdValue = rng.nextFloat() * 2.f - 1.f;
                out = holdValue;
                break;
            default: break;
            }
            phase += rateHz / (float)sampleRate;
            if (phase >= 1.f) phase -= 1.f;
            return out;
        }

        void reset() { phase = 0.f; }

        double sampleRate = 44100.0;
        float phase = 0.f;
        float holdValue = 0.f;
        juce::Random rng;
    };

    //==========================================================================
    // One-pole filters — used for the HEIGHT tone band and head bump
    //==========================================================================
    struct OnePoleHP
    {
        float z = 0.f;
        float coeff = 0.f;
        float lastHz = -1.f;
        float sampleRate = 44100.f;

        void prepare(double sr) { sampleRate = (float)sr; z = 0.f; lastHz = -1.f; }
        void reset() { z = 0.f; }

        void setCutoff(float hz)
        {
            if (hz == lastHz) return;
            lastHz = hz;
            const float fc = std::min(hz, sampleRate * 0.45f);
            coeff = 1.f - std::exp(-2.f * (float)M_PI * fc / sampleRate);
        }

        float process(float in)
        {
            z += coeff * (in - z);
            if (std::abs(z) < 1e-20f) z = 0.f;
            return in - z;
        }
    };

    struct OnePoleLP
    {
        float z = 0.f;
        float coeff = 0.f;
        float lastHz = -1.f;
        float sampleRate = 44100.f;

        void prepare(double sr) { sampleRate = (float)sr; z = 0.f; lastHz = -1.f; }
        void reset() { z = 0.f; }

        void setCutoff(float hz)
        {
            if (hz == lastHz) return;
            lastHz = hz;
            const float fc = std::min(hz, sampleRate * 0.45f);
            coeff = 1.f - std::exp(-2.f * (float)M_PI * fc / sampleRate);
        }

        float process(float in)
        {
            z += coeff * (in - z);
            if (std::abs(z) < 1e-20f) z = 0.f;
            return z;
        }
    };

    //==========================================================================
    // Feedback safety limiter
    //==========================================================================
    inline float feedbackLimiter(float x, float threshold)
    {
        return std::tanh(x / threshold) * threshold;
    }

} // namespace EchoDSP

//==============================================================================
// Parameter IDs
//==============================================================================
namespace EchoParams
{
    static constexpr const char* ENGINE = "engine";
    static constexpr const char* BYPASS = "bypass";

    // --- The four primary controls ---
    static constexpr const char* CENTER = "center";         // 0..1, delay origin
    static constexpr const char* WIDTH = "width";           // 0..1, stereo spread
    static constexpr const char* HEIGHT = "height";         // 0..1, band width
    static constexpr const char* BUFFER = "buffer";         // 0..1, active buffer

    // --- Secondary delay-time controls ---
    static constexpr const char* TIME_TRIM = "time_trim";   // -1..+1, seconds offset
    static constexpr const char* SYNC = "sync";             // bool
    static constexpr const char* SYNC_DIV = "sync_div";     // choice
    static constexpr const char* TAP_TEMPO = "tap_tempo";   // momentary

    // --- Feedback ---
    static constexpr const char* FEEDBACK = "feedback";     // 0..0.98
    static constexpr const char* PING_PONG = "ping_pong";   // PingPongMode
    static constexpr const char* CROSS_FEED = "cross_feed"; // 0..1

    // --- HEIGHT trim ---
    static constexpr const char* HEIGHT_TRIM = "height_trim"; // 0..1
    static constexpr const char* HP_TRIM = "hp_trim";       // 20..2000 Hz
    static constexpr const char* LP_TRIM = "lp_trim";       // 200..20000 Hz

    // --- Modulation ---
    static constexpr const char* MOD_RATE = "mod_rate";     // 0.01..10 Hz
    static constexpr const char* MOD_DEPTH = "mod_depth";   // 0..1
    static constexpr const char* MOD_SHAPE = "mod_shape";   // ModShape

    // --- BBD engine ---
    static constexpr const char* BBD_DRIVE = "bbd_drive";
    static constexpr const char* BBD_CHARGE = "bbd_charge";
    static constexpr const char* BBD_LEAK = "bbd_leak";
    static constexpr const char* BBD_STUTTER = "bbd_stutter";

    // --- Tape engine ---
    static constexpr const char* TAPE_WOW = "tape_wow";
    static constexpr const char* TAPE_FLUTTER = "tape_flutter";
    static constexpr const char* TAPE_SAT = "tape_sat";
    static constexpr const char* TAPE_HEAD = "tape_head";

    // --- Diffuse engine ---
    static constexpr const char* DIFF_SPREAD = "diff_spread";
    static constexpr const char* DIFF_TAPS = "diff_taps";
    static constexpr const char* DIFF_DAMP = "diff_damp";

    // --- Output ---
    static constexpr const char* MIX = "mix";
    static constexpr const char* OUTPUT = "output";
    static constexpr const char* OUT_WIDTH = "out_width";   // 0..2 output stereo width

    // Sync-division names (index matches position)
    inline juce::StringArray syncDivisionNames()
    {
        return { "1/16", "1/8T", "1/8", "1/4T", "1/4",
                 "1/2", "1/1", "2/1" };
    }
}

//==============================================================================
// Processor
//==============================================================================
class EchoForgeProcessor : public juce::AudioProcessor,
    public juce::AudioProcessorValueTreeState::Listener
{
public:
    EchoForgeProcessor();
    ~EchoForgeProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "EchoForge"; }
    bool acceptsMidi()  const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 8.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void parameterChanged(const juce::String& paramID, float newValue) override;

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    PresetManager& getPresetManager() { return *presetManager; }
    const PresetManager& getPresetManager() const { return *presetManager; }

    void loadPreviousPreset();
    void loadNextPreset();
    void loadPresetByIndex(int index);
    int  getCurrentPresetIndex() const { return currentPresetIndex; }

    std::function<void(int)> onPresetChanged;

    void captureAState();
    void swapAB();
    bool hasAState() const { return aStateValid; }

    // Output meters
    std::atomic<float> levelPeakL{ 0.0f };
    std::atomic<float> levelPeakR{ 0.0f };
    std::atomic<float> levelInRmsL{ 0.0f };
    std::atomic<float> levelInRmsR{ 0.0f };
    std::atomic<float> feedbackAmount{ 0.0f };

    //==========================================================================
    // Live snapshot for editor display — relaxed ordering, display-only
    //==========================================================================
    struct Snapshot
    {
        std::atomic<int>   engine{ 0 };
        std::atomic<float> center{ 0.3f };
        std::atomic<float> width{ 0.5f };
        std::atomic<float> height{ 0.7f };
        std::atomic<float> buffer{ 1.f };
        std::atomic<float> timeTrim{ 0.f };
        std::atomic<float> feedback{ 0.3f };
        std::atomic<int>   pingPong{ 0 };
        std::atomic<float> modDepth{ 0.f };
        std::atomic<float> modRate{ 0.5f };
        std::atomic<int>   modShape{ 0 };
        std::atomic<float> hpFreq{ 20.f };
        std::atomic<float> lpFreq{ 20000.f };
        std::atomic<bool>  synced{ false };

        // Live modulation value (-1..1)
        std::atomic<float> modValue{ 0.f };

        // Live delay times in ms (post-CENTER + TRIM + MOD)
        std::atomic<float> liveDelayL{ 300.f };
        std::atomic<float> liveDelayR{ 300.f };

        // Active buffer size in samples and ms
        std::atomic<int>   activeSamples{ 0 };
        std::atomic<float> activeBufferMs{ 2000.f };

        std::atomic<float> sampleRate{ 44100.f };
    };
    Snapshot snapshot;

private:
    double currentSampleRate = 44100.0;
    int    currentBlockSize = 512;
    bool   isPrepared = false;

    // Maximum delay in samples at the current sample rate
    int maxDelaySamples = 0;

    // --- Delay buffers (two independent ones, L and R) ---
    // The SimpleLine shares buffers with BBD/Tape/Diffuse? No — each engine
    // keeps its own buffer so switching engines doesn't inherit stale audio.
    EchoDSP::DelayBuffer simpleBufL, simpleBufR;
    EchoDSP::SimpleLine  simpleL, simpleR;

    EchoDSP::BBDLine     bbdL, bbdR;
    EchoDSP::TapeLine    tapeL, tapeR;
    EchoDSP::DiffuseLine diffL, diffR;

    // Modulation LFO (shared by all engines)
    EchoDSP::ModLFO modLfo;

    // Tone filters — HEIGHT band (HP + LP cascade) plus trims
    EchoDSP::OnePoleHP heightHpL, heightHpR;
    EchoDSP::OnePoleLP heightLpL, heightLpR;
    EchoDSP::OnePoleHP hpTrimL, hpTrimR;
    EchoDSP::OnePoleLP lpTrimL, lpTrimR;

    // Tape head bump — a gentle low-shelf (implemented via LP subtraction)
    EchoDSP::OnePoleLP headBumpL, headBumpR;

    // Feedback state per channel
    float fbStateL = 0.f;
    float fbStateR = 0.f;

    // BBD clock boundary tracker (for the stutter injector)
    float bbdClockPhase = 0.f;

    // Tap tempo
    double lastTapTime = 0.0;
    float  measuredTempoMs = 500.f;

    // Cached parameter pointers
    std::atomic<float>* pEngine = nullptr;
    std::atomic<float>* pCenter = nullptr;
    std::atomic<float>* pWidth = nullptr;
    std::atomic<float>* pHeight = nullptr;
    std::atomic<float>* pBuffer = nullptr;
    std::atomic<float>* pTimeTrim = nullptr;
    std::atomic<float>* pSync = nullptr;
    std::atomic<float>* pSyncDiv = nullptr;
    std::atomic<float>* pFeedback = nullptr;
    std::atomic<float>* pPingPong = nullptr;
    std::atomic<float>* pCrossFeed = nullptr;
    std::atomic<float>* pHeightTrim = nullptr;
    std::atomic<float>* pHpTrim = nullptr;
    std::atomic<float>* pLpTrim = nullptr;
    std::atomic<float>* pModRate = nullptr;
    std::atomic<float>* pModDepth = nullptr;
    std::atomic<float>* pModShape = nullptr;
    std::atomic<float>* pBbdDrive = nullptr;
    std::atomic<float>* pBbdCharge = nullptr;
    std::atomic<float>* pBbdLeak = nullptr;
    std::atomic<float>* pBbdStutter = nullptr;
    std::atomic<float>* pTapeWow = nullptr;
    std::atomic<float>* pTapeFlutter = nullptr;
    std::atomic<float>* pTapeSat = nullptr;
    std::atomic<float>* pTapeHead = nullptr;
    std::atomic<float>* pDiffSpread = nullptr;
    std::atomic<float>* pDiffTaps = nullptr;
    std::atomic<float>* pDiffDamp = nullptr;
    std::atomic<float>* pMix = nullptr;
    std::atomic<float>* pOutput = nullptr;
    std::atomic<float>* pOutWidth = nullptr;

    // Smoothed values
    juce::SmoothedValue<float> smoothedOutput;
    juce::SmoothedValue<float> smoothedMix;

    // A/B snapshot
    juce::ValueTree aState;
    bool aStateValid = false;

    std::unique_ptr<PresetManager> presetManager;
    int currentPresetIndex = -1;

    void updateSnapshot();

    // Helpers — compute the effective delay time and filter frequencies from
    // the CENTER / TIME_TRIM / SYNC and HEIGHT params.
    float computeDelayMs(float center01, float timeTrimSec, bool syncOn,
        int syncDiv, float bpm) const;
    float computeHeightCenterHz(float center01) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EchoForgeProcessor)
};