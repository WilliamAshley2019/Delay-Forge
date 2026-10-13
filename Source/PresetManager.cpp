#include "PresetManager.h"
#include "PluginProcessor.h"
#include <set>
#include <algorithm>

//==============================================================================
// Factory preset bank
//
// Categories: Clean, BBD, Tape, Ping-Pong, Diffuse, Creative
//==============================================================================
namespace
{
    using P = std::pair<const char*, float>;

    PresetManager::FactoryPreset makePreset(const char* name,
        const char* category,
        std::initializer_list<P> pairs)
    {
        PresetManager::FactoryPreset fp;
        fp.name = name;
        fp.category = category;
        fp.author = "Factory";
        for (auto& p : pairs)
            fp.params.push_back({ juce::String(p.first), p.second });
        return fp;
    }
}

const std::vector<PresetManager::FactoryPreset>&
PresetManager::getFactoryPresets()
{
    using namespace EchoParams;

    static const std::vector<FactoryPreset> presets = []()
        {
            std::vector<FactoryPreset> v;

            // =========================================================
            // CLEAN — pure digital delays
            // =========================================================
            v.push_back(makePreset("Init (Clean)", "Clean", {
                { ENGINE, 0.f },{ BYPASS, 0.f },
                { CENTER, 0.3f },{ WIDTH, 0.5f },{ HEIGHT, 0.9f },{ BUFFER, 1.f },
                { TIME_TRIM, 0.f },{ SYNC, 0.f },{ SYNC_DIV, 4.f },{ TAP_TEMPO, 0.f },
                { FEEDBACK, 0.35f },{ PING_PONG, 0.f },{ CROSS_FEED, 0.5f },
                { HEIGHT_TRIM, 0.5f },{ HP_TRIM, 20.f },{ LP_TRIM, 20000.f },
                { MOD_RATE, 0.5f },{ MOD_DEPTH, 0.f },{ MOD_SHAPE, 0.f },
                { MIX, 0.35f },{ OUTPUT, 0.f },{ OUT_WIDTH, 1.f }
                }));

            v.push_back(makePreset("Slapback", "Clean", {
                { ENGINE, 0.f },{ BYPASS, 0.f },
                { CENTER, 0.08f },{ WIDTH, 0.3f },{ HEIGHT, 0.85f },{ BUFFER, 0.15f },
                { TIME_TRIM, 0.f },{ SYNC, 0.f },{ SYNC_DIV, 4.f },{ TAP_TEMPO, 0.f },
                { FEEDBACK, 0.15f },{ PING_PONG, 0.f },{ CROSS_FEED, 0.5f },
                { HEIGHT_TRIM, 0.6f },{ HP_TRIM, 200.f },{ LP_TRIM, 6000.f },
                { MOD_RATE, 0.5f },{ MOD_DEPTH, 0.f },{ MOD_SHAPE, 0.f },
                { MIX, 0.3f },{ OUTPUT, 0.f },{ OUT_WIDTH, 0.6f }
                }));

            v.push_back(makePreset("Quarter Note Echo", "Clean", {
                { ENGINE, 0.f },{ BYPASS, 0.f },
                { CENTER, 0.25f },{ WIDTH, 0.4f },{ HEIGHT, 0.9f },{ BUFFER, 0.6f },
                { TIME_TRIM, 0.f },{ SYNC, 1.f },{ SYNC_DIV, 4.f },{ TAP_TEMPO, 0.f },
                { FEEDBACK, 0.4f },{ PING_PONG, 0.f },{ CROSS_FEED, 0.5f },
                { HEIGHT_TRIM, 0.5f },{ HP_TRIM, 100.f },{ LP_TRIM, 8000.f },
                { MOD_RATE, 0.5f },{ MOD_DEPTH, 0.f },{ MOD_SHAPE, 0.f },
                { MIX, 0.35f },{ OUTPUT, 0.f },{ OUT_WIDTH, 1.f }
                }));

            v.push_back(makePreset("Dotted 1/8", "Clean", {
                { ENGINE, 0.f },{ BYPASS, 0.f },
                { CENTER, 0.22f },{ WIDTH, 0.6f },{ HEIGHT, 0.85f },{ BUFFER, 0.5f },
                { TIME_TRIM, 0.f },{ SYNC, 1.f },{ SYNC_DIV, 2.f },{ TAP_TEMPO, 0.f },
                { FEEDBACK, 0.45f },{ PING_PONG, 1.f },{ CROSS_FEED, 0.5f },
                { HEIGHT_TRIM, 0.5f },{ HP_TRIM, 120.f },{ LP_TRIM, 9000.f },
                { MOD_RATE, 0.5f },{ MOD_DEPTH, 0.f },{ MOD_SHAPE, 0.f },
                { MIX, 0.4f },{ OUTPUT, -1.f },{ OUT_WIDTH, 1.2f }
                }));

            // =========================================================
            // BBD — bucket-brigade character
            // =========================================================
            v.push_back(makePreset("BBD Classic", "BBD", {
                { ENGINE, 1.f },{ BYPASS, 0.f },
                { CENTER, 0.18f },{ WIDTH, 0.5f },{ HEIGHT, 0.6f },{ BUFFER, 0.4f },
                { TIME_TRIM, 0.f },{ SYNC, 0.f },{ SYNC_DIV, 4.f },{ TAP_TEMPO, 0.f },
                { FEEDBACK, 0.45f },{ PING_PONG, 0.f },{ CROSS_FEED, 0.5f },
                { HEIGHT_TRIM, 0.4f },{ HP_TRIM, 80.f },{ LP_TRIM, 5000.f },
                { MOD_RATE, 0.4f },{ MOD_DEPTH, 0.15f },{ MOD_SHAPE, 0.f },
                { BBD_DRIVE, 0.35f },{ BBD_CHARGE, 0.4f },{ BBD_LEAK, 0.25f },{ BBD_STUTTER, 0.f },
                { MIX, 0.4f },{ OUTPUT, -2.f },{ OUT_WIDTH, 1.f }
                }));

            v.push_back(makePreset("BBD Dark Dub", "BBD", {
                { ENGINE, 1.f },{ BYPASS, 0.f },
                { CENTER, 0.22f },{ WIDTH, 0.55f },{ HEIGHT, 0.35f },{ BUFFER, 0.5f },
                { TIME_TRIM, 0.f },{ SYNC, 0.f },{ SYNC_DIV, 4.f },{ TAP_TEMPO, 0.f },
                { FEEDBACK, 0.7f },{ PING_PONG, 3.f },{ CROSS_FEED, 0.5f },
                { HEIGHT_TRIM, 0.25f },{ HP_TRIM, 60.f },{ LP_TRIM, 3000.f },
                { MOD_RATE, 0.3f },{ MOD_DEPTH, 0.25f },{ MOD_SHAPE, 0.f },
                { BBD_DRIVE, 0.55f },{ BBD_CHARGE, 0.6f },{ BBD_LEAK, 0.4f },{ BBD_STUTTER, 0.f },
                { MIX, 0.45f },{ OUTPUT, -3.f },{ OUT_WIDTH, 1.2f }
                }));

            v.push_back(makePreset("BBD Lo-Fi", "BBD", {
                { ENGINE, 1.f },{ BYPASS, 0.f },
                { CENTER, 0.15f },{ WIDTH, 0.35f },{ HEIGHT, 0.25f },{ BUFFER, 0.3f },
                { TIME_TRIM, 0.f },{ SYNC, 0.f },{ SYNC_DIV, 4.f },{ TAP_TEMPO, 0.f },
                { FEEDBACK, 0.4f },{ PING_PONG, 0.f },{ CROSS_FEED, 0.5f },
                { HEIGHT_TRIM, 0.2f },{ HP_TRIM, 300.f },{ LP_TRIM, 2500.f },
                { MOD_RATE, 0.7f },{ MOD_DEPTH, 0.3f },{ MOD_SHAPE, 0.f },
                { BBD_DRIVE, 0.6f },{ BBD_CHARGE, 0.7f },{ BBD_LEAK, 0.5f },{ BBD_STUTTER, 0.1f },
                { MIX, 0.4f },{ OUTPUT, -4.f },{ OUT_WIDTH, 0.9f }
                }));

            v.push_back(makePreset("BBD Ghost", "BBD", {
                { ENGINE, 1.f },{ BYPASS, 0.f },
                { CENTER, 0.4f },{ WIDTH, 0.7f },{ HEIGHT, 0.5f },{ BUFFER, 0.7f },
                { TIME_TRIM, 0.f },{ SYNC, 0.f },{ SYNC_DIV, 4.f },{ TAP_TEMPO, 0.f },
                { FEEDBACK, 0.55f },{ PING_PONG, 3.f },{ CROSS_FEED, 0.4f },
                { HEIGHT_TRIM, 0.35f },{ HP_TRIM, 150.f },{ LP_TRIM, 4500.f },
                { MOD_RATE, 0.2f },{ MOD_DEPTH, 0.35f },{ MOD_SHAPE, 0.f },
                { BBD_DRIVE, 0.3f },{ BBD_CHARGE, 0.5f },{ BBD_LEAK, 0.3f },{ BBD_STUTTER, 0.f },
                { MIX, 0.5f },{ OUTPUT, -3.f },{ OUT_WIDTH, 1.3f }
                }));

            v.push_back(makePreset("BBD Stutter", "BBD", {
                { ENGINE, 1.f },{ BYPASS, 0.f },
                { CENTER, 0.12f },{ WIDTH, 0.4f },{ HEIGHT, 0.55f },{ BUFFER, 0.25f },
                { TIME_TRIM, 0.f },{ SYNC, 0.f },{ SYNC_DIV, 4.f },{ TAP_TEMPO, 0.f },
                { FEEDBACK, 0.5f },{ PING_PONG, 1.f },{ CROSS_FEED, 0.6f },
                { HEIGHT_TRIM, 0.45f },{ HP_TRIM, 200.f },{ LP_TRIM, 5000.f },
                { MOD_RATE, 0.5f },{ MOD_DEPTH, 0.2f },{ MOD_SHAPE, 0.f },
                { BBD_DRIVE, 0.4f },{ BBD_CHARGE, 0.3f },{ BBD_LEAK, 0.2f },{ BBD_STUTTER, 0.5f },
                { MIX, 0.45f },{ OUTPUT, -2.f },{ OUT_WIDTH, 1.1f }
                }));

            // =========================================================
            // TAPE — tape echo
            // =========================================================
            v.push_back(makePreset("Tape Warm", "Tape", {
                { ENGINE, 2.f },{ BYPASS, 0.f },
                { CENTER, 0.2f },{ WIDTH, 0.5f },{ HEIGHT, 0.7f },{ BUFFER, 0.5f },
                { TIME_TRIM, 0.f },{ SYNC, 0.f },{ SYNC_DIV, 4.f },{ TAP_TEMPO, 0.f },
                { FEEDBACK, 0.4f },{ PING_PONG, 0.f },{ CROSS_FEED, 0.5f },
                { HEIGHT_TRIM, 0.5f },{ HP_TRIM, 60.f },{ LP_TRIM, 7000.f },
                { MOD_RATE, 0.5f },{ MOD_DEPTH, 0.f },{ MOD_SHAPE, 0.f },
                { TAPE_WOW, 0.3f },{ TAPE_FLUTTER, 0.2f },{ TAPE_SAT, 0.4f },{ TAPE_HEAD, 0.5f },
                { MIX, 0.4f },{ OUTPUT, -1.f },{ OUT_WIDTH, 1.1f }
                }));

            v.push_back(makePreset("Tape Slap", "Tape", {
                { ENGINE, 2.f },{ BYPASS, 0.f },
                { CENTER, 0.08f },{ WIDTH, 0.4f },{ HEIGHT, 0.6f },{ BUFFER, 0.2f },
                { TIME_TRIM, 0.f },{ SYNC, 0.f },{ SYNC_DIV, 4.f },{ TAP_TEMPO, 0.f },
                { FEEDBACK, 0.25f },{ PING_PONG, 0.f },{ CROSS_FEED, 0.5f },
                { HEIGHT_TRIM, 0.6f },{ HP_TRIM, 120.f },{ LP_TRIM, 6000.f },
                { MOD_RATE, 0.8f },{ MOD_DEPTH, 0.f },{ MOD_SHAPE, 0.f },
                { TAPE_WOW, 0.4f },{ TAPE_FLUTTER, 0.3f },{ TAPE_SAT, 0.5f },{ TAPE_HEAD, 0.6f },
                { MIX, 0.35f },{ OUTPUT, 0.f },{ OUT_WIDTH, 0.8f }
                }));

            v.push_back(makePreset("Tape Cathedral", "Tape", {
                { ENGINE, 2.f },{ BYPASS, 0.f },
                { CENTER, 0.55f },{ WIDTH, 0.7f },{ HEIGHT, 0.5f },{ BUFFER, 0.9f },
                { TIME_TRIM, 0.f },{ SYNC, 0.f },{ SYNC_DIV, 4.f },{ TAP_TEMPO, 0.f },
                { FEEDBACK, 0.6f },{ PING_PONG, 3.f },{ CROSS_FEED, 0.5f },
                { HEIGHT_TRIM, 0.4f },{ HP_TRIM, 80.f },{ LP_TRIM, 4500.f },
                { MOD_RATE, 0.3f },{ MOD_DEPTH, 0.f },{ MOD_SHAPE, 0.f },
                { TAPE_WOW, 0.5f },{ TAPE_FLUTTER, 0.4f },{ TAPE_SAT, 0.35f },{ TAPE_HEAD, 0.5f },
                { MIX, 0.5f },{ OUTPUT, -3.f },{ OUT_WIDTH, 1.3f }
                }));

            v.push_back(makePreset("Tape Wobbly", "Tape", {
                { ENGINE, 2.f },{ BYPASS, 0.f },
                { CENTER, 0.25f },{ WIDTH, 0.5f },{ HEIGHT, 0.55f },{ BUFFER, 0.6f },
                { TIME_TRIM, 0.f },{ SYNC, 0.f },{ SYNC_DIV, 4.f },{ TAP_TEMPO, 0.f },
                { FEEDBACK, 0.45f },{ PING_PONG, 0.f },{ CROSS_FEED, 0.5f },
                { HEIGHT_TRIM, 0.5f },{ HP_TRIM, 100.f },{ LP_TRIM, 5500.f },
                { MOD_RATE, 0.5f },{ MOD_DEPTH, 0.f },{ MOD_SHAPE, 0.f },
                { TAPE_WOW, 0.8f },{ TAPE_FLUTTER, 0.6f },{ TAPE_SAT, 0.4f },{ TAPE_HEAD, 0.4f },
                { MIX, 0.4f },{ OUTPUT, -2.f },{ OUT_WIDTH, 1.f }
                }));

            v.push_back(makePreset("Vintage 50s", "Tape", {
                { ENGINE, 2.f },{ BYPASS, 0.f },
                { CENTER, 0.15f },{ WIDTH, 0.3f },{ HEIGHT, 0.3f },{ BUFFER, 0.4f },
                { TIME_TRIM, 0.f },{ SYNC, 0.f },{ SYNC_DIV, 4.f },{ TAP_TEMPO, 0.f },
                { FEEDBACK, 0.35f },{ PING_PONG, 0.f },{ CROSS_FEED, 0.5f },
                { HEIGHT_TRIM, 0.3f },{ HP_TRIM, 200.f },{ LP_TRIM, 3500.f },
                { MOD_RATE, 0.6f },{ MOD_DEPTH, 0.f },{ MOD_SHAPE, 0.f },
                { TAPE_WOW, 0.5f },{ TAPE_FLUTTER, 0.35f },{ TAPE_SAT, 0.6f },{ TAPE_HEAD, 0.8f },
                { MIX, 0.4f },{ OUTPUT, -2.f },{ OUT_WIDTH, 0.7f }
                }));

            // =========================================================
            // PING-PONG — stereo bouncing
            // =========================================================
            v.push_back(makePreset("Ping-Pong Clean", "Ping-Pong", {
                { ENGINE, 0.f },{ BYPASS, 0.f },
                { CENTER, 0.22f },{ WIDTH, 0.8f },{ HEIGHT, 0.85f },{ BUFFER, 0.5f },
                { TIME_TRIM, 0.f },{ SYNC, 1.f },{ SYNC_DIV, 4.f },{ TAP_TEMPO, 0.f },
                { FEEDBACK, 0.5f },{ PING_PONG, 3.f },{ CROSS_FEED, 0.7f },
                { HEIGHT_TRIM, 0.5f },{ HP_TRIM, 100.f },{ LP_TRIM, 10000.f },
                { MOD_RATE, 0.5f },{ MOD_DEPTH, 0.f },{ MOD_SHAPE, 0.f },
                { MIX, 0.4f },{ OUTPUT, -1.f },{ OUT_WIDTH, 1.4f }
                }));

            v.push_back(makePreset("Ping-Pong Dub", "Ping-Pong", {
                { ENGINE, 1.f },{ BYPASS, 0.f },
                { CENTER, 0.3f },{ WIDTH, 0.9f },{ HEIGHT, 0.35f },{ BUFFER, 0.7f },
                { TIME_TRIM, 0.f },{ SYNC, 1.f },{ SYNC_DIV, 3.f },{ TAP_TEMPO, 0.f },
                { FEEDBACK, 0.7f },{ PING_PONG, 3.f },{ CROSS_FEED, 0.6f },
                { HEIGHT_TRIM, 0.3f },{ HP_TRIM, 80.f },{ LP_TRIM, 3500.f },
                { MOD_RATE, 0.3f },{ MOD_DEPTH, 0.3f },{ MOD_SHAPE, 0.f },
                { BBD_DRIVE, 0.5f },{ BBD_CHARGE, 0.5f },{ BBD_LEAK, 0.35f },{ BBD_STUTTER, 0.f },
                { MIX, 0.5f },{ OUTPUT, -3.f },{ OUT_WIDTH, 1.5f }
                }));

            v.push_back(makePreset("Ping-Pong Triplet", "Ping-Pong", {
                { ENGINE, 0.f },{ BYPASS, 0.f },
                { CENTER, 0.18f },{ WIDTH, 0.75f },{ HEIGHT, 0.8f },{ BUFFER, 0.4f },
                { TIME_TRIM, 0.f },{ SYNC, 1.f },{ SYNC_DIV, 1.f },{ TAP_TEMPO, 0.f },
                { FEEDBACK, 0.55f },{ PING_PONG, 3.f },{ CROSS_FEED, 0.65f },
                { HEIGHT_TRIM, 0.5f },{ HP_TRIM, 100.f },{ LP_TRIM, 9000.f },
                { MOD_RATE, 0.5f },{ MOD_DEPTH, 0.f },{ MOD_SHAPE, 0.f },
                { MIX, 0.42f },{ OUTPUT, -2.f },{ OUT_WIDTH, 1.3f }
                }));

            v.push_back(makePreset("Ping-Pong Tape", "Ping-Pong", {
                { ENGINE, 2.f },{ BYPASS, 0.f },
                { CENTER, 0.25f },{ WIDTH, 0.85f },{ HEIGHT, 0.6f },{ BUFFER, 0.5f },
                { TIME_TRIM, 0.f },{ SYNC, 0.f },{ SYNC_DIV, 4.f },{ TAP_TEMPO, 0.f },
                { FEEDBACK, 0.55f },{ PING_PONG, 3.f },{ CROSS_FEED, 0.55f },
                { HEIGHT_TRIM, 0.45f },{ HP_TRIM, 100.f },{ LP_TRIM, 6000.f },
                { MOD_RATE, 0.4f },{ MOD_DEPTH, 0.f },{ MOD_SHAPE, 0.f },
                { TAPE_WOW, 0.5f },{ TAPE_FLUTTER, 0.35f },{ TAPE_SAT, 0.4f },{ TAPE_HEAD, 0.5f },
                { MIX, 0.45f },{ OUTPUT, -3.f },{ OUT_WIDTH, 1.4f }
                }));

            // =========================================================
            // DIFFUSE — smeared clouds
            // =========================================================
            v.push_back(makePreset("Diffuse Smear", "Diffuse", {
                { ENGINE, 3.f },{ BYPASS, 0.f },
                { CENTER, 0.1f },{ WIDTH, 0.6f },{ HEIGHT, 0.7f },{ BUFFER, 0.3f },
                { TIME_TRIM, 0.f },{ SYNC, 0.f },{ SYNC_DIV, 4.f },{ TAP_TEMPO, 0.f },
                { FEEDBACK, 0.5f },{ PING_PONG, 0.f },{ CROSS_FEED, 0.5f },
                { HEIGHT_TRIM, 0.5f },{ HP_TRIM, 200.f },{ LP_TRIM, 6000.f },
                { MOD_RATE, 0.5f },{ MOD_DEPTH, 0.f },{ MOD_SHAPE, 0.f },
                { DIFF_SPREAD, 0.7f },{ DIFF_TAPS, 6.f },{ DIFF_DAMP, 0.5f },
                { MIX, 0.4f },{ OUTPUT, -2.f },{ OUT_WIDTH, 1.2f }
                }));

            v.push_back(makePreset("Diffuse Cloud", "Diffuse", {
                { ENGINE, 3.f },{ BYPASS, 0.f },
                { CENTER, 0.2f },{ WIDTH, 0.8f },{ HEIGHT, 0.5f },{ BUFFER, 0.5f },
                { TIME_TRIM, 0.f },{ SYNC, 0.f },{ SYNC_DIV, 4.f },{ TAP_TEMPO, 0.f },
                { FEEDBACK, 0.6f },{ PING_PONG, 1.f },{ CROSS_FEED, 0.6f },
                { HEIGHT_TRIM, 0.4f },{ HP_TRIM, 150.f },{ LP_TRIM, 4500.f },
                { MOD_RATE, 0.4f },{ MOD_DEPTH, 0.2f },{ MOD_SHAPE, 0.f },
                { DIFF_SPREAD, 0.85f },{ DIFF_TAPS, 8.f },{ DIFF_DAMP, 0.6f },
                { MIX, 0.45f },{ OUTPUT, -3.f },{ OUT_WIDTH, 1.4f }
                }));

            v.push_back(makePreset("Diffuse Reverb", "Diffuse", {
                { ENGINE, 3.f },{ BYPASS, 0.f },
                { CENTER, 0.05f },{ WIDTH, 0.9f },{ HEIGHT, 0.55f },{ BUFFER, 0.25f },
                { TIME_TRIM, 0.f },{ SYNC, 0.f },{ SYNC_DIV, 4.f },{ TAP_TEMPO, 0.f },
                { FEEDBACK, 0.7f },{ PING_PONG, 3.f },{ CROSS_FEED, 0.5f },
                { HEIGHT_TRIM, 0.45f },{ HP_TRIM, 250.f },{ LP_TRIM, 5000.f },
                { MOD_RATE, 0.3f },{ MOD_DEPTH, 0.3f },{ MOD_SHAPE, 0.f },
                { DIFF_SPREAD, 0.9f },{ DIFF_TAPS, 8.f },{ DIFF_DAMP, 0.7f },
                { MIX, 0.5f },{ OUTPUT, -4.f },{ OUT_WIDTH, 1.5f }
                }));

            v.push_back(makePreset("Diffuse Chorus", "Diffuse", {
                { ENGINE, 3.f },{ BYPASS, 0.f },
                { CENTER, 0.04f },{ WIDTH, 0.7f },{ HEIGHT, 0.75f },{ BUFFER, 0.15f },
                { TIME_TRIM, 0.f },{ SYNC, 0.f },{ SYNC_DIV, 4.f },{ TAP_TEMPO, 0.f },
                { FEEDBACK, 0.35f },{ PING_PONG, 1.f },{ CROSS_FEED, 0.5f },
                { HEIGHT_TRIM, 0.55f },{ HP_TRIM, 200.f },{ LP_TRIM, 7000.f },
                { MOD_RATE, 1.5f },{ MOD_DEPTH, 0.4f },{ MOD_SHAPE, 0.f },
                { DIFF_SPREAD, 0.6f },{ DIFF_TAPS, 4.f },{ DIFF_DAMP, 0.3f },
                { MIX, 0.35f },{ OUTPUT, -1.f },{ OUT_WIDTH, 1.3f }
                }));

            // =========================================================
            // CREATIVE — extreme / sound design
            // =========================================================
            v.push_back(makePreset("Infinite Feedback", "Creative", {
                { ENGINE, 1.f },{ BYPASS, 0.f },
                { CENTER, 0.4f },{ WIDTH, 0.7f },{ HEIGHT, 0.4f },{ BUFFER, 0.8f },
                { TIME_TRIM, 0.f },{ SYNC, 0.f },{ SYNC_DIV, 4.f },{ TAP_TEMPO, 0.f },
                { FEEDBACK, 0.95f },{ PING_PONG, 3.f },{ CROSS_FEED, 0.5f },
                { HEIGHT_TRIM, 0.35f },{ HP_TRIM, 200.f },{ LP_TRIM, 4000.f },
                { MOD_RATE, 0.2f },{ MOD_DEPTH, 0.4f },{ MOD_SHAPE, 0.f },
                { BBD_DRIVE, 0.5f },{ BBD_CHARGE, 0.4f },{ BBD_LEAK, 0.5f },{ BBD_STUTTER, 0.f },
                { MIX, 0.5f },{ OUTPUT, -6.f },{ OUT_WIDTH, 1.3f }
                }));

            v.push_back(makePreset("Riser", "Creative", {
                { ENGINE, 2.f },{ BYPASS, 0.f },
                { CENTER, 0.5f },{ WIDTH, 0.4f },{ HEIGHT, 0.7f },{ BUFFER, 0.9f },
                { TIME_TRIM, 0.3f },{ SYNC, 0.f },{ SYNC_DIV, 4.f },{ TAP_TEMPO, 0.f },
                { FEEDBACK, 0.75f },{ PING_PONG, 0.f },{ CROSS_FEED, 0.5f },
                { HEIGHT_TRIM, 0.6f },{ HP_TRIM, 300.f },{ LP_TRIM, 12000.f },
                { MOD_RATE, 0.5f },{ MOD_DEPTH, 0.f },{ MOD_SHAPE, 0.f },
                { TAPE_WOW, 0.6f },{ TAPE_FLUTTER, 0.5f },{ TAPE_SAT, 0.4f },{ TAPE_HEAD, 0.3f },
                { MIX, 0.5f },{ OUTPUT, -4.f },{ OUT_WIDTH, 1.2f }
                }));

            v.push_back(makePreset("Ambient Pad", "Creative", {
                { ENGINE, 2.f },{ BYPASS, 0.f },
                { CENTER, 0.6f },{ WIDTH, 0.9f },{ HEIGHT, 0.65f },{ BUFFER, 1.f },
                { TIME_TRIM, 0.f },{ SYNC, 0.f },{ SYNC_DIV, 4.f },{ TAP_TEMPO, 0.f },
                { FEEDBACK, 0.7f },{ PING_PONG, 3.f },{ CROSS_FEED, 0.5f },
                { HEIGHT_TRIM, 0.55f },{ HP_TRIM, 150.f },{ LP_TRIM, 7000.f },
                { MOD_RATE, 0.15f },{ MOD_DEPTH, 0.5f },{ MOD_SHAPE, 0.f },
                { TAPE_WOW, 0.4f },{ TAPE_FLUTTER, 0.3f },{ TAPE_SAT, 0.3f },{ TAPE_HEAD, 0.4f },
                { MIX, 0.55f },{ OUTPUT, -3.f },{ OUT_WIDTH, 1.5f }
                }));

            v.push_back(makePreset("Metallic Comb", "Creative", {
                { ENGINE, 0.f },{ BYPASS, 0.f },
                { CENTER, 0.015f },{ WIDTH, 0.5f },{ HEIGHT, 0.9f },{ BUFFER, 0.05f },
                { TIME_TRIM, 0.f },{ SYNC, 0.f },{ SYNC_DIV, 4.f },{ TAP_TEMPO, 0.f },
                { FEEDBACK, 0.7f },{ PING_PONG, 0.f },{ CROSS_FEED, 0.5f },
                { HEIGHT_TRIM, 0.6f },{ HP_TRIM, 100.f },{ LP_TRIM, 15000.f },
                { MOD_RATE, 3.f },{ MOD_DEPTH, 0.3f },{ MOD_SHAPE, 0.f },
                { MIX, 0.5f },{ OUTPUT, -3.f },{ OUT_WIDTH, 1.f }
                }));

            v.push_back(makePreset("Reverse Illusion", "Creative", {
                { ENGINE, 2.f },{ BYPASS, 0.f },
                { CENTER, 0.35f },{ WIDTH, 0.6f },{ HEIGHT, 0.5f },{ BUFFER, 0.7f },
                { TIME_TRIM, 0.f },{ SYNC, 0.f },{ SYNC_DIV, 4.f },{ TAP_TEMPO, 0.f },
                { FEEDBACK, 0.65f },{ PING_PONG, 3.f },{ CROSS_FEED, 0.5f },
                { HEIGHT_TRIM, 0.4f },{ HP_TRIM, 250.f },{ LP_TRIM, 5000.f },
                { MOD_RATE, 0.5f },{ MOD_DEPTH, 0.f },{ MOD_SHAPE, 0.f },
                { TAPE_WOW, 0.5f },{ TAPE_FLUTTER, 0.5f },{ TAPE_SAT, 0.4f },{ TAPE_HEAD, 0.6f },
                { MIX, 0.5f },{ OUTPUT, -3.f },{ OUT_WIDTH, 1.3f }
                }));

            // Keep categories contiguous so the browser shows one header each
            static const char* order[] = { "Clean", "BBD", "Tape", "Ping-Pong",
                                            "Diffuse", "Creative" };
            auto rank = [](const juce::String& cat) {
                for (int k = 0; k < 6; ++k) if (cat == order[k]) return k;
                return 6; };
            std::stable_sort(v.begin(), v.end(), [&](const FactoryPreset& a,
                const FactoryPreset& b)
                { return rank(a.category) < rank(b.category); });

            return v;
        }();

    return presets;
}

//==============================================================================
// Constructor
//==============================================================================
PresetManager::PresetManager(juce::AudioProcessorValueTreeState& a)
    : apvts(a)
{
    refresh();
}

//==============================================================================
// refresh
//==============================================================================
void PresetManager::refresh()
{
    presets.clear();

    const auto& factory = getFactoryPresets();
    for (int i = 0; i < (int)factory.size(); ++i)
    {
        PresetInfo info;
        info.name = factory[(size_t)i].name;
        info.category = factory[(size_t)i].category;
        info.author = factory[(size_t)i].author;
        info.isFactory = true;
        info.factoryIndex = i;
        presets.push_back(info);
    }

    scanUserPresets();
}

//==============================================================================
// scanUserPresets
//==============================================================================
void PresetManager::scanUserPresets()
{
    auto dir = getUserPresetDirectory();
    if (!dir.isDirectory()) return;

    juce::Array<juce::File> files;
    dir.findChildFiles(files, juce::File::findFiles, false, "*.echoforge");

    std::sort(files.begin(), files.end(),
        [](const juce::File& a, const juce::File& b)
        {
            return a.getFileNameWithoutExtension().compareNatural(
                b.getFileNameWithoutExtension()) < 0;
        });

    for (auto& f : files)
    {
        juce::String name, category, author;
        if (readUserPresetMetadata(f, name, category, author))
        {
            PresetInfo info;
            info.name = name;
            info.category = category;
            info.author = author;
            info.isFactory = false;
            info.factoryIndex = -1;
            info.file = f;
            presets.push_back(info);
        }
    }
}

//==============================================================================
// Query helpers
//==============================================================================
int PresetManager::getNumPresets() const
{
    return (int)presets.size();
}

const PresetManager::PresetInfo& PresetManager::getPresetInfo(int index) const
{
    jassert(index >= 0 && index < (int)presets.size());
    return presets[(size_t)index];
}

//==============================================================================
// getCurrentPresetIndex — best-effort match against current APVTS state
//==============================================================================
int PresetManager::getCurrentPresetIndex() const
{
    const auto& factory = getFactoryPresets();
    for (int i = 0; i < (int)factory.size(); ++i)
    {
        bool match = true;
        for (auto& kv : factory[(size_t)i].params)
        {
            auto* param = apvts.getParameter(kv.first);
            if (!param) { match = false; break; }
            const float natural = param->getNormalisableRange().convertFrom0to1(
                param->getValue());
            if (std::abs(natural - kv.second) > 1e-3f)
            {
                match = false;
                break;
            }
        }
        if (match)
        {
            // Params the preset does not list are implicitly at their default
            std::set<juce::String> listed;
            for (auto& kv : factory[(size_t)i].params) listed.insert(kv.first);
            for (auto* param : apvts.processor.getParameters())
                if (auto* p = dynamic_cast<juce::RangedAudioParameter*>(param))
                    if (listed.count(p->getParameterID()) == 0
                        && std::abs(p->getValue() - p->getDefaultValue()) > 1e-3f)
                    {
                        match = false; break;
                    }
        }
        if (match) return i;
    }
    return -1;
}

//==============================================================================
// loadPreset
//==============================================================================
bool PresetManager::loadPreset(int index)
{
    if (index < 0 || index >= (int)presets.size()) return false;
    const auto& info = presets[(size_t)index];

    if (info.isFactory)
    {
        const auto& factory = getFactoryPresets();
        if (info.factoryIndex < 0 || info.factoryIndex >= (int)factory.size())
            return false;

        // Start from defaults so params added after a preset was authored
        // (or not listed in it) never inherit the previous patch.
        resetAllToDefaults();

        for (auto& kv : factory[(size_t)info.factoryIndex].params)
        {
            if (auto* p = apvts.getParameter(kv.first))
                p->setValueNotifyingHost(
                    p->getNormalisableRange().convertTo0to1(kv.second));
        }
        return true;
    }
    else
    {
        return loadPresetFromFile(info.file);
    }
}

//==============================================================================
// loadPresetFromFile
//==============================================================================
bool PresetManager::loadPresetFromFile(const juce::File& file)
{
    if (!file.existsAsFile()) return false;
    auto xml = juce::XmlDocument::parse(file);
    if (xml == nullptr) return false;
    return applyPresetXml(*xml);
}

//==============================================================================
// saveUserPreset
//==============================================================================
juce::File PresetManager::saveUserPreset(const juce::String& name,
    const juce::String& category,
    const juce::String& author,
    bool allowOverwrite)
{
    auto dir = getUserPresetDirectory();
    if (!dir.isDirectory())
        dir.createDirectory();

    auto safeName = juce::File::createLegalFileName(name);
    auto target = dir.getChildFile(safeName + ".echoforge");

    if (target.existsAsFile() && !allowOverwrite)
        return {};

    if (auto xml = writePresetXml(name, category, author))
        xml->writeTo(target);

    refresh();
    return target;
}

//==============================================================================
// exportPresetToFile
//==============================================================================
bool PresetManager::exportPresetToFile(const juce::File& destination,
    const juce::String& name,
    const juce::String& category,
    const juce::String& author)
{
    if (auto xml = writePresetXml(name, category, author))
        return xml->writeTo(destination);
    return false;
}

//==============================================================================
// deletePreset
//==============================================================================
bool PresetManager::deletePreset(int index)
{
    if (index < 0 || index >= (int)presets.size()) return false;
    const auto& info = presets[(size_t)index];
    if (info.isFactory) return false;

    if (info.file.existsAsFile())
    {
        info.file.deleteFile();
        refresh();
        return true;
    }
    return false;
}

//==============================================================================
// getUserPresetDirectory
//==============================================================================
juce::File PresetManager::getUserPresetDirectory() const
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("EchoForge")
        .getChildFile("Presets");
}

//==============================================================================
// XML serialization
//==============================================================================
void PresetManager::writeParametersToXml(juce::XmlElement& parent) const
{
    auto* paramsXml = parent.createNewChildElement("PARAMETERS");
    for (auto* param : apvts.processor.getParameters())
    {
        if (auto* p = dynamic_cast<juce::RangedAudioParameter*>(param))
        {
            auto* pXml = paramsXml->createNewChildElement("PARAM");
            pXml->setAttribute("id", p->getParameterID());
            pXml->setAttribute("value", (double)p->getValue());
        }
    }
}

void PresetManager::resetAllToDefaults()
{
    for (auto* param : apvts.processor.getParameters())
        if (auto* p = dynamic_cast<juce::RangedAudioParameter*>(param))
            p->setValueNotifyingHost(p->getDefaultValue());
}

void PresetManager::applyParametersFromXml(const juce::XmlElement& parent)
{
    if (auto* paramsXml = parent.getChildByName("PARAMETERS"))
    {
        for (auto* pXml : paramsXml->getChildWithTagNameIterator("PARAM"))
        {
            auto id = pXml->getStringAttribute("id");
            auto val = (float)pXml->getDoubleAttribute("value");
            if (auto* p = apvts.getParameter(id))
                p->setValueNotifyingHost(val);
        }
    }
}

//==============================================================================
// XML write / read
//==============================================================================
std::unique_ptr<juce::XmlElement> PresetManager::writePresetXml(
    const juce::String& name,
    const juce::String& category,
    const juce::String& author) const
{
    auto xml = std::make_unique<juce::XmlElement>("ECHOFORGE_PRESET");
    xml->setAttribute("version", 1);
    xml->setAttribute("name", name);
    xml->setAttribute("category", category);
    xml->setAttribute("author", author);
    writeParametersToXml(*xml);
    return xml;
}

bool PresetManager::applyPresetXml(const juce::XmlElement& xml)
{
    if (!xml.hasTagName("ECHOFORGE_PRESET")) return false;
    resetAllToDefaults();
    applyParametersFromXml(xml);
    return true;
}

bool PresetManager::readUserPresetMetadata(const juce::File& file,
    juce::String& name,
    juce::String& category,
    juce::String& author) const
{
    auto xml = juce::XmlDocument::parse(file);
    if (xml == nullptr) return false;
    if (!xml->hasTagName("ECHOFORGE_PRESET")) return false;

    name = xml->getStringAttribute("name", file.getFileNameWithoutExtension());
    category = xml->getStringAttribute("category", "User");
    author = xml->getStringAttribute("author", "User");
    return true;
}