#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "PresetManager.h"

//==============================================================================
// EchoLAF
//==============================================================================
EchoLAF::EchoLAF()
{
    setColour(juce::Slider::thumbColourId, EchoColour::Orange);
    setColour(juce::Slider::rotarySliderFillColourId, EchoColour::Orange);
    setColour(juce::Slider::trackColourId, EchoColour::OrangeDim);
    setColour(juce::Label::textColourId, EchoColour::IvoryText);
    setColour(juce::ComboBox::backgroundColourId, EchoColour::Panel);
    setColour(juce::ComboBox::textColourId, EchoColour::IvoryText);
    setColour(juce::ComboBox::outlineColourId, EchoColour::PanelBdr);
    setColour(juce::ComboBox::arrowColourId, EchoColour::Orange);
    setColour(juce::PopupMenu::backgroundColourId, EchoColour::Panel);
    setColour(juce::PopupMenu::textColourId, EchoColour::IvoryText);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, EchoColour::OrangeDim);
    setColour(juce::TextButton::buttonColourId, EchoColour::Panel);
    setColour(juce::TextButton::textColourOffId, EchoColour::Orange);
    setColour(juce::TextButton::textColourOnId, EchoColour::BG);
    setColour(juce::ToggleButton::textColourId, EchoColour::IvoryText);
    setColour(juce::ToggleButton::tickDisabledColourId, EchoColour::DimText);
    setColour(juce::ToggleButton::tickColourId, EchoColour::Orange);
    setColour(juce::TextEditor::backgroundColourId, EchoColour::ListBg);
    setColour(juce::TextEditor::textColourId, EchoColour::IvoryText);
    setColour(juce::TextEditor::outlineColourId, EchoColour::PanelBdr);
    setColour(juce::TextEditor::focusedOutlineColourId, EchoColour::Orange);
    setColour(juce::TextEditor::highlightColourId, EchoColour::OrangeDim);
    setColour(juce::ScrollBar::thumbColourId, EchoColour::PanelBdr);
}

juce::Font EchoLAF::monoFont(float size)
{
    return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(),
        size, juce::Font::plain));
}

juce::Font EchoLAF::displayFont(float size)
{
    return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(),
        size, juce::Font::bold));
}

void EchoLAF::drawRotarySlider(juce::Graphics& g,
    int x, int y, int w, int h,
    float sliderPos, float startAng, float endAng,
    juce::Slider& slider)
{
    const float radius = (float)juce::jmin(w, h) * 0.5f - 3.0f;
    const float centreX = (float)x + (float)w * 0.5f;
    const float centreY = (float)y + (float)h * 0.5f;
    const float rx = centreX - radius;
    const float ry = centreY - radius;
    const float rw = radius * 2.0f;
    const float angle = startAng + sliderPos * (endAng - startAng);

    g.setColour(EchoColour::BG);
    g.fillEllipse(rx, ry, rw, rw);
    g.setColour(EchoColour::PanelBdr);
    g.drawEllipse(rx, ry, rw, rw, 1.5f);

    const juce::Colour accentCol = slider.findColour(juce::Slider::rotarySliderFillColourId);

    const bool bipolar = slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0;
    const float zeroAngle = bipolar ? (startAng + endAng) * 0.5f : startAng;

    {
        juce::Path trackArc;
        trackArc.addArc(rx + 2.0f, ry + 2.0f, rw - 4.0f, rw - 4.0f,
            startAng, endAng, true);
        g.setColour(accentCol.darker(1.4f));
        g.strokePath(trackArc, juce::PathStrokeType(2.5f,
            juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    {
        juce::Path fillArc;
        if (bipolar)
        {
            const float a1 = juce::jmin(zeroAngle, angle);
            const float a2 = juce::jmax(zeroAngle, angle);
            fillArc.addArc(rx + 2.0f, ry + 2.0f, rw - 4.0f, rw - 4.0f,
                a1, a2, true);
        }
        else
        {
            fillArc.addArc(rx + 2.0f, ry + 2.0f, rw - 4.0f, rw - 4.0f,
                startAng, angle, true);
        }
        g.setColour(accentCol);
        g.strokePath(fillArc, juce::PathStrokeType(2.5f,
            juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    {
        juce::Path pointer;
        const float pointerLen = radius * 0.55f;
        const float pointerW = 2.0f;
        pointer.addRectangle(-pointerW * 0.5f, -radius + 3.0f, pointerW, pointerLen);
        pointer.applyTransform(juce::AffineTransform::rotation(angle)
            .translated(centreX, centreY));
        g.setColour(EchoColour::White);
        g.fillPath(pointer);
    }
    g.setColour(accentCol);
    g.fillEllipse(centreX - 2.5f, centreY - 2.5f, 5.0f, 5.0f);
}

void EchoLAF::drawToggleButton(juce::Graphics& g, juce::ToggleButton& btn,
    bool, bool)
{
    const bool isOn = btn.getToggleState();
    const auto bounds = btn.getLocalBounds().toFloat();

    const float rx = 4.0f, h = bounds.getHeight() - 8.0f;
    juce::Rectangle<float> pill(rx, 4.0f, h * 2.0f, h);
    g.setColour(isOn ? EchoColour::Orange : EchoColour::PanelBdr);
    g.fillRoundedRectangle(pill, h * 0.5f);

    const float thumbX = isOn ? pill.getRight() - h - 2.0f : pill.getX() + 2.0f;
    g.setColour(isOn ? EchoColour::BG : EchoColour::DimText);
    g.fillEllipse(thumbX, pill.getY() + 2.0f, h - 4.0f, h - 4.0f);

    g.setColour(isOn ? EchoColour::IvoryText : EchoColour::DimText);
    g.setFont(monoFont(11.0f));
    g.drawText(btn.getButtonText(),
        (int)pill.getRight() + 6, 0,
        btn.getWidth() - (int)pill.getRight() - 6, btn.getHeight(),
        juce::Justification::centredLeft, true);
}

void EchoLAF::drawComboBox(juce::Graphics& g, int width, int height, bool,
    int buttonX, int buttonY, int buttonW, int buttonH,
    juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();
    g.setColour(EchoColour::Panel);
    g.fillRoundedRectangle(bounds, 3.0f);
    g.setColour(EchoColour::PanelBdr);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 3.0f, 1.0f);

    const float ax = (float)(buttonX + buttonW / 2);
    const float ay = (float)(buttonY + buttonH / 2);
    juce::Path arrow;
    arrow.addTriangle(ax - 4.f, ay - 2.f, ax + 4.f, ay - 2.f, ax, ay + 4.f);
    g.setColour(box.findColour(juce::ComboBox::arrowColourId));
    g.fillPath(arrow);

    // No text — ComboBox draws its selection through its own child Label
}

juce::Font EchoLAF::getComboBoxFont(juce::ComboBox&)
{
    return monoFont(10.5f);
}

//==============================================================================
// EchoKnob
//==============================================================================
EchoKnob::EchoKnob(const juce::String& labelStr, const juce::String& pid,
    juce::AudioProcessorValueTreeState& apvts, juce::Colour accentCol,
    bool compactMode)
    : paramID(pid), apvtsRef(apvts), accent(accentCol), compact(compactMode)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setPopupDisplayEnabled(true, false, nullptr);
    slider.setColour(juce::Slider::rotarySliderFillColourId, accentCol);
    slider.setColour(juce::Slider::thumbColourId, accentCol);
    addAndMakeVisible(slider);

    nameLabel.setText(labelStr, juce::dontSendNotification);
    nameLabel.setJustificationType(juce::Justification::centred);
    nameLabel.setColour(juce::Label::textColourId, EchoColour::DimText);
    nameLabel.setFont(EchoLAF::monoFont(10.0f));
    addAndMakeVisible(nameLabel);

    valueLabel.setJustificationType(juce::Justification::centred);
    valueLabel.setColour(juce::Label::textColourId, accentCol);
    valueLabel.setFont(EchoLAF::monoFont(10.0f));
    addAndMakeVisible(valueLabel);
    nameLabel.setVisible(!compact);
    valueLabel.setVisible(!compact);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, pid, slider);

    slider.onValueChange = [this]()
        {
            if (auto* param = apvtsRef.getParameter(paramID))
                valueLabel.setText(param->getText(param->getValue(), 6),
                    juce::dontSendNotification);
        };
    slider.onValueChange();
}

void EchoKnob::resized()
{
    auto b = getLocalBounds();
    if (compact)
    {
        slider.setBounds(b);
        return;
    }
    const int labelH = 14;
    nameLabel.setBounds(b.removeFromTop(labelH));
    valueLabel.setBounds(b.removeFromBottom(labelH));
    slider.setBounds(b.reduced(2));
}

void EchoKnob::paint(juce::Graphics&) {}

//==============================================================================
// EchoSection
//==============================================================================
EchoSection::EchoSection(const juce::String& titleText, juce::Colour accentCol)
    : title(titleText), accent(accentCol)
{
}

void EchoSection::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();

    g.setColour(EchoColour::Panel);
    g.fillRoundedRectangle(b, 6.0f);

    g.setColour(accent.withAlpha(0.5f));
    g.drawRoundedRectangle(b.reduced(0.5f), 6.0f, 1.0f);

    juce::Rectangle<float> hdr(b.getX(), b.getY(), b.getWidth(), 20.0f);
    g.setColour(EchoColour::Header);
    g.fillRoundedRectangle(hdr, 6.0f);
    g.fillRect(hdr.withTop(hdr.getBottom() - 4.0f));

    g.setColour(accent);
    g.setFont(EchoLAF::displayFont(10.5f));
    g.drawText(title, hdr.toNearestInt(), juce::Justification::centred, false);

    g.setColour(accent.withAlpha(0.85f));
    g.fillRect(b.getX() + 8.0f, b.getY() + 19.0f, b.getWidth() - 16.0f, 1.5f);
}

//==============================================================================
// DelayScope
//==============================================================================
DelayScope::DelayScope(EchoForgeProcessor& p) : proc(p)
{
    trail.fill(0.f);
    startTimerHz(30);
}

DelayScope::~DelayScope()
{
    stopTimer();
}

void DelayScope::timerCallback()
{
    using namespace EchoParams;
    static constexpr auto rlx = std::memory_order_relaxed;

    // Normalised tap positions come from the snapshot (already in ms / buffer)
    const float activeMs = proc.snapshot.activeBufferMs.load(rlx);
    const float liveMsL = proc.snapshot.liveDelayL.load(rlx);
    const float liveMsR = proc.snapshot.liveDelayR.load(rlx);

    const float normL = (activeMs > 1.f) ? (liveMsL / activeMs) : 0.f;
    const float normR = (activeMs > 1.f) ? (liveMsR / activeMs) : 0.f;

    smoothedL = smoothedL * 0.7f + normL * 0.3f;
    smoothedR = smoothedR * 0.7f + normR * 0.3f;

    // Feedback meter drives the trail brightness
    const float fb = proc.feedbackAmount.load(rlx);
    smoothedFb = smoothedFb * 0.8f + fb * 0.2f;

    trail[(size_t)trailIdx] = smoothedFb;
    trailIdx = (trailIdx + 1) % kTrailSize;

    repaint();
}

void DelayScope::paint(juce::Graphics& g)
{
    using namespace EchoParams;
    static constexpr auto rlx = std::memory_order_relaxed;

    auto b = getLocalBounds().toFloat();

    // Background
    g.setColour(EchoColour::BG);
    g.fillRoundedRectangle(b, 4.0f);
    g.setColour(EchoColour::PanelBdr);
    g.drawRoundedRectangle(b.reduced(0.5f), 4.0f, 1.0f);

    // Header
    auto hdr = b.removeFromTop(18.0f);
    g.setColour(EchoColour::Header);
    g.fillRect(hdr);
    g.setColour(EchoColour::Magenta);
    g.setFont(EchoLAF::displayFont(10.0f));
    g.drawText("DELAY SCOPE", hdr.toNearestInt(),
        juce::Justification::centred, false);

    // Split into top (buffer) and bottom (trail)
    auto content = b.reduced(8.0f, 6.0f);
    const float trailH = content.getHeight() * 0.28f;
    auto bufferArea = content.withTrimmedBottom(trailH + 4.0f);
    auto trailArea = content.withTop(content.getBottom() - trailH);

    // --- Buffer region ---
    // The active portion of the buffer is drawn brighter; the rest is dim.
    const float activePct = juce::jlimit(0.f, 1.f,
        proc.snapshot.buffer.load(rlx));
    const float activeWidth = bufferArea.getWidth() * (0.125f + activePct * 0.875f);
    auto activeArea = bufferArea.withWidth(activeWidth);

    g.setColour(EchoColour::PanelBdr.withAlpha(0.35f));
    g.fillRoundedRectangle(bufferArea, 2.0f);
    g.setColour(EchoColour::Panel.withAlpha(0.75f));
    g.fillRoundedRectangle(activeArea, 2.0f);

    // Grid lines every quarter of the active region
    g.setColour(EchoColour::PanelBdr.withAlpha(0.5f));
    for (int i = 1; i < 4; ++i)
    {
        const float x = activeArea.getX()
            + activeArea.getWidth() * (float)i / 4.0f;
        g.drawVerticalLine((int)x, activeArea.getY(), activeArea.getBottom());
    }

    // --- Tap markers ---
    auto drawMarker = [&](float norm, juce::Colour colour, const juce::String& label)
        {
            const float x = activeArea.getX() + norm * activeArea.getWidth();

            // Glow
            g.setColour(colour.withAlpha(0.35f));
            g.fillRect(x - 3.5f, bufferArea.getY(), 7.0f, bufferArea.getHeight());
            // Core
            g.setColour(colour);
            g.fillRect(x - 1.0f, bufferArea.getY(), 2.0f, bufferArea.getHeight());

            // Label above the marker
            g.setColour(colour);
            g.setFont(EchoLAF::monoFont(8.0f));
            g.drawText(label, (int)x - 20, (int)bufferArea.getY() - 11,
                40, 10, juce::Justification::centred, false);
        };

    drawMarker(smoothedL, EchoColour::Cyan, "L");
    drawMarker(smoothedR, EchoColour::Orange, "R");

    // --- Centre label ---
    const float liveMsL = proc.snapshot.liveDelayL.load(rlx);
    const float liveMsR = proc.snapshot.liveDelayR.load(rlx);
    const int engine = proc.snapshot.engine.load(rlx);
    static const char* engineNames[] = { "SIMPLE", "BBD", "TAPE", "DIFFUSE" };

    g.setFont(EchoLAF::monoFont(8.5f));
    g.setColour(EchoColour::DimText);
    g.drawText(juce::String(engineNames[juce::jlimit(0, 3, engine)])
        + "  L " + juce::String(liveMsL, 0) + "ms"
        + "  R " + juce::String(liveMsR, 0) + "ms",
        bufferArea.toNearestInt().removeFromBottom(12),
        juce::Justification::centredRight, false);

    // --- Feedback trail ---
    g.setColour(EchoColour::PanelBdr.withAlpha(0.35f));
    g.fillRoundedRectangle(trailArea, 2.0f);

    juce::Path trailPath;
    bool started = false;
    for (int i = 0; i < kTrailSize; ++i)
    {
        const int idx = (trailIdx + i) % kTrailSize;
        const float v = juce::jlimit(0.f, 1.f, trail[(size_t)idx]);
        const float x = trailArea.getX()
            + (float)i / (float)(kTrailSize - 1) * trailArea.getWidth();
        const float y = trailArea.getBottom() - v * trailArea.getHeight();
        if (!started) { trailPath.startNewSubPath(x, y); started = true; }
        else trailPath.lineTo(x, y);
    }

    g.setColour(EchoColour::Green.withAlpha(0.4f));
    g.strokePath(trailPath, juce::PathStrokeType(3.0f,
        juce::PathStrokeType::curved));
    g.setColour(EchoColour::Green);
    g.strokePath(trailPath, juce::PathStrokeType(1.5f,
        juce::PathStrokeType::curved));

    // Label
    g.setColour(EchoColour::DimText);
    g.setFont(EchoLAF::monoFont(8.0f));
    g.drawText("FEEDBACK", trailArea.toNearestInt().removeFromLeft(60),
        juce::Justification::centredLeft, false);
}

//==============================================================================
// EchoMeter
//==============================================================================
EchoMeter::EchoMeter(EchoForgeProcessor& p) : proc(p)
{
    startTimerHz(30);
}

EchoMeter::~EchoMeter()
{
    stopTimer();
}

void EchoMeter::timerCallback()
{
    const float inL = proc.levelInRmsL.load();
    const float inR = proc.levelInRmsR.load();
    const float outL = proc.levelPeakL.load();
    const float outR = proc.levelPeakR.load();
    const float fb = proc.feedbackAmount.load();

    dispInL = juce::jmax(dispInL * 0.8f, inL);
    dispInR = juce::jmax(dispInR * 0.8f, inR);
    dispOutL = juce::jmax(dispOutL * 0.8f, outL);
    dispOutR = juce::jmax(dispOutR * 0.8f, outR);
    dispFb = juce::jmax(dispFb * 0.85f, fb);

    repaint();
}

void EchoMeter::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour(EchoColour::BG);
    g.fillRoundedRectangle(b, 3.0f);
    g.setColour(EchoColour::PanelBdr);
    g.drawRoundedRectangle(b.reduced(0.5f), 3.0f, 1.0f);

    const int barW = (getWidth() - 8) / 4;

    auto drawBar = [&](int x, float lin, juce::Colour col)
        {
            const float db = 20.0f * std::log10(juce::jmax(1e-6f, lin));
            const float norm = juce::jlimit(0.0f, 1.0f, (db + 60.0f) / 60.0f);
            const int h = getHeight() - 14;
            const int filled = (int)(norm * (float)h);
            const int top = 4 + h - filled;

            g.setColour(col.darker(1.6f));
            g.fillRect(x, 4, barW - 1, h);
            g.setColour(col);
            g.fillRect(x, top, barW - 1, filled);
        };

    drawBar(4, dispInL, EchoColour::Steel.darker(0.4f));
    drawBar(4 + barW, dispInR, EchoColour::Steel.darker(0.4f));
    drawBar(4 + barW * 2, dispOutL, EchoColour::Magenta);
    drawBar(4 + barW * 3, dispOutR, EchoColour::Magenta);

    // FB overlay strip on top of the OUT bars
    {
        const float fbNorm = juce::jlimit(0.f, 1.f, dispFb);
        const int h = (int)(fbNorm * (getHeight() - 14));
        if (h > 0)
        {
            g.setColour(EchoColour::Green.withAlpha(0.55f));
            g.fillRect(4 + barW * 2, getHeight() - 10 - h, barW * 2 - 1, h);
        }
    }

    g.setFont(EchoLAF::monoFont(7.5f));
    g.setColour(EchoColour::DimText);
    g.drawText("IN L", 4, getHeight() - 10, barW, 10, juce::Justification::centred);
    g.drawText("IN R", 4 + barW, getHeight() - 10, barW, 10, juce::Justification::centred);
    g.drawText("OUT L", 4 + barW * 2, getHeight() - 10, barW, 10, juce::Justification::centred);
    g.drawText("OUT R", 4 + barW * 3, getHeight() - 10, barW, 10, juce::Justification::centred);
}

//==============================================================================
// PresetBrowser::PresetList
//==============================================================================
class PresetBrowser::PresetList : public juce::Component
{
public:
    PresetList(PresetBrowser& ownerRef) : owner(ownerRef) {}
    ~PresetList() override = default;

    void paint(juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        g.setColour(EchoColour::ListBg);
        g.fillRect(b);

        for (auto& row : owner.rows)
        {
            auto r = row.bounds;

            if (row.isHeader)
            {
                g.setColour(EchoColour::Header);
                g.fillRect(r);
                g.setColour(EchoColour::Orange.withAlpha(0.75f));
                g.setFont(EchoLAF::displayFont(9.5f));
                g.drawText(row.display.toUpperCase(), r.reduced(8, 0),
                    juce::Justification::centredLeft, false);
            }
            else
            {
                const bool selected = (row.presetIndex == owner.selectedIndex);
                const bool hovered = (row.presetIndex == hoveredIndex);
                juce::Colour bg = row.isUser ? EchoColour::ListRowAlt
                    : EchoColour::ListRow;
                if (selected) bg = EchoColour::ListRowSelected;
                else if (hovered) bg = EchoColour::ListRowHover;

                g.setColour(bg);
                g.fillRect(r);

                g.setColour(EchoColour::PanelBdr.withAlpha(0.4f));
                g.drawHorizontalLine(r.getBottom() - 1,
                    (float)r.getX(), (float)r.getRight());

                if (selected)
                {
                    g.setColour(EchoColour::Orange);
                    g.fillRect(r.getX(), r.getY(), 3, r.getHeight());
                }

                g.setColour(selected ? EchoColour::White : EchoColour::IvoryText);
                g.setFont(EchoLAF::monoFont(11.0f));
                g.drawText(row.name,
                    r.reduced(12, 0),
                    juce::Justification::centredLeft, true);

                if (row.isUser)
                {
                    g.setColour(EchoColour::Orange.withAlpha(0.7f));
                    g.setFont(EchoLAF::monoFont(8.5f));
                    g.drawText("\u2022",
                        r.removeFromRight(16).reduced(2, 0),
                        juce::Justification::centred, false);
                }
            }
        }
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        auto pos = e.getPosition();
        for (auto& row : owner.rows)
        {
            if (row.isHeader) continue;
            if (row.bounds.contains(pos))
            {
                owner.handleRowClick(row.presetIndex);
                return;
            }
        }
    }

    void mouseMove(const juce::MouseEvent& e) override
    {
        int newHover = -1;
        auto pos = e.getPosition();
        for (auto& row : owner.rows)
        {
            if (row.isHeader) continue;
            if (row.bounds.contains(pos)) { newHover = row.presetIndex; break; }
        }
        if (newHover != hoveredIndex)
        {
            hoveredIndex = newHover;
            repaint();
        }
    }

    void mouseExit(const juce::MouseEvent&) override
    {
        if (hoveredIndex != -1) { hoveredIndex = -1; repaint(); }
    }

private:
    PresetBrowser& owner;
    int hoveredIndex = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetList)
};

//==============================================================================
// PresetBrowser
//==============================================================================
PresetBrowser::PresetBrowser(EchoForgeProcessor& p) : proc(p)
{
    headerLabel = std::make_unique<juce::Label>();
    headerLabel->setText("PRESETS", juce::dontSendNotification);
    headerLabel->setFont(EchoLAF::displayFont(11.0f));
    headerLabel->setColour(juce::Label::textColourId, EchoColour::Orange);
    headerLabel->setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(*headerLabel);

    searchField = std::make_unique<juce::TextEditor>();
    searchField->setTextToShowWhenEmpty("Search...", EchoColour::DimText);
    searchField->setFont(EchoLAF::monoFont(11.0f));
    searchField->onTextChange = [this]() {
        rebuildRows(searchField->getText());
        };
    addAndMakeVisible(*searchField);

    list = std::make_unique<PresetList>(*this);
    listViewport.setViewedComponent(list.get(), false);
    listViewport.setScrollBarsShown(true, false);
    listViewport.setScrollBarThickness(6);
    addAndMakeVisible(listViewport);

    saveButton = std::make_unique<juce::TextButton>("SAVE");
    saveButton->setColour(juce::TextButton::buttonColourId, EchoColour::Panel);
    saveButton->setColour(juce::TextButton::textColourOffId, EchoColour::Orange);
    saveButton->onClick = [this]() { if (onSaveRequested) onSaveRequested(); };
    addAndMakeVisible(*saveButton);

    deleteButton = std::make_unique<juce::TextButton>("DEL");
    deleteButton->setColour(juce::TextButton::buttonColourId, EchoColour::Panel);
    deleteButton->setColour(juce::TextButton::textColourOffId, EchoColour::Orange);
    deleteButton->onClick = [this]() {
        if (selectedIndex < 0) return;
        auto& mgr = proc.getPresetManager();
        if (selectedIndex < mgr.getNumPresets())
        {
            const auto& info = mgr.getPresetInfo(selectedIndex);
            if (info.isFactory)
            {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::MessageBoxIconType::InfoIcon,
                    "Cannot Delete",
                    "Factory presets cannot be deleted.");
                return;
            }
            mgr.deletePreset(selectedIndex);
            refresh();
        }
        };
    addAndMakeVisible(*deleteButton);

    exportButton = std::make_unique<juce::TextButton>("EXP");
    exportButton->setColour(juce::TextButton::buttonColourId, EchoColour::Panel);
    exportButton->setColour(juce::TextButton::textColourOffId, EchoColour::Orange);
    exportButton->onClick = [this]() {
        if (selectedIndex < 0) return;
        auto& mgr = proc.getPresetManager();
        if (selectedIndex >= mgr.getNumPresets()) return;
        const auto& info = mgr.getPresetInfo(selectedIndex);

        auto chooser = std::make_shared<juce::FileChooser>(
            "Export Preset",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
            .getChildFile(info.name + ".echoforge"),
            "*.echoforge");
        chooser->launchAsync(
            juce::FileBrowserComponent::saveMode |
            juce::FileBrowserComponent::warnAboutOverwriting,
            [chooser, this](const juce::FileChooser& fc)
            {
                auto f = fc.getResult();
                if (f == juce::File()) return;
                if (!f.hasFileExtension("echoforge"))
                    f = f.withFileExtension("echoforge");
                auto& m = proc.getPresetManager();
                if (selectedIndex < m.getNumPresets())
                {
                    const auto& i = m.getPresetInfo(selectedIndex);
                    m.exportPresetToFile(f, i.name, i.category, i.author);
                }
            });
        };
    addAndMakeVisible(*exportButton);

    folderButton = std::make_unique<juce::TextButton>("DIR");
    folderButton->setColour(juce::TextButton::buttonColourId, EchoColour::Panel);
    folderButton->setColour(juce::TextButton::textColourOffId, EchoColour::Orange);
    folderButton->onClick = [this]() {
        proc.getPresetManager().getUserPresetDirectory().revealToUser();
        };
    addAndMakeVisible(*folderButton);

    refresh();
    startTimerHz(10);
}

PresetBrowser::~PresetBrowser()
{
    stopTimer();
    listViewport.setViewedComponent(nullptr, false);
}

void PresetBrowser::timerCallback()
{
    const int current = proc.getCurrentPresetIndex();
    if (current != lastKnownManagerIndex)
    {
        lastKnownManagerIndex = current;
        setSelectedIndexSilent(current);
    }
}

void PresetBrowser::refresh()
{
    rebuildRows(searchField ? searchField->getText() : juce::String());
    const int current = proc.getCurrentPresetIndex();
    if (current >= 0)
        setSelectedIndexSilent(current);
}

void PresetBrowser::setSelectedIndexSilent(int index)
{
    selectedIndex = index;
    if (list) list->repaint();
}

void PresetBrowser::rebuildRows(const juce::String& filterText)
{
    rows.clear();

    auto& mgr = proc.getPresetManager();
    const auto& all = mgr.getAllPresets();

    juce::String lastCategory;
    const juce::String filter = filterText.trim().toLowerCase();

    for (size_t i = 0; i < all.size(); ++i)
    {
        const auto& info = all[i];

        if (filter.isNotEmpty())
        {
            const bool match = info.name.toLowerCase().contains(filter)
                || info.category.toLowerCase().contains(filter);
            if (!match) continue;
        }

        if (info.category != lastCategory)
        {
            Row hdr;
            hdr.display = info.category;
            hdr.category = info.category;
            hdr.isHeader = true;
            rows.push_back(hdr);
            lastCategory = info.category;
        }

        Row r;
        r.display = info.name;
        r.name = info.name;
        r.category = info.category;
        r.presetIndex = (int)i;
        r.isHeader = false;
        r.isUser = !info.isFactory;
        rows.push_back(r);
    }

    int y = 0;
    const int headerH = 18;
    const int rowH = 22;
    const int listW = juce::jmax(10, listViewport.getWidth()
        - (listViewport.isVerticalScrollBarShown() ? 8 : 0));
    for (auto& row : rows)
    {
        const int h = row.isHeader ? headerH : rowH;
        row.bounds = juce::Rectangle<int>(0, y, listW, h);
        y += h;
    }

    if (list)
        list->setSize(listW, juce::jmax(y, 1));
}

void PresetBrowser::handleRowClick(int presetIndex)
{
    if (presetIndex < 0) return;
    selectedIndex = presetIndex;
    if (list) list->repaint();
    if (onPresetSelected) onPresetSelected(presetIndex);
}

void PresetBrowser::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour(EchoColour::Panel);
    g.fillRoundedRectangle(b, 6.0f);
    g.setColour(EchoColour::Orange.withAlpha(0.5f));
    g.drawRoundedRectangle(b.reduced(0.5f), 6.0f, 1.0f);

    juce::Rectangle<float> hdr(b.getX(), b.getY(), b.getWidth(), 20.0f);
    g.setColour(EchoColour::Header);
    g.fillRoundedRectangle(hdr, 6.0f);
    g.fillRect(hdr.withTop(hdr.getBottom() - 4.0f));

    g.setColour(EchoColour::Orange.withAlpha(0.85f));
    g.fillRect(b.getX() + 8.0f, b.getY() + 19.0f, b.getWidth() - 16.0f, 1.5f);
}

void PresetBrowser::resized()
{
    auto b = getLocalBounds().reduced(8);

    // Bottom buttons reserved first — this is the fix for the overlap
    auto btnArea = b.removeFromBottom(56);
    const int btnH = 22;
    const int btnW = btnArea.getWidth() / 2 - 2;

    auto r1 = btnArea.removeFromTop(btnH);
    saveButton->setBounds(r1.removeFromLeft(btnW));
    r1.removeFromLeft(4);
    deleteButton->setBounds(r1);

    btnArea.removeFromTop(4);

    auto r2 = btnArea.removeFromTop(btnH);
    exportButton->setBounds(r2.removeFromLeft(btnW));
    r2.removeFromLeft(4);
    folderButton->setBounds(r2);

    // Top header + search
    auto hdr = b.removeFromTop(18);
    headerLabel->setBounds(hdr);
    b.removeFromTop(4);

    auto searchRow = b.removeFromTop(24);
    searchField->setBounds(searchRow);
    b.removeFromTop(6);

    // Remaining area is the viewport
    listViewport.setBounds(b);

    rebuildRows(searchField->getText());
}

//==============================================================================
// Editor
//==============================================================================
EchoForgeEditor::EchoForgeEditor(EchoForgeProcessor& p)
    : juce::AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&laf);

    using namespace EchoParams;
    auto& a = p.apvts;

    // ===== Sections =====
    addAndMakeVisible(secPrimary);
    addAndMakeVisible(secEngine);
    addAndMakeVisible(secTone);
    addAndMakeVisible(secMod);
    addAndMakeVisible(secPingPong);
    addAndMakeVisible(secOutput);

    // ===== Displays =====
    delayScope = std::make_unique<DelayScope>(audioProcessor);
    addAndMakeVisible(*delayScope);

    meter = std::make_unique<EchoMeter>(audioProcessor);
    addAndMakeVisible(*meter);

    // ===== Primary controls =====
    centerKnob = std::make_unique<EchoKnob>("CENTER", CENTER, a, EchoColour::Orange);
    widthKnob = std::make_unique<EchoKnob>("WIDTH", WIDTH, a, EchoColour::Cyan);
    heightKnob = std::make_unique<EchoKnob>("HEIGHT", HEIGHT, a, EchoColour::Purple);
    bufferKnob = std::make_unique<EchoKnob>("BUFFER", BUFFER, a, EchoColour::Yellow);
    feedbackKnob = std::make_unique<EchoKnob>("FEEDBACK", FEEDBACK, a, EchoColour::Green);
    mixKnob = std::make_unique<EchoKnob>("MIX", MIX, a, EchoColour::Steel);

    addAndMakeVisible(*centerKnob);
    addAndMakeVisible(*widthKnob);
    addAndMakeVisible(*heightKnob);
    addAndMakeVisible(*bufferKnob);
    addAndMakeVisible(*feedbackKnob);
    addAndMakeVisible(*mixKnob);

    // ===== Engine selector =====
    setupCombo(engineCombo, { "Simple", "BBD", "Tape", "Diffuse" });
    engineAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        a, ENGINE, engineCombo);
    addAndMakeVisible(engineCombo);

    // BBD controls
    bbdDriveKnob = std::make_unique<EchoKnob>("DRIVE", BBD_DRIVE, a, EchoColour::Steel);
    bbdChargeKnob = std::make_unique<EchoKnob>("CHARGE", BBD_CHARGE, a, EchoColour::Steel);
    bbdLeakKnob = std::make_unique<EchoKnob>("LEAK", BBD_LEAK, a, EchoColour::Steel);
    bbdStutterKnob = std::make_unique<EchoKnob>("STUTTER", BBD_STUTTER, a, EchoColour::Steel);
    addAndMakeVisible(*bbdDriveKnob);
    addAndMakeVisible(*bbdChargeKnob);
    addAndMakeVisible(*bbdLeakKnob);
    addAndMakeVisible(*bbdStutterKnob);

    // Tape controls
    tapeWowKnob = std::make_unique<EchoKnob>("WOW", TAPE_WOW, a, EchoColour::Steel);
    tapeFlutterKnob = std::make_unique<EchoKnob>("FLUTTER", TAPE_FLUTTER, a, EchoColour::Steel);
    tapeSatKnob = std::make_unique<EchoKnob>("SAT", TAPE_SAT, a, EchoColour::Steel);
    tapeHeadKnob = std::make_unique<EchoKnob>("HEAD", TAPE_HEAD, a, EchoColour::Steel);
    addAndMakeVisible(*tapeWowKnob);
    addAndMakeVisible(*tapeFlutterKnob);
    addAndMakeVisible(*tapeSatKnob);
    addAndMakeVisible(*tapeHeadKnob);

    // Diffuse controls
    diffSpreadKnob = std::make_unique<EchoKnob>("SPREAD", DIFF_SPREAD, a, EchoColour::Steel);
    diffTapsKnob = std::make_unique<EchoKnob>("TAPS", DIFF_TAPS, a, EchoColour::Steel);
    diffDampKnob = std::make_unique<EchoKnob>("DAMP", DIFF_DAMP, a, EchoColour::Steel);
    addAndMakeVisible(*diffSpreadKnob);
    addAndMakeVisible(*diffTapsKnob);
    addAndMakeVisible(*diffDampKnob);

    // ===== Time (secondary) =====
    timeTrimKnob = std::make_unique<EchoKnob>("TIME TRIM", TIME_TRIM, a, EchoColour::Orange);

    syncBtn.setLookAndFeel(&laf);
    syncAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        a, SYNC, syncBtn);

    setupCombo(syncDivCombo, syncDivisionNames());
    syncDivAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        a, SYNC_DIV, syncDivCombo);

    tapTempoBtn.setColour(juce::TextButton::buttonColourId, EchoColour::Panel);
    tapTempoBtn.setColour(juce::TextButton::textColourOffId, EchoColour::Orange);
    tapTempoBtn.onClick = [this]() {
        if (auto* param = audioProcessor.apvts.getParameter(EchoParams::TAP_TEMPO))
        {
            // Momentary toggle: send 1, then 0
            param->setValueNotifyingHost(1.0f);
            param->setValueNotifyingHost(0.0f);
        }
        };

    addAndMakeVisible(*timeTrimKnob);
    addAndMakeVisible(syncBtn);
    addAndMakeVisible(syncDivCombo);
    addAndMakeVisible(tapTempoBtn);

    // ===== Tone =====
    heightTrimKnob = std::make_unique<EchoKnob>("HEIGHT TRIM", HEIGHT_TRIM, a, EchoColour::Purple);
    hpTrimKnob = std::make_unique<EchoKnob>("HP", HP_TRIM, a, EchoColour::Purple);
    lpTrimKnob = std::make_unique<EchoKnob>("LP", LP_TRIM, a, EchoColour::Purple);
    addAndMakeVisible(*heightTrimKnob);
    addAndMakeVisible(*hpTrimKnob);
    addAndMakeVisible(*lpTrimKnob);

    // ===== Modulation =====
    modRateKnob = std::make_unique<EchoKnob>("RATE", MOD_RATE, a, EchoColour::Cyan);
    modDepthKnob = std::make_unique<EchoKnob>("DEPTH", MOD_DEPTH, a, EchoColour::Cyan);
    setupCombo(modShapeCombo, { "Sine", "Triangle", "Saw", "S&H" });
    modShapeAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        a, MOD_SHAPE, modShapeCombo);
    addAndMakeVisible(*modRateKnob);
    addAndMakeVisible(*modDepthKnob);
    addAndMakeVisible(modShapeCombo);

    // ===== Ping-Pong =====
    setupCombo(pingPongCombo, { "Off", "L -> R", "R -> L", "Both" });
    pingPongAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        a, PING_PONG, pingPongCombo);
    crossFeedKnob = std::make_unique<EchoKnob>("CROSS", CROSS_FEED, a, EchoColour::Magenta);
    addAndMakeVisible(pingPongCombo);
    addAndMakeVisible(*crossFeedKnob);

    // ===== Output =====
    outputKnob = std::make_unique<EchoKnob>("GAIN", OUTPUT, a, EchoColour::Green);
    outWidthKnob = std::make_unique<EchoKnob>("WIDTH", OUT_WIDTH, a, EchoColour::Green);
    addAndMakeVisible(*outputKnob);
    addAndMakeVisible(*outWidthKnob);

    bypassBtn.setLookAndFeel(&laf);
    bypassAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        a, BYPASS, bypassBtn);
    addAndMakeVisible(bypassBtn);

    // ===== A/B =====
    abButton.onClick = [this]() { audioProcessor.swapAB(); };
    captureBtn.onClick = [this]() { audioProcessor.captureAState(); };
    addAndMakeVisible(abButton);
    addAndMakeVisible(captureBtn);

    // ===== Preset browser =====
    presetBrowser = std::make_unique<PresetBrowser>(audioProcessor);
    presetBrowser->onPresetSelected = [this](int idx) {
        audioProcessor.loadPresetByIndex(idx);
        };
    presetBrowser->onSaveRequested = [this]() {
        auto* window = new juce::AlertWindow("Save Preset",
            "Enter a name for the new preset:",
            juce::MessageBoxIconType::NoIcon);
        window->addTextEditor("name", "", "Name:");

        const int currentIdx = presetBrowser->getSelectedIndex();
        if (currentIdx >= 0)
        {
            auto& manager = audioProcessor.getPresetManager();
            if (currentIdx < manager.getNumPresets())
            {
                const auto& info = manager.getPresetInfo(currentIdx);
                window->getTextEditor("name")->setText(info.name + " (copy)",
                    juce::dontSendNotification);
                window->getTextEditor("name")->selectAll();
            }
        }

        window->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
        window->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

        window->enterModalState(true,
            juce::ModalCallbackFunction::create(
                [this, window](int result)
                {
                    juce::String name = window->getTextEditorContents("name").trim();
                    if (result == 0 || name.isEmpty()) return;
                    auto& manager = audioProcessor.getPresetManager();
                    manager.saveUserPreset(name, "User", "", true);
                    presetBrowser->refresh();
                }), true);
        };
    addAndMakeVisible(*presetBrowser);

    // Hook processor preset changes
    audioProcessor.onPresetChanged = [this](int newIndex) {
        onProcessorPresetChanged(newIndex);
        };

    startTimerHz(10);

    setSize(1280, 820);
    setResizable(false, false);
}

EchoForgeEditor::~EchoForgeEditor()
{
    stopTimer();
    audioProcessor.onPresetChanged = nullptr;

    syncBtn.setLookAndFeel(nullptr);
    bypassBtn.setLookAndFeel(nullptr);

    for (auto* cb : { &engineCombo, &syncDivCombo, &modShapeCombo, &pingPongCombo })
        cb->setLookAndFeel(nullptr);

    setLookAndFeel(nullptr);
}

void EchoForgeEditor::setupCombo(juce::ComboBox& box, const juce::StringArray& items)
{
    box.setLookAndFeel(&laf);
    int id = 1;
    for (auto& item : items)
        box.addItem(item, id++);
    box.setSelectedId(1, juce::dontSendNotification);
}

void EchoForgeEditor::drawHeader(juce::Graphics& g, juce::Rectangle<int> area)
{
    g.setColour(EchoColour::Header);
    g.fillRect(area);

    juce::ColourGradient accent(
        EchoColour::Magenta.withAlpha(0.7f), 0.0f, (float)(area.getBottom() - 1),
        EchoColour::Cyan.withAlpha(0.7f), (float)area.getWidth(),
        (float)(area.getBottom() - 1), false);
    g.setGradientFill(accent);
    g.fillRect(0, area.getBottom() - 2, area.getWidth(), 2);

    g.setFont(EchoLAF::displayFont(20.0f));
    g.setColour(EchoColour::IvoryText);
    g.drawText("ECHOFORGE", area.getX() + 16, area.getY(),
        220, area.getHeight(), juce::Justification::centredLeft);

    g.setFont(EchoLAF::monoFont(9.0f));
    g.setColour(EchoColour::DimText);
    g.drawText("Simple / BBD / Tape / Diffuse  |  AlphaAudio",
        area.getX() + 216, area.getY(),
        400, area.getHeight(), juce::Justification::centredLeft);

    g.setFont(EchoLAF::monoFont(9.0f));
    g.drawText("v1.0.0", area.getRight() - 300, area.getY(),
        54, area.getHeight(), juce::Justification::centredRight);
}

void EchoForgeEditor::paint(juce::Graphics& g)
{
    g.fillAll(EchoColour::BG);
    drawHeader(g, getLocalBounds().removeFromTop(44));

    g.setColour(juce::Colours::white.withAlpha(0.012f));
    for (int y = 44; y < getHeight(); y += 3)
        g.drawHorizontalLine(y, 0.0f, (float)getWidth());
}

void EchoForgeEditor::resized()
{
    const int headerH = 44;
    const int pad = 10;
    const int barH = 24;

    auto full = getLocalBounds();
    auto header = full.removeFromTop(headerH);
    auto body = full.reduced(pad);

    // --- Header ---
    {
        auto top = header.reduced(pad, (headerH - barH) / 2);
        meter->setBounds(top.removeFromRight(160).reduced(0, 1));
        top.removeFromRight(10);

        bypassBtn.setBounds(top.removeFromRight(90).reduced(0, 2));
        top.removeFromRight(8);
        abButton.setBounds(top.removeFromRight(60));
        top.removeFromRight(4);
        captureBtn.setBounds(top.removeFromRight(72));
    }

    // --- Preset browser (right column) ---
    auto browserArea = body.removeFromRight(280);
    presetBrowser->setBounds(browserArea);
    body.removeFromRight(pad);

    // --- Row 1: Delay scope (200px tall) ---
    {
        auto row = body.removeFromTop(180);
        delayScope->setBounds(row);
        body.removeFromTop(pad);
    }

    // --- Row 2: Primary controls (140px) ---
    {
        auto row = body.removeFromTop(140);
        secPrimary.setBounds(row);

        auto inner = row.reduced(10, 0);
        inner.removeFromTop(26);

        const int n = 6;
        const int slotW = inner.getWidth() / n;
        auto place = [&](std::unique_ptr<EchoKnob>& k) {
            k->setBounds(inner.removeFromLeft(slotW).reduced(2));
            };
        place(centerKnob);
        place(widthKnob);
        place(heightKnob);
        place(bufferKnob);
        place(feedbackKnob);
        place(mixKnob);

        body.removeFromTop(pad);
    }

    // --- Row 3: Engine (140px) ---
    {
        auto row = body.removeFromTop(140);
        secEngine.setBounds(row);

        auto inner = row.reduced(10, 0);
        inner.removeFromTop(26);

        // Left column: engine selector
        auto leftCol = inner.removeFromLeft(120);
        engineCombo.setBounds(leftCol.removeFromTop(22).reduced(0, 1));

        inner.removeFromLeft(10);

        // Right area: stacked engine-specific knob rows
        // We show all controls but they visually cluster per engine. The
        // engine section reserves four knob slots per row and we place the
        // active engine's knobs into them, since the sections and layout
        // are shared.
        //
        // Row A: BBD / Tape shared layout (4 knobs)
        // Row B: Tape (four more) / Diffuse (three)
        const int rows = 2;
        const int rowH = inner.getHeight() / rows;
        auto rowA = inner.removeFromTop(rowH);
        auto rowB = inner;

        // Row A: 4 knobs
        const int slotA = rowA.getWidth() / 4;
        bbdDriveKnob->setBounds(rowA.removeFromLeft(slotA).reduced(2));
        bbdChargeKnob->setBounds(rowA.removeFromLeft(slotA).reduced(2));
        bbdLeakKnob->setBounds(rowA.removeFromLeft(slotA).reduced(2));
        bbdStutterKnob->setBounds(rowA.removeFromLeft(slotA).reduced(2));

        // Row B: 4 knobs (Tape) and the Diffuse row shares space
        const int slotB = rowB.getWidth() / 7;
        tapeWowKnob->setBounds(rowB.removeFromLeft(slotB).reduced(2));
        tapeFlutterKnob->setBounds(rowB.removeFromLeft(slotB).reduced(2));
        tapeSatKnob->setBounds(rowB.removeFromLeft(slotB).reduced(2));
        tapeHeadKnob->setBounds(rowB.removeFromLeft(slotB).reduced(2));
        diffSpreadKnob->setBounds(rowB.removeFromLeft(slotB).reduced(2));
        diffTapsKnob->setBounds(rowB.removeFromLeft(slotB).reduced(2));
        diffDampKnob->setBounds(rowB.removeFromLeft(slotB).reduced(2));

        body.removeFromTop(pad);
    }

    // --- Row 4: Time (secondary) | Tone | Mod | Ping-Pong ---
    {
        auto row = body.removeFromTop(120);
        const int qw = (row.getWidth() - pad * 3) / 4;
        auto timeArea = row.removeFromLeft(qw);
        row.removeFromLeft(pad);
        auto toneArea = row.removeFromLeft(qw);
        row.removeFromLeft(pad);
        auto modArea = row.removeFromLeft(qw);
        row.removeFromLeft(pad);
        auto ppArea = row;

        // --- Time ---
        secPrimary.setVisible(true);  // reuse primary's section? no, we want a "time" section
        // Actually we already have secPrimary for the primary row. Let's
        // not double-draw — the primary section is gone by now because we
        // reused secPrimary bounds above. So we need a new section for
        // time. Since we didn't declare one, let's just paint the time
        // panel via a locally created EchoSection.
        static EchoSection timeSection{ "TIME", EchoColour::Orange };
        static bool timeSectionAdded = false;
        if (!timeSectionAdded)
        {
            addAndMakeVisible(timeSection);
            timeSectionAdded = true;
        }
        timeSection.setBounds(timeArea);
        {
            auto inner = timeArea.reduced(8, 0);
            inner.removeFromTop(26);

            // Left column: sync toggle + sync div combo
            auto lc = inner.removeFromLeft(100);
            syncBtn.setBounds(lc.removeFromTop(22).reduced(0, 1));
            lc.removeFromTop(4);
            syncDivCombo.setBounds(lc.removeFromTop(22).reduced(0, 1));

            inner.removeFromLeft(6);

            // Right: time trim + tap
            auto rc = inner;
            tapTempoBtn.setBounds(rc.removeFromRight(60).reduced(0, 2));
            rc.removeFromRight(6);
            timeTrimKnob->setBounds(rc.reduced(2));
        }

        // --- Tone ---
        secTone.setBounds(toneArea);
        {
            auto inner = toneArea.reduced(8, 0);
            inner.removeFromTop(26);
            const int slot = inner.getWidth() / 3;
            heightTrimKnob->setBounds(inner.removeFromLeft(slot).reduced(2));
            hpTrimKnob->setBounds(inner.removeFromLeft(slot).reduced(2));
            lpTrimKnob->setBounds(inner.removeFromLeft(slot).reduced(2));
        }

        // --- Mod ---
        secMod.setBounds(modArea);
        {
            auto inner = modArea.reduced(8, 0);
            inner.removeFromTop(26);

            auto lc = inner.removeFromLeft(100);
            modShapeCombo.setBounds(lc.removeFromTop(22).reduced(0, 1));

            inner.removeFromLeft(6);

            const int slot = inner.getWidth() / 2;
            modRateKnob->setBounds(inner.removeFromLeft(slot).reduced(2));
            modDepthKnob->setBounds(inner.removeFromLeft(slot).reduced(2));
        }

        // --- Ping-Pong ---
        secPingPong.setBounds(ppArea);
        {
            auto inner = ppArea.reduced(8, 0);
            inner.removeFromTop(26);

            auto lc = inner.removeFromLeft(120);
            pingPongCombo.setBounds(lc.removeFromTop(22).reduced(0, 1));

            inner.removeFromLeft(6);

            crossFeedKnob->setBounds(inner.reduced(2));
        }

        body.removeFromTop(pad);
    }

    // --- Row 5: Output ---
    {
        auto row = body.removeFromTop(110);
        secOutput.setBounds(row);

        auto inner = row.reduced(10, 0);
        inner.removeFromTop(26);

        // Left: output gain
        auto oc = inner.removeFromLeft(110);
        outputKnob->setBounds(oc.reduced(2));

        inner.removeFromLeft(10);

        // Middle: output width
        auto wc = inner.removeFromLeft(110);
        outWidthKnob->setBounds(wc.reduced(2));

        inner.removeFromLeft(10);

        // Right: A/B + capture buttons sit at header, but we'll put
        // nothing else here — output is a clean 2-knob section.
        juce::ignoreUnused(inner);
    }
}

void EchoForgeEditor::timerCallback()
{
    // The preset browser polls the manager itself; here we can dim controls
    // that are inactive in the current engine.
    const int engine = (int)audioProcessor.apvts.getRawParameterValue(EchoParams::ENGINE)->load();

    auto setAlphaForEngine = [&](EchoKnob* k, bool active) {
        if (k) k->setAlpha(active ? 1.0f : 0.35f);
        };

    const bool bbdOn = (engine == 1);
    const bool tapeOn = (engine == 2);
    const bool diffOn = (engine == 3);

    setAlphaForEngine(bbdDriveKnob.get(), bbdOn);
    setAlphaForEngine(bbdChargeKnob.get(), bbdOn);
    setAlphaForEngine(bbdLeakKnob.get(), bbdOn);
    setAlphaForEngine(bbdStutterKnob.get(), bbdOn);

    setAlphaForEngine(tapeWowKnob.get(), tapeOn);
    setAlphaForEngine(tapeFlutterKnob.get(), tapeOn);
    setAlphaForEngine(tapeSatKnob.get(), tapeOn);
    setAlphaForEngine(tapeHeadKnob.get(), tapeOn);

    setAlphaForEngine(diffSpreadKnob.get(), diffOn);
    setAlphaForEngine(diffTapsKnob.get(), diffOn);
    setAlphaForEngine(diffDampKnob.get(), diffOn);

    // Cross-feed only matters when Ping-Pong is on
    const int pp = (int)audioProcessor.apvts.getRawParameterValue(EchoParams::PING_PONG)->load();
    setAlphaForEngine(crossFeedKnob.get(), pp != 0);
}

void EchoForgeEditor::onProcessorPresetChanged(int newIndex)
{
    if (presetBrowser)
        presetBrowser->setSelectedIndexSilent(newIndex);
}