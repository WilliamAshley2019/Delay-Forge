#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

//==============================================================================
// Colour palette — "Echo Forge"
//==============================================================================
namespace EchoColour
{
    inline const juce::Colour BG{ 0xFF0A0C10 };
    inline const juce::Colour Panel{ 0xFF161A22 };
    inline const juce::Colour PanelBdr{ 0xFF242A36 };
    inline const juce::Colour Header{ 0xFF050608 };
    inline const juce::Colour ListBg{ 0xFF0E1116 };
    inline const juce::Colour ListRow{ 0xFF141820 };
    inline const juce::Colour ListRowAlt{ 0xFF181C24 };
    inline const juce::Colour ListRowHover{ 0xFF202634 };
    inline const juce::Colour ListRowSelected{ 0xFF2A3448 };

    // Primary control colours
    inline const juce::Colour Orange{ 0xFFFFC080 };     // CENTER
    inline const juce::Colour OrangeDim{ 0xFF6A4A20 };
    inline const juce::Colour Cyan{ 0xFF60C0FF };       // WIDTH
    inline const juce::Colour CyanDim{ 0xFF205070 };
    inline const juce::Colour Purple{ 0xFFC060D0 };     // HEIGHT
    inline const juce::Colour PurpleDim{ 0xFF502060 };
    inline const juce::Colour Yellow{ 0xFFFFD070 };     // BUFFER
    inline const juce::Colour YellowDim{ 0xFF6A5820 };
    inline const juce::Colour Green{ 0xFF70D080 };      // Feedback / mix
    inline const juce::Colour GreenDim{ 0xFF2A5028 };
    inline const juce::Colour Steel{ 0xFFA0A8B4 };      // Utility
    inline const juce::Colour Magenta{ 0xFFFF4080 };    // Scope / display
    inline const juce::Colour IvoryText{ 0xFFD8DDE8 };
    inline const juce::Colour DimText{ 0xFF6A7080 };
    inline const juce::Colour White{ 0xFFFFFFFF };
}

//==============================================================================
// LookAndFeel
//==============================================================================
class EchoLAF : public juce::LookAndFeel_V4
{
public:
    EchoLAF();

    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h,
        float sliderPos, float startAng, float endAng,
        juce::Slider&) override;

    void drawToggleButton(juce::Graphics&, juce::ToggleButton&,
        bool highlight, bool down) override;

    void drawComboBox(juce::Graphics&, int width, int height, bool isButtonDown,
        int buttonX, int buttonY, int buttonW, int buttonH,
        juce::ComboBox&) override;

    juce::Font getComboBoxFont(juce::ComboBox&) override;

    static juce::Font monoFont(float size);
    static juce::Font displayFont(float size);
};

//==============================================================================
// Labelled knob
//==============================================================================
class EchoKnob : public juce::Component
{
public:
    EchoKnob(const juce::String& label, const juce::String& paramID,
        juce::AudioProcessorValueTreeState& apvts,
        juce::Colour accent = EchoColour::Orange,
        bool compact = false);

    void resized() override;
    void paint(juce::Graphics&) override;

    juce::Slider slider;

private:
    juce::Label nameLabel, valueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    juce::String paramID;
    juce::AudioProcessorValueTreeState& apvtsRef;
    juce::Colour accent;
    bool compact;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EchoKnob)
};

//==============================================================================
// Section panel
//==============================================================================
class EchoSection : public juce::Component
{
public:
    EchoSection(const juce::String& titleText, juce::Colour accentCol);
    void paint(juce::Graphics&) override;

    juce::String title;
    juce::Colour accent;
};

//==============================================================================
// Delay scope
//
// A horizontal bar representing the full 2-second buffer. Two vertical
// markers show the current L and R tap positions as they move. Their
// separation visually represents the WIDTH knob. The active portion (set by
// BUFFER) is highlighted as a brighter region; the idle tail is dimmed.
//
// The bottom quarter of the display shows a decay trail — the feedback
// repeats as they decay over time, running off to the right. This gives a
// sense of the "size" of the delay at a glance.
//==============================================================================
class DelayScope : public juce::Component,
    private juce::Timer
{
public:
    DelayScope(EchoForgeProcessor& p);
    ~DelayScope() override;

    void paint(juce::Graphics&) override;
    void timerCallback() override;

private:
    EchoForgeProcessor& proc;

    // Smoothed marker positions for a natural-feeling motion
    float smoothedL = 0.3f;
    float smoothedR = 0.3f;
    float smoothedFb = 0.f;

    // Feedback trail history
    static constexpr int kTrailSize = 128;
    std::array<float, kTrailSize> trail{};
    int trailIdx = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DelayScope)
};

//==============================================================================
// Level meter
//==============================================================================
class EchoMeter : public juce::Component,
    private juce::Timer
{
public:
    EchoMeter(EchoForgeProcessor& p);
    ~EchoMeter() override;
    void paint(juce::Graphics&) override;
    void timerCallback() override;

private:
    EchoForgeProcessor& proc;
    float dispInL = 0.f, dispInR = 0.f;
    float dispOutL = 0.f, dispOutR = 0.f;
    float dispFb = 0.f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EchoMeter)
};

