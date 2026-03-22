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

    auto layoutSection = [&] (juce::Rectangle<int> secArea, juce::Label& title,
                               juce::Slider* sliders[], juce::Label* labels[], int count)
    {
        title.setBounds (secArea.removeFromTop (titleH).reduced (4, 0));
        int kw = secArea.getWidth() / count;
        for (int i = 0; i < count; ++i)
        {
            labels[i]->setFont (juce::FontOptions (labelSize));
            auto col = secArea.removeFromLeft (kw);
            labels[i]->setBounds (col.removeFromBottom (labelH));
            sliders[i]->setBounds (col);
        }
    };

    // Top-left: Compressor
    {
        auto sec = area.removeFromTop (rowH).removeFromLeft (halfW).reduced (4, 2);
        juce::Slider* s[] = { &compThreshSlider, &compRatioSlider };
        juce::Label*  l[] = { &compThreshLabel,  &compRatioLabel };
        layoutSection (sec, compTitle, s, l, 2);
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
        layoutSection (rightTop, eqTitle, s, l, 3);
    }

    // Bottom-left: Distortion
    auto bottomArea = getLocalBounds().reduced (8);
    bottomArea.removeFromTop ((int) (headerH + 4 + rowH));
    {
        auto sec = bottomArea.removeFromLeft (halfW).reduced (4, 2);
        juce::Slider* s[] = { &distDriveSlider, &distMixSlider };
        juce::Label*  l[] = { &distDriveLabel,  &distMixLabel };
        layoutSection (sec, distTitle, s, l, 2);
    }

    // Bottom-right: Reverb
    {
        auto sec = bottomArea.reduced (4, 2);
        juce::Slider* s[] = { &reverbSizeSlider, &reverbDecaySlider };
        juce::Label*  l[] = { &reverbSizeLabel,  &reverbDecayLabel };
        layoutSection (sec, reverbTitle, s, l, 2);
    }
}
