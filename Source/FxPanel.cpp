#include "FxPanel.h"
#include "LookAndFeel.h"

FxPanel::FxPanel()
{
    setupTitle (compTitle, "COMPRESSOR");
    setupTitle (eqTitle,   "EQ");
    setupTitle (distTitle, "DISTORTION");
    setupTitle (reverbTitle, "REVERB");

    setupKnob (compThreshSlider, compThreshLabel, "Threshold", -60.0, 0.0, -12.0, 0.5);
    setupKnob (compRatioSlider,  compRatioLabel,  "Ratio",     1.0, 20.0, 4.0, 0.1);
    setupKnob (eqLoSlider,  eqLoLabel,  "Lo",  -12.0, 12.0, 0.0, 0.1);
    setupKnob (eqMidSlider, eqMidLabel, "Mid", -12.0, 12.0, 0.0, 0.1);
    setupKnob (eqHiSlider,  eqHiLabel,  "Hi",  -12.0, 12.0, 0.0, 0.1);
    setupKnob (distDriveSlider, distDriveLabel, "Drive", 0.0, 1.0, 0.0, 0.01);
    setupKnob (distMixSlider,   distMixLabel,   "Mix",   0.0, 1.0, 0.0, 0.01);
    setupKnob (reverbSizeSlider,  reverbSizeLabel,  "Size",  0.0, 1.0, 0.3, 0.01);
    setupKnob (reverbDecaySlider, reverbDecayLabel, "Decay", 0.0, 1.0, 0.5, 0.01);

    setupToggle (compToggle);
    setupToggle (eqToggle);
    setupToggle (distToggle);
    setupToggle (reverbToggle);
}

void FxPanel::setupKnob (juce::Slider& s, juce::Label& l, const juce::String& name,
                          double min, double max, double val, double step)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    s.setRange (min, max, step);
    s.setValue (val, juce::dontSendNotification);
    s.setDoubleClickReturnValue (true, val);
    s.getProperties().set (InstaDrumsLookAndFeel::knobTypeProperty, "dark");
    addAndMakeVisible (s);

    l.setText (name, juce::dontSendNotification);
    l.setColour (juce::Label::textColourId, InstaDrumsLookAndFeel::textSecondary);
    l.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (l);
}

void FxPanel::setupTitle (juce::Label& l, const juce::String& text)
{
    l.setText (text, juce::dontSendNotification);
    l.setColour (juce::Label::textColourId, InstaDrumsLookAndFeel::accent);
    l.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (l);
}

void FxPanel::setupToggle (juce::ToggleButton& t)
{
    t.setToggleState (false, juce::dontSendNotification);
    t.setButtonText ("");
    addAndMakeVisible (t);
}

void FxPanel::setPad (DrumPad* pad)
{
    currentPad = pad;
    syncFromPad();
}

void FxPanel::syncFromPad()
{
    if (currentPad == nullptr) return;

    compThreshSlider.setValue (currentPad->fxCompThreshold, juce::dontSendNotification);
    compRatioSlider.setValue (currentPad->fxCompRatio, juce::dontSendNotification);
    compToggle.setToggleState (currentPad->fxCompEnabled, juce::dontSendNotification);

    eqLoSlider.setValue (currentPad->fxEqLo, juce::dontSendNotification);
    eqMidSlider.setValue (currentPad->fxEqMid, juce::dontSendNotification);
    eqHiSlider.setValue (currentPad->fxEqHi, juce::dontSendNotification);
    eqToggle.setToggleState (currentPad->fxEqEnabled, juce::dontSendNotification);

    distDriveSlider.setValue (currentPad->fxDistDrive, juce::dontSendNotification);
    distMixSlider.setValue (currentPad->fxDistMix, juce::dontSendNotification);
    distToggle.setToggleState (currentPad->fxDistEnabled, juce::dontSendNotification);

    reverbSizeSlider.setValue (currentPad->fxReverbSize, juce::dontSendNotification);
    reverbDecaySlider.setValue (currentPad->fxReverbDecay, juce::dontSendNotification);
    reverbToggle.setToggleState (currentPad->fxReverbEnabled, juce::dontSendNotification);
}

void FxPanel::syncToPad()
{
    if (currentPad == nullptr) return;

    currentPad->fxCompThreshold = (float) compThreshSlider.getValue();
    currentPad->fxCompRatio     = (float) compRatioSlider.getValue();
    currentPad->fxCompEnabled   = compToggle.getToggleState();

    currentPad->fxEqLo      = (float) eqLoSlider.getValue();
    currentPad->fxEqMid     = (float) eqMidSlider.getValue();
    currentPad->fxEqHi      = (float) eqHiSlider.getValue();
    currentPad->fxEqEnabled = eqToggle.getToggleState();

    currentPad->fxDistDrive   = (float) distDriveSlider.getValue();
    currentPad->fxDistMix     = (float) distMixSlider.getValue();
    currentPad->fxDistEnabled = distToggle.getToggleState();

    currentPad->fxReverbSize    = (float) reverbSizeSlider.getValue();
    currentPad->fxReverbDecay   = (float) reverbDecaySlider.getValue();
    currentPad->fxReverbEnabled = reverbToggle.getToggleState();
}

void FxPanel::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float scale = (float) getHeight() / 220.0f;

    juce::ColourGradient bgGrad (InstaDrumsLookAndFeel::bgMedium, 0, bounds.getY(),
                                  InstaDrumsLookAndFeel::bgMedium.darker (0.2f), 0, bounds.getBottom(), false);
    g.setGradientFill (bgGrad);
    g.fillRoundedRectangle (bounds, 6.0f);
    g.setColour (InstaDrumsLookAndFeel::bgLight.withAlpha (0.6f));
    g.drawRoundedRectangle (bounds, 6.0f, 1.0f);

    // "FX" header bar
    int headerH = (int) (22 * scale);
    auto headerBar = bounds.reduced (1).removeFromTop ((float) headerH);
    g.setColour (InstaDrumsLookAndFeel::bgDark.withAlpha (0.5f));
    g.fillRoundedRectangle (headerBar.getX(), headerBar.getY(), headerBar.getWidth(), headerBar.getHeight(), 5.0f);
    g.setColour (InstaDrumsLookAndFeel::textSecondary);
    g.setFont (juce::FontOptions (std::max (13.0f, 16.0f * scale), juce::Font::bold));
    g.drawText ("FX", headerBar.reduced (8, 0), juce::Justification::centredLeft);

    // Bordered boxes for each FX section
    auto area = bounds.reduced (6);
    area.removeFromTop ((float) headerH + 4);
    float halfW = area.getWidth() / 2;
    float rowH = area.getHeight() / 2;

    auto drawFxBox = [&] (juce::Rectangle<float> r)
    {
        g.setColour (InstaDrumsLookAndFeel::bgDark.withAlpha (0.4f));
        g.fillRoundedRectangle (r, 4.0f);
        g.setColour (InstaDrumsLookAndFeel::bgLight.withAlpha (0.35f));
        g.drawRoundedRectangle (r, 4.0f, 1.0f);
    };

    float gap = 3.0f;
    drawFxBox ({ area.getX(), area.getY(), halfW - gap, rowH - gap });
    drawFxBox ({ area.getX() + halfW + gap, area.getY(), halfW - gap, rowH - gap });
    drawFxBox ({ area.getX(), area.getY() + rowH + gap, halfW - gap, rowH - gap });
    drawFxBox ({ area.getX() + halfW + gap, area.getY() + rowH + gap, halfW - gap, rowH - gap });
}

void FxPanel::resized()
{
    auto area = getLocalBounds().reduced (8);
    float scale = (float) getHeight() / 220.0f;

    float titleSize = std::max (10.0f, 13.0f * scale);
    float labelSize = std::max (9.0f, 12.0f * scale);
    int labelH = (int) (labelSize + 6);
    int headerH = (int) (22 * scale);
    int titleH = (int) (titleSize + 4);

    area.removeFromTop (headerH + 4);

    int halfW = area.getWidth() / 2;
    int rowH = area.getHeight() / 2;

    // Update title fonts
    compTitle.setFont (juce::FontOptions (titleSize, juce::Font::bold));
    eqTitle.setFont (juce::FontOptions (titleSize, juce::Font::bold));
    distTitle.setFont (juce::FontOptions (titleSize, juce::Font::bold));
    reverbTitle.setFont (juce::FontOptions (titleSize, juce::Font::bold));

    int toggleW = std::max (28, (int) (36 * scale));
    int toggleH = std::max (16, (int) (20 * scale));
    int sectionTitleH = std::max (titleH, toggleH + 4);

    auto layoutSection = [&] (juce::Rectangle<int> secArea, juce::Label& title,
                               juce::ToggleButton& toggle,
                               juce::Slider* sliders[], juce::Label* labels[], int count)
    {
        auto titleRow = secArea.removeFromTop (sectionTitleH).reduced (2, 0);
        auto toggleArea = titleRow.removeFromLeft (toggleW + 4);
        toggle.setBounds (toggleArea.withSizeKeepingCentre (toggleW, toggleH));
        title.setBounds (titleRow);
        int kw = secArea.getWidth() / count;
        for (int i = 0; i < count; ++i)
        {
            labels[i]->setFont (juce::FontOptions (labelSize));
            auto col = secArea.removeFromLeft (kw);
            labels[i]->setBounds (col.removeFromBottom (labelH));
            sliders[i]->setBounds (col);
        }
    };

    // Top-left: Compressor (with GR meter area on the right)
    {
        auto compFullArea = area.removeFromTop (rowH).removeFromLeft (halfW).reduced (4, 2);
        int grMeterW = std::max (10, (int) (14 * scale));
        compGrArea = compFullArea.removeFromRight (grMeterW);
        auto sec = compFullArea;
        juce::Slider* s[] = { &compThreshSlider, &compRatioSlider };
        juce::Label*  l[] = { &compThreshLabel,  &compRatioLabel };
        layoutSection (sec, compTitle, compToggle, s, l, 2);
    }

    // Top-right: EQ (need to recalculate since we consumed area)
    // Reset area for right side
    auto rightTop = getLocalBounds().reduced (8);
    rightTop.removeFromTop ((int) (headerH + 4));
    rightTop.removeFromLeft (halfW);
    rightTop = rightTop.removeFromTop (rowH).reduced (4, 2);
    {
        juce::Slider* s[] = { &eqLoSlider, &eqMidSlider, &eqHiSlider };
        juce::Label*  l[] = { &eqLoLabel,  &eqMidLabel,  &eqHiLabel };
        layoutSection (rightTop, eqTitle, eqToggle, s, l, 3);
    }

    // Bottom-left: Distortion
    auto bottomArea = getLocalBounds().reduced (8);
    bottomArea.removeFromTop ((int) (headerH + 4 + rowH));
    {
        auto sec = bottomArea.removeFromLeft (halfW).reduced (4, 2);
        juce::Slider* s[] = { &distDriveSlider, &distMixSlider };
        juce::Label*  l[] = { &distDriveLabel,  &distMixLabel };
        layoutSection (sec, distTitle, distToggle, s, l, 2);
    }

    // Bottom-right: Reverb
    {
        auto sec = bottomArea.reduced (4, 2);
        juce::Slider* s[] = { &reverbSizeSlider, &reverbDecaySlider };
        juce::Label*  l[] = { &reverbSizeLabel,  &reverbDecayLabel };
        layoutSection (sec, reverbTitle, reverbToggle, s, l, 2);
    }
}

void FxPanel::paintOverChildren (juce::Graphics& g)
{
    // Draw compressor GR meter
    if (compGrArea.isEmpty()) return;

    auto bar = compGrArea.toFloat().reduced (2, 4);

    // Background
    g.setColour (juce::Colour (0xff111122));
    g.fillRoundedRectangle (bar, 2.0f);

    // GR bar (grows downward from top, since GR is negative)
    float grNorm = juce::jlimit (0.0f, 1.0f, std::abs (compGrDb) / 30.0f); // 30dB range
    float barH = bar.getHeight() * grNorm;

    if (barH > 0.5f)
    {
        auto filled = bar.removeFromTop (barH);

        // Colour: green for light GR, orange for medium, red for heavy
        juce::Colour grColour;
        if (grNorm < 0.3f)
            grColour = juce::Colour (0xff00cc44);
        else if (grNorm < 0.6f)
            grColour = juce::Colour (0xffccaa00);
        else
            grColour = juce::Colour (0xffff4422);

        g.setColour (grColour);
        g.fillRoundedRectangle (filled, 2.0f);

        // Glow
        g.setColour (grColour.withAlpha (0.2f));
        g.fillRoundedRectangle (filled.expanded (2, 0), 3.0f);
    }

    // "GR" label
    g.setColour (InstaDrumsLookAndFeel::textSecondary.withAlpha (0.6f));
    g.setFont (juce::FontOptions (8.0f));
    g.drawText ("GR", compGrArea.toFloat(), juce::Justification::centredBottom);
}