//==============================================================================
// Preset browser — right-side list panel inside a Viewport
//==============================================================================
class PresetBrowser : public juce::Component,
    private juce::Timer
{
public:
    PresetBrowser(EchoForgeProcessor& p);
    ~PresetBrowser() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    void refresh();
    std::function<void(int)> onPresetSelected;
    std::function<void()>    onSaveRequested;

    int getSelectedIndex() const { return selectedIndex; }
    void setSelectedIndexSilent(int index);

private:
    class PresetList;

    EchoForgeProcessor& proc;
    std::unique_ptr<PresetList> list;
    juce::Viewport listViewport;
    std::unique_ptr<juce::TextEditor> searchField;
    std::unique_ptr<juce::TextButton> saveButton;
    std::unique_ptr<juce::TextButton> deleteButton;
    std::unique_ptr<juce::TextButton> exportButton;
    std::unique_ptr<juce::TextButton> folderButton;
    std::unique_ptr<juce::Label> headerLabel;

    struct Row
    {
        juce::String display;
        juce::String name;
        juce::String category;
        int presetIndex = -1;
        bool isHeader = false;
        bool isUser = false;
        juce::Rectangle<int> bounds;
    };
    std::vector<Row> rows;
    int selectedIndex = -1;
    int lastKnownManagerIndex = -1;

    void rebuildRows(const juce::String& filterText);
    void handleRowClick(int presetIndex);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBrowser)
};

//==============================================================================
// Main editor
//==============================================================================
class EchoForgeEditor : public juce::AudioProcessorEditor,
    private juce::Timer
{
public:
    EchoForgeEditor(EchoForgeProcessor&);
    ~EchoForgeEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    EchoForgeProcessor& audioProcessor;
    EchoLAF laf;

    //==========================================================================
    // Displays
    //==========================================================================
    std::unique_ptr<DelayScope> delayScope;
    std::unique_ptr<EchoMeter>  meter;

    //==========================================================================
    // Sections
    //==========================================================================
    EchoSection secPrimary{ "PRIMARY", EchoColour::Orange };
    EchoSection secEngine{ "ENGINE", EchoColour::Steel };
    EchoSection secTone{ "TONE", EchoColour::Purple };
    EchoSection secMod{ "MODULATION", EchoColour::Cyan };
    EchoSection secPingPong{ "PING-PONG", EchoColour::Magenta };
    EchoSection secOutput{ "OUTPUT", EchoColour::Green };

    //==========================================================================
    // Primary controls
    //==========================================================================
    std::unique_ptr<EchoKnob> centerKnob;
    std::unique_ptr<EchoKnob> widthKnob;
    std::unique_ptr<EchoKnob> heightKnob;
    std::unique_ptr<EchoKnob> bufferKnob;
    std::unique_ptr<EchoKnob> feedbackKnob;
    std::unique_ptr<EchoKnob> mixKnob;

    //==========================================================================
    // Engine
    //==========================================================================
    juce::ComboBox engineCombo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> engineAttach;

    // BBD
    std::unique_ptr<EchoKnob> bbdDriveKnob;
    std::unique_ptr<EchoKnob> bbdChargeKnob;
    std::unique_ptr<EchoKnob> bbdLeakKnob;
    std::unique_ptr<EchoKnob> bbdStutterKnob;

    // Tape
    std::unique_ptr<EchoKnob> tapeWowKnob;
    std::unique_ptr<EchoKnob> tapeFlutterKnob;
    std::unique_ptr<EchoKnob> tapeSatKnob;
    std::unique_ptr<EchoKnob> tapeHeadKnob;

    // Diffuse
    std::unique_ptr<EchoKnob> diffSpreadKnob;
    std::unique_ptr<EchoKnob> diffTapsKnob;
    std::unique_ptr<EchoKnob> diffDampKnob;

    //==========================================================================
    // Time (secondary)
    //==========================================================================
    std::unique_ptr<EchoKnob> timeTrimKnob;
    juce::ToggleButton syncBtn{ "SYNC" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> syncAttach;
    juce::ComboBox syncDivCombo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> syncDivAttach;
    juce::TextButton tapTempoBtn{ "TAP" };

    //==========================================================================
    // Tone
    //==========================================================================
    std::unique_ptr<EchoKnob> heightTrimKnob;
    std::unique_ptr<EchoKnob> hpTrimKnob;
    std::unique_ptr<EchoKnob> lpTrimKnob;

    //==========================================================================
    // Modulation
    //==========================================================================
    std::unique_ptr<EchoKnob> modRateKnob;
    std::unique_ptr<EchoKnob> modDepthKnob;
    juce::ComboBox modShapeCombo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modShapeAttach;

    //==========================================================================
    // Ping-Pong
    //==========================================================================
    juce::ComboBox pingPongCombo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> pingPongAttach;
    std::unique_ptr<EchoKnob> crossFeedKnob;

    //==========================================================================
    // Output
    //==========================================================================
    std::unique_ptr<EchoKnob> outputKnob;
    std::unique_ptr<EchoKnob> outWidthKnob;
    juce::ToggleButton bypassBtn{ "BYPASS" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttach;

    //==========================================================================
    // A/B
    //==========================================================================
    juce::TextButton abButton{ "A/B" };
    juce::TextButton captureBtn{ "CAP A" };

    //==========================================================================
    // Preset browser
    //==========================================================================
    std::unique_ptr<PresetBrowser> presetBrowser;

    //==========================================================================
    // Helpers
    //==========================================================================
    void drawHeader(juce::Graphics&, juce::Rectangle<int>);
    void setupCombo(juce::ComboBox&, const juce::StringArray&);

    void onProcessorPresetChanged(int newIndex);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EchoForgeEditor)
};