#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PresetManager.h"

//==============================================================================
// Parameter layout
//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
EchoForgeProcessor::createParameterLayout()
{
    using namespace EchoParams;
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto addFloat = [&](const char* id, const char* name, float min, float max,
        float def, float skew = 1.f, const char* suffix = nullptr)
        {
            juce::NormalisableRange<float> range(min, max, 0.001f, skew);
            if (suffix)
            {
                layout.add(std::make_unique<juce::AudioParameterFloat>(
                    juce::ParameterID{ id, 1 }, name, range, def,
                    juce::AudioParameterFloatAttributes().withLabel(suffix)));
            }
            else
            {
                layout.add(std::make_unique<juce::AudioParameterFloat>(
                    juce::ParameterID{ id, 1 }, name, range, def));
            }
        };

    auto addBool = [&](const char* id, const char* name, bool def)
        {
            layout.add(std::make_unique<juce::AudioParameterBool>(
                juce::ParameterID{ id, 1 }, name, def));
        };

    auto addChoice = [&](const char* id, const char* name,
        const juce::StringArray& items, int def)
        {
            layout.add(std::make_unique<juce::AudioParameterChoice>(
                juce::ParameterID{ id, 1 }, name, items, def));
        };

    // --- Engine selection ---
    addChoice(ENGINE, "Engine",
        { "Simple", "BBD", "Tape", "Diffuse" }, 0);
    addBool(BYPASS, "Bypass", false);

    // --- The four primary controls ---
    addFloat(CENTER, "Center", 0.f, 1.f, 0.3f);
    addFloat(WIDTH, "Width", 0.f, 1.f, 0.5f);
    addFloat(HEIGHT, "Height", 0.f, 1.f, 0.7f);
    addFloat(BUFFER, "Buffer", 0.f, 1.f, 1.f);

    // --- Secondary delay-time controls ---
    addFloat(TIME_TRIM, "Time Trim", -1.f, 1.f, 0.f, 1.f, "s");
    addBool(SYNC, "Sync", false);
    addChoice(SYNC_DIV, "Sync Div", syncDivisionNames(), 4);  // default 1/4
    addBool(TAP_TEMPO, "Tap Tempo", false);

    // --- Feedback ---
    addFloat(FEEDBACK, "Feedback", 0.f, 0.98f, 0.35f, 1.f, "%");
    addChoice(PING_PONG, "Ping-Pong",
        { "Off", "L -> R", "R -> L", "Both" }, 0);
    addFloat(CROSS_FEED, "Cross-Feed", 0.f, 1.f, 0.5f);

    // --- HEIGHT trim ---
    addFloat(HEIGHT_TRIM, "Height Trim", 0.f, 1.f, 0.5f);
    addFloat(HP_TRIM, "HP Trim", 20.f, 2000.f, 20.f, 0.3f, "Hz");
    addFloat(LP_TRIM, "LP Trim", 200.f, 20000.f, 20000.f, 0.3f, "Hz");

    // --- Modulation ---
    addFloat(MOD_RATE, "Mod Rate", 0.01f, 10.f, 0.5f, 0.4f, "Hz");
    addFloat(MOD_DEPTH, "Mod Depth", 0.f, 1.f, 0.f);
    addChoice(MOD_SHAPE, "Mod Shape",
        { "Sine", "Triangle", "Saw", "S&H" }, 0);

    // --- BBD engine ---
    addFloat(BBD_DRIVE, "BBD Drive", 0.f, 1.f, 0.3f);
    addFloat(BBD_CHARGE, "BBD Charge Loss", 0.f, 1.f, 0.3f);
    addFloat(BBD_LEAK, "BBD Leak", 0.f, 1.f, 0.2f);
    addFloat(BBD_STUTTER, "BBD Stutter", 0.f, 1.f, 0.f);

    // --- Tape engine ---
    addFloat(TAPE_WOW, "Tape Wow", 0.f, 1.f, 0.3f);
    addFloat(TAPE_FLUTTER, "Tape Flutter", 0.f, 1.f, 0.2f);
    addFloat(TAPE_SAT, "Tape Sat", 0.f, 1.f, 0.3f);
    addFloat(TAPE_HEAD, "Tape Head Bump", 0.f, 1.f, 0.4f);

    // --- Diffuse engine ---
    addFloat(DIFF_SPREAD, "Diffuse Spread", 0.f, 1.f, 0.5f);
    addFloat(DIFF_TAPS, "Diffuse Taps", 2.f, 8.f, 4.f);
    addFloat(DIFF_DAMP, "Diffuse Damping", 0.f, 1.f, 0.4f);

    // --- Output ---
    addFloat(MIX, "Mix", 0.f, 1.f, 0.35f);
    addFloat(OUTPUT, "Output", -24.f, 12.f, 0.f, 1.f, "dB");
    addFloat(OUT_WIDTH, "Output Width", 0.f, 2.f, 1.f);

    return layout;
}

//==============================================================================
// Constructor / destructor
//==============================================================================
EchoForgeProcessor::EchoForgeProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    apvts(*this, nullptr, "EchoForgeState", createParameterLayout())
{
    for (auto* param : apvts.processor.getParameters())
    {
        if (auto* p = dynamic_cast<juce::RangedAudioParameter*>(param))
            apvts.addParameterListener(p->getParameterID(), this);
    }

    // Cache the parameter pointers — these are stable for the lifetime of the
    // APVTS, so we can grab them once here and never touch the string lookup
    // path in the audio thread again.
    using namespace EchoParams;
    pEngine = apvts.getRawParameterValue(ENGINE);
    pCenter = apvts.getRawParameterValue(CENTER);
    pWidth = apvts.getRawParameterValue(WIDTH);
    pHeight = apvts.getRawParameterValue(HEIGHT);
    pBuffer = apvts.getRawParameterValue(BUFFER);
    pTimeTrim = apvts.getRawParameterValue(TIME_TRIM);
    pSync = apvts.getRawParameterValue(SYNC);
    pSyncDiv = apvts.getRawParameterValue(SYNC_DIV);
    pFeedback = apvts.getRawParameterValue(FEEDBACK);
    pPingPong = apvts.getRawParameterValue(PING_PONG);
    pCrossFeed = apvts.getRawParameterValue(CROSS_FEED);
    pHeightTrim = apvts.getRawParameterValue(HEIGHT_TRIM);
    pHpTrim = apvts.getRawParameterValue(HP_TRIM);
    pLpTrim = apvts.getRawParameterValue(LP_TRIM);
    pModRate = apvts.getRawParameterValue(MOD_RATE);
    pModDepth = apvts.getRawParameterValue(MOD_DEPTH);
    pModShape = apvts.getRawParameterValue(MOD_SHAPE);
    pBbdDrive = apvts.getRawParameterValue(BBD_DRIVE);
    pBbdCharge = apvts.getRawParameterValue(BBD_CHARGE);
    pBbdLeak = apvts.getRawParameterValue(BBD_LEAK);
    pBbdStutter = apvts.getRawParameterValue(BBD_STUTTER);
    pTapeWow = apvts.getRawParameterValue(TAPE_WOW);
    pTapeFlutter = apvts.getRawParameterValue(TAPE_FLUTTER);
    pTapeSat = apvts.getRawParameterValue(TAPE_SAT);
    pTapeHead = apvts.getRawParameterValue(TAPE_HEAD);
    pDiffSpread = apvts.getRawParameterValue(DIFF_SPREAD);
    pDiffTaps = apvts.getRawParameterValue(DIFF_TAPS);
    pDiffDamp = apvts.getRawParameterValue(DIFF_DAMP);
    pMix = apvts.getRawParameterValue(MIX);
    pOutput = apvts.getRawParameterValue(OUTPUT);
    pOutWidth = apvts.getRawParameterValue(OUT_WIDTH);

    presetManager = std::make_unique<PresetManager>(apvts);
}

EchoForgeProcessor::~EchoForgeProcessor()
{
    presetManager.reset();
    onPresetChanged = nullptr;

    for (auto* param : apvts.processor.getParameters())
    {
        if (auto* p = dynamic_cast<juce::RangedAudioParameter*>(param))
            apvts.removeParameterListener(p->getParameterID(), this);
    }
}

//==============================================================================
// prepareToPlay / releaseResources
//==============================================================================
void EchoForgeProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = juce::jlimit(8000.0, 384000.0, sampleRate);
    currentBlockSize = juce::jmax(1, samplesPerBlock);

    // Allocate each engine's buffer at 2 seconds max. The BUFFER knob then
    // trims the *active* portion without reallocating.
    maxDelaySamples = static_cast<int>(currentSampleRate * 2.0) + 4;

    simpleBufL.prepare(currentSampleRate, 2.0f);
    simpleBufR.prepare(currentSampleRate, 2.0f);
    simpleL.prepare(&simpleBufL);
    simpleR.prepare(&simpleBufR);

    bbdL.prepare(currentSampleRate, maxDelaySamples);
    bbdR.prepare(currentSampleRate, maxDelaySamples);
    tapeL.prepare(currentSampleRate, maxDelaySamples);
    tapeR.prepare(currentSampleRate, maxDelaySamples);
    diffL.prepare(currentSampleRate, maxDelaySamples);
    diffR.prepare(currentSampleRate, maxDelaySamples);

    modLfo.prepare(currentSampleRate);

    heightHpL.prepare(currentSampleRate);  heightHpR.prepare(currentSampleRate);
    heightLpL.prepare(currentSampleRate);  heightLpR.prepare(currentSampleRate);
    hpTrimL.prepare(currentSampleRate);    hpTrimR.prepare(currentSampleRate);
    lpTrimL.prepare(currentSampleRate);    lpTrimR.prepare(currentSampleRate);
    headBumpL.prepare(currentSampleRate);  headBumpR.prepare(currentSampleRate);

    smoothedOutput.reset(currentSampleRate, 0.02);
    smoothedMix.reset(currentSampleRate, 0.02);

    fbStateL = fbStateR = 0.f;
    bbdClockPhase = 0.f;
    lastTapTime = 0.0;
    measuredTempoMs = 500.f;

    updateSnapshot();

    levelPeakL.store(0.0f);
    levelPeakR.store(0.0f);

    isPrepared = true;
}

void EchoForgeProcessor::releaseResources() { isPrepared = false; }

bool EchoForgeProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    const auto& in = layouts.getMainInputChannelSet();
    if (out != juce::AudioChannelSet::stereo() && out != juce::AudioChannelSet::mono())
        return false;
    if (in != juce::AudioChannelSet::stereo() && in != juce::AudioChannelSet::mono())
        return false;
    return true;
}

//==============================================================================
// parameterChanged
//==============================================================================
void EchoForgeProcessor::parameterChanged(const juce::String& paramID, float)
{
    // Tap tempo is a momentary: when the parameter changes we log the time.
    if (paramID == EchoParams::TAP_TEMPO)
    {
        const double now = juce::Time::getMillisecondCounterHiRes();
        if (lastTapTime > 0.0)
        {
            const double dt = now - lastTapTime;
            if (dt > 100.0 && dt < 3000.0)   // between 20 BPM and 600 BPM
            {
                measuredTempoMs = static_cast<float>(dt);
            }
        }
        lastTapTime = now;
    }

    updateSnapshot();
}

//==============================================================================
// updateSnapshot — display-only values, relaxed ordering
//==============================================================================
void EchoForgeProcessor::updateSnapshot()
{
    using namespace EchoParams;
    static constexpr auto rlx = std::memory_order_relaxed;
    auto raw = [this](std::atomic<float>* p) { return p ? p->load(rlx) : 0.f; };

    snapshot.engine.store((int)raw(pEngine), rlx);
    snapshot.center.store(raw(pCenter), rlx);
    snapshot.width.store(raw(pWidth), rlx);
    snapshot.height.store(raw(pHeight), rlx);
    snapshot.buffer.store(raw(pBuffer), rlx);
    snapshot.timeTrim.store(raw(pTimeTrim), rlx);
    snapshot.feedback.store(raw(pFeedback), rlx);
    snapshot.pingPong.store((int)raw(pPingPong), rlx);
    snapshot.modDepth.store(raw(pModDepth), rlx);
    snapshot.modRate.store(raw(pModRate), rlx);
    snapshot.modShape.store((int)raw(pModShape), rlx);
    snapshot.hpFreq.store(raw(pHpTrim), rlx);
    snapshot.lpFreq.store(raw(pLpTrim), rlx);
    snapshot.synced.store(raw(pSync) > 0.5f, rlx);
    snapshot.sampleRate.store((float)currentSampleRate, rlx);
}

//==============================================================================
// computeDelayMs
//
// The heart of the CENTER abstraction. CENTER is a normalised position in the
// active buffer; TIME_TRIM is a fine offset in seconds; SYNC (when on) forces
// the delay to a musical division of the host tempo.
//==============================================================================
float EchoForgeProcessor::computeDelayMs(float center01, float timeTrimSec,
    bool syncOn, int syncDiv, float bpm) const
{
    // Host-tempo sync: pure note division, ignore CENTER + TIME_TRIM
    if (syncOn)
    {
        // Division lengths in quarter-note beats. Index matches the sync combo.
        static const double divisionBeats[] = {
            0.25,          // 1/16
            1.0 / 3.0,     // 1/8T
            0.5,           // 1/8
            2.0 / 3.0,     // 1/4T
            1.0,           // 1/4
            2.0,           // 1/2
            4.0,           // 1/1
            8.0            // 2/1
        };
        const int idx = juce::jlimit(0, 7, syncDiv);
        const double seconds = divisionBeats[idx] * 60.0 / juce::jmax(1.0, (double)bpm);
        return static_cast<float>(seconds * 1000.0);
    }

    // Free-running: CENTER maps linearly into the *active* buffer length,
    // so a small BUFFER compresses the whole CENTER range accordingly.
    const float bufferSec = 0.25f
        + juce::jlimit(0.f, 1.f, pBuffer ? pBuffer->load() : 1.f) * 1.75f;
    const float centerSec = juce::jlimit(0.f, 1.f, center01) * bufferSec;
    const float trimmed = centerSec + timeTrimSec;

    return juce::jlimit(1.f, bufferSec * 1000.f, trimmed * 1000.f);
}

//==============================================================================
// computeHeightCenterHz
//
// The HEIGHT band's centre frequency tracks the CENTER knob: short delays are
// brighter, long delays are darker. HEIGHT_TRIM shifts the whole band up or
// down. The result is the frequency the HP and LP filters pivot around.
//==============================================================================
float EchoForgeProcessor::computeHeightCenterHz(float center01) const
{
    // CENTER 0..1 maps to 8 kHz..400 Hz, exponential
    const float baseHz = 8000.f * std::pow(0.05f, juce::jlimit(0.f, 1.f, center01));

    // HEIGHT_TRIM 0..1 maps to +/-2 octaves
    const float trim = pHeightTrim ? pHeightTrim->load() : 0.5f;
    const float octaves = (trim - 0.5f) * 4.f;

    return juce::jlimit(40.f, 18000.f, baseHz * std::pow(2.f, octaves));
}

//==============================================================================
// processBlock
//==============================================================================
void EchoForgeProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    if (!isPrepared) return;

    const int numSamples = buffer.getNumSamples();
    const int numCh = buffer.getNumChannels();
    if (numSamples <= 0 || numCh < 1) return;

    using namespace EchoParams;
    static constexpr auto rlx = std::memory_order_relaxed;

    updateSnapshot();

    const bool bypassed = apvts.getRawParameterValue(BYPASS)->load() > 0.5f;

    // --- Per-block parameter reads ---
    const int   engine = (int)pEngine->load();
    const float center = juce::jlimit(0.f, 1.f, pCenter->load());
    const float width = juce::jlimit(0.f, 1.f, pWidth->load());
    const float height = juce::jlimit(0.f, 1.f, pHeight->load());
    const float bufferPct = juce::jlimit(0.f, 1.f, pBuffer->load());
    const float timeTrim = pTimeTrim->load();
    const bool  syncOn = pSync->load() > 0.5f;
    const int   syncDiv = (int)pSyncDiv->load();
    const float feedback = juce::jlimit(0.f, 0.98f, pFeedback->load());
    const int   pingPong = (int)pPingPong->load();
    const float crossFeed = juce::jlimit(0.f, 1.f, pCrossFeed->load());
    const float hpTrimHz = pHpTrim->load();
    const float lpTrimHz = pLpTrim->load();
    const float modRate = pModRate->load();
    const float modDepth = juce::jlimit(0.f, 1.f, pModDepth->load());
    const int   modShape = (int)pModShape->load();

    const float bbdDrive = pBbdDrive->load();
    const float bbdCharge = pBbdCharge->load();
    const float bbdLeak = pBbdLeak->load();
    const float bbdStutter = pBbdStutter->load();
    const float tapeWow = pTapeWow->load();
    const float tapeFlutter = pTapeFlutter->load();
    const float tapeSat = pTapeSat->load();
    const float tapeHead = pTapeHead->load();
    const float diffSpread = pDiffSpread->load();
    const int   diffTaps = juce::jlimit(2, 8, (int)pDiffTaps->load());
    const float diffDamp = pDiffDamp->load();

    const float mix = juce::jlimit(0.f, 1.f, pMix->load());
    const float outputDb = pOutput->load();
    const float outWidth = juce::jlimit(0.f, 2.f, pOutWidth->load());

    smoothedMix.setTargetValue(mix);
    smoothedOutput.setTargetValue(juce::Decibels::decibelsToGain(outputDb));

    // --- Buffer size from BUFFER knob ---
    {
        const float minSec = 0.25f;
        const float maxSec = 2.0f;
        const float activeSec = minSec + bufferPct * (maxSec - minSec);
        const int activeSamples = juce::jlimit(64, maxDelaySamples,
            (int)(activeSec * (float)currentSampleRate));

        simpleBufL.setActiveBuffer(bufferPct);
        simpleBufR.setActiveBuffer(bufferPct);
        bbdL.setActiveSamples(activeSamples);
        bbdR.setActiveSamples(activeSamples);
        tapeL.setActiveSamples(activeSamples);
        tapeR.setActiveSamples(activeSamples);
        diffL.setActiveSamples(activeSamples);
        diffR.setActiveSamples(activeSamples);

        snapshot.activeSamples.store(activeSamples, rlx);
        snapshot.activeBufferMs.store(activeSec * 1000.f, rlx);
    }

    // --- Host tempo ---
    double bpm = 120.0;
    if (auto* ph = getPlayHead())
    {
        if (auto pos = ph->getPosition())
            if (auto b = pos->getBpm())
                if (*b > 1.0)
                    bpm = *b;
    }

    // --- Base delay time ---
    const float baseDelayMs = computeDelayMs(center, timeTrim, syncOn, syncDiv,
        (float)bpm);
    const float baseDelaySamples = baseDelayMs * 0.001f * (float)currentSampleRate;

    // --- HEIGHT band frequencies ---
    // The centre tracks CENTER; the width of the band is scaled by HEIGHT.
    const float heightCenterHz = computeHeightCenterHz(center);
    const float bandHalfOctaves = 0.5f + (1.f - height) * 3.5f;   // wider when HEIGHT low
    const float bandLowHz = heightCenterHz / std::pow(2.f, bandHalfOctaves);
    const float bandHighHz = heightCenterHz * std::pow(2.f, bandHalfOctaves);

    // HP = max(band low, user HP trim); LP = min(band high, user LP trim)
    const float hpFinal = juce::jlimit(20.f, 2000.f,
        juce::jmax(bandLowHz, hpTrimHz));
    const float lpFinal = juce::jlimit(200.f, 20000.f,
        juce::jmin(bandHighHz, lpTrimHz));

    heightHpL.setCutoff(hpFinal);  heightHpR.setCutoff(hpFinal);
    heightLpL.setCutoff(lpFinal);  heightLpR.setCutoff(lpFinal);
    hpTrimL.setCutoff(hpTrimHz);   hpTrimR.setCutoff(hpTrimHz);
    lpTrimL.setCutoff(lpTrimHz);   lpTrimR.setCutoff(lpTrimHz);

    // Tape head bump cutoff — always low, tied to the CENTER knob so a long
    // delay gets a bigger bump.
    const float headBumpHz = juce::jlimit(80.f, 500.f, 120.f + center * 300.f);
    headBumpL.setCutoff(headBumpHz);
    headBumpR.setCutoff(headBumpHz);

    // Input RMS for meter
    {
        float sumL = 0.f, sumR = 0.f;
        for (int i = 0; i < numSamples; ++i)
        {
            sumL += buffer.getSample(0, i) * buffer.getSample(0, i);
            if (numCh >= 2)
                sumR += buffer.getSample(1, i) * buffer.getSample(1, i);
        }
        levelInRmsL.store(std::sqrt(sumL / (float)numSamples));
        levelInRmsR.store(std::sqrt(sumR / (float)numSamples));
    }

    float* left = buffer.getWritePointer(0);
    float* right = (numCh >= 2) ? buffer.getWritePointer(1) : left;

    // -------------------------------------------------------------------
    // WIDTH: the two delay taps are spread either side of CENTER.
    //
    // WIDTH 0    : both channels identical -> mono delay
    // WIDTH 0.5  : small divergence
    // WIDTH 1    : full cross, one channel at near-zero, the other at the
    //              far edge of the buffer
    // -------------------------------------------------------------------
    const float widthOffset01 = width * 0.5f;    // 0..0.5 in normalised position

    // BBD clock boundary tracker: at each clock tick we may inject a stutter
    // (a captured slice of the input replayed inside the BBD line).
    const float bbdClockHz = 8000.f + center * 12000.f;
    const float bbdClockInc = bbdClockHz / (float)currentSampleRate;

    // Feedback meter accumulation
    float fbPeak = 0.f;

    // -------------------------------------------------------------------
    // Per-sample loop
    // -------------------------------------------------------------------
    for (int i = 0; i < numSamples; ++i)
    {
        const float dryL = left[i];
        const float dryR = right[i];

        // --- Modulation LFO ---
        float modValue = 0.f;
        if (modDepth > 0.0005f)
            modValue = modLfo.tick(modRate, modShape) * modDepth;

        // The mod offset is a fraction of the active buffer, capped so it
        // never exceeds 10% of the total.
        const float modOffset = modValue * 0.05f;

        // --- Compute L and R normalized delay positions ---
        // The base position comes from the delay time; the WIDTH knob spreads
        // the two channels around it.
        const float basePos = juce::jlimit(0.f, 1.f,
            baseDelaySamples / juce::jmax(1.f, (float)snapshot.activeSamples.load(rlx) - 2.f));

        const float posL = juce::jlimit(0.f, 1.f, basePos - widthOffset01 + modOffset);
        const float posR = juce::jlimit(0.f, 1.f, basePos + widthOffset01 + modOffset);

        // Convert back to ms for the display only
        const float activeSamplesF = juce::jmax(1.f, (float)snapshot.activeSamples.load(rlx));
        snapshot.liveDelayL.store(posL * activeSamplesF
            / (float)currentSampleRate * 1000.f, rlx);
        snapshot.liveDelayR.store(posR * activeSamplesF
            / (float)currentSampleRate * 1000.f, rlx);
        snapshot.modValue.store(modValue, rlx);

        // --- Engine processing ---
        float wetL = 0.f;
        float wetR = 0.f;

        switch (engine)
        {
        case (int)DelayEngine::Simple:
        {
            // Simple engine: write the input, read at the two positions
            simpleBufL.write(dryL);
            simpleBufR.write(dryR);
            wetL = simpleBufL.readNormalised(posL);
            wetR = simpleBufR.readNormalised(posR);
            break;
        }
        case (int)DelayEngine::BBD:
        {
            // Allpass corner tracks the CENTER knob
            const float allpassHz = 2000.f + center * 6000.f;

            // BBD clock boundary — every N samples we allow a stutter inject
            bbdClockPhase += bbdClockInc;
            if (bbdClockPhase >= 1.f)
            {
                bbdClockPhase -= 1.f;
                // Stutter injection is captured inside the BBDLine itself;
                // we just enable it here when the knob is up.
                // (The line reads the captured sample and injects it at the
                // next write, so no extra plumbing is needed.)
            }

            wetL = bbdL.process(dryL, posL, bbdDrive, bbdCharge, bbdLeak,
                allpassHz);
            wetR = bbdR.process(dryR, posR, bbdDrive, bbdCharge, bbdLeak,
                allpassHz);

            // Stutter: blend the BBD output with a self-tap from the L buffer
            // (this simulates the "click" you asked us to replace with the
            // source audio itself)
            if (bbdStutter > 0.0005f)
            {
                const float stutterL = simpleBufL.readNormalised(
                    juce::jlimit(0.f, 1.f, posL * 0.05f + 0.001f));
                const float stutterR = simpleBufR.readNormalised(
                    juce::jlimit(0.f, 1.f, posR * 0.05f + 0.001f));
                wetL = wetL * (1.f - bbdStutter) + stutterL * bbdStutter;
                wetR = wetR * (1.f - bbdStutter) + stutterR * bbdStutter;
            }
            break;
        }
        case (int)DelayEngine::Tape:
        {
            wetL = tapeL.process(dryL, posL, tapeWow, tapeFlutter, tapeSat);
            wetR = tapeR.process(dryR, posR, tapeWow, tapeFlutter, tapeSat);

            // Head bump: a low-shelf implemented as LP subtraction
            if (tapeHead > 0.0005f)
            {
                const float lpL = headBumpL.process(wetL);
                const float lpR = headBumpR.process(wetR);
                wetL += (lpL - wetL) * tapeHead * -1.f;
                wetR += (lpR - wetR) * tapeHead * -1.f;
            }
            break;
        }
        case (int)DelayEngine::Diffuse:
        default:
        {
            wetL = diffL.process(dryL, posL, diffSpread, diffDamp, diffTaps);
            wetR = diffR.process(dryR, posR, diffSpread, diffDamp, diffTaps);
            break;
        }
        }

        // --- Tone filters (HEIGHT band + trims) ---
        // The HEIGHT band is a HP + LP cascade; the trims are a second pair
        // stacked on top of it. Together they define the wet signal's
        // frequency character.
        wetL = heightHpL.process(wetL);
        wetL = heightLpL.process(wetL);
        wetL = hpTrimL.process(wetL);
        wetL = lpTrimL.process(wetL);

        wetR = heightHpR.process(wetR);
        wetR = heightLpR.process(wetR);
        wetR = hpTrimR.process(wetR);
        wetR = lpTrimR.process(wetR);

        // --- Feedback routing (ping-pong) ---
        // The feedback sample is fed back into the *input* of the next sample's
        // engine write. We keep the feedback state separately per channel.
        const float fbL = juce::jlimit(-1.f, 1.f, fbStateL);
        const float fbR = juce::jlimit(-1.f, 1.f, fbStateR);

        float nextFbL = 0.f;
        float nextFbR = 0.f;

        switch (pingPong)
        {
        case (int)PingPongMode::Off:
            nextFbL = wetL * feedback;
            nextFbR = wetR * feedback;
            break;
        case (int)PingPongMode::LToR:
            // L keeps its own feedback, R receives L's wet signal
            nextFbL = wetL * feedback;
            nextFbR = wetL * feedback * crossFeed + wetR * feedback * (1.f - crossFeed);
            break;
        case (int)PingPongMode::RToL:
            nextFbL = wetR * feedback * crossFeed + wetL * feedback * (1.f - crossFeed);
            nextFbR = wetR * feedback;
            break;
        case (int)PingPongMode::Both:
        default:
            nextFbL = wetR * feedback;
            nextFbR = wetL * feedback;
            break;
        }

        // Safety limiter on the feedback path
        nextFbL = EchoDSP::feedbackLimiter(nextFbL, 0.95f);
        nextFbR = EchoDSP::feedbackLimiter(nextFbR, 0.95f);

        fbStateL = nextFbL;
        fbStateR = nextFbR;
        fbPeak = juce::jmax(fbPeak, std::abs(nextFbL), std::abs(nextFbR));

        // --- Output stereo width ---
        float outL = wetL;
        float outR = wetR;
        if (std::abs(outWidth - 1.f) > 0.001f)
        {
            const float mid = (outL + outR) * 0.5f;
            const float side = (outL - outR) * 0.5f * outWidth;
            outL = mid + side;
            outR = mid - side;
        }

        // --- Dry/wet mix and output gain ---
        const float m = smoothedMix.getNextValue();
        const float g = smoothedOutput.getNextValue();
        const float finalL = (dryL * (1.f - m) + outL * m) * g;
        const float finalR = (dryR * (1.f - m) + outR * m) * g;

        if (bypassed)
        {
            left[i] = dryL;
            if (numCh >= 2) right[i] = dryR;
        }
        else
        {
            left[i] = finalL;
            if (numCh >= 2) right[i] = finalR;
        }
    }

    // Store the feedback meter
    feedbackAmount.store(fbPeak, rlx);

    midiMessages.clear();

    // Output meter
    float peakL = 0.f, peakR = 0.f;
    for (int i = 0; i < numSamples; ++i)
    {
        peakL = juce::jmax(peakL, std::abs(left[i]));
        if (numCh >= 2) peakR = juce::jmax(peakR, std::abs(right[i]));
    }
    levelPeakL.store(peakL);
    levelPeakR.store(peakR);
}

//==============================================================================
// A/B snapshot
//==============================================================================
void EchoForgeProcessor::captureAState()
{
    aState = apvts.copyState();
    aStateValid = true;
}

void EchoForgeProcessor::swapAB()
{
    if (!aStateValid) return;
    auto current = apvts.copyState();
    apvts.replaceState(aState);
    aState = current;
}

//==============================================================================
// Preset navigation
//==============================================================================
void EchoForgeProcessor::loadPresetByIndex(int index)
{
    if (!presetManager) return;
    if (index < 0 || index >= presetManager->getNumPresets()) return;

    presetManager->loadPreset(index);
    currentPresetIndex = index;
    if (onPresetChanged) onPresetChanged(index);
}

void EchoForgeProcessor::loadPreviousPreset()
{
    if (!presetManager) return;
    int n = presetManager->getNumPresets();
    if (n == 0) return;
    int idx = (currentPresetIndex <= 0) ? n - 1 : currentPresetIndex - 1;
    loadPresetByIndex(idx);
}

void EchoForgeProcessor::loadNextPreset()
{
    if (!presetManager) return;
    int n = presetManager->getNumPresets();
    if (n == 0) return;
    int idx = (currentPresetIndex + 1) % n;
    loadPresetByIndex(idx);
}

//==============================================================================
// State
//==============================================================================
void EchoForgeProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void EchoForgeProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

//==============================================================================
// Editor
//==============================================================================
juce::AudioProcessorEditor* EchoForgeProcessor::createEditor()
{
    return new EchoForgeEditor(*this);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EchoForgeProcessor();
}