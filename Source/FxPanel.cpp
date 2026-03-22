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
    addAndMakeVisible (s);

    l.setText (name, juce::dontSendNotification);
    l.setFont (juce::FontOptions (9.0f));
    l.setColour (juce::Label::textColourId, InstaDrumsLookAndFeel::textSecondary);
    l.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (l);
}

void FxPanel::setupTitle (juce::Label& l, const juce::String& text)
{
    l.setText (text, juce::dontSendNotification);
    l.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    l.setColour (juce::Label::textColourId, InstaDrumsLookAndFeel::accent);
    l.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (l);
}

void FxPanel::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour (InstaDrumsLookAndFeel::bgMedium);
    g.fillRoundedRectangle (bounds, 6.0f);
    g.setColour (InstaDrumsLookAndFeel::bgLight.withAlpha (0.5f));
    g.drawRoundedRectangle (bounds, 6.0f, 1.0f);

    // "FX" header
    g.setColour (InstaDrumsLookAndFeel::textSecondary);
    g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
    g.drawText ("FX", bounds.reduced (6, 4).removeFromTop (18), juce::Justification::centredLeft);
}

void FxPanel::resized()
{
    auto area = getLocalBounds().reduced (6);
    area.removeFromTop (20); // FX header

    int halfW = area.getWidth() / 2;
    int rowH = area.getHeight() / 2;

    // Top row: Compressor | EQ
    auto topRow = area.removeFromTop (rowH);
    {
        auto compArea = topRow.removeFromLeft (halfW).reduced (2);
        compTitle.setBounds (compArea.removeFromTop (14));
        int kw = compArea.getWidth() / 2;
        auto c1 = compArea.removeFromLeft (kw);
        compThreshLabel.setBounds (c1.removeFromBottom (12));
        compThreshSlider.setBounds (c1);
        compRatioLabel.setBounds (compArea.removeFromBottom (12));
        compRatioSlider.setBounds (compArea);
    }
    {
        auto eqArea = topRow.reduced (2);
        eqTitle.setBounds (eqArea.removeFromTop (14));
        int kw = eqArea.getWidth() / 3;
        auto c1 = eqArea.removeFromLeft (kw);
        eqLoLabel.setBounds (c1.removeFromBottom (12));
        eqLoSlider.setBounds (c1);
        auto c2 = eqArea.removeFromLeft (kw);
        eqMidLabel.setBounds (c2.removeFromBottom (12));
        eqMidSlider.setBounds (c2);
        eqHiLabel.setBounds (eqArea.removeFromBottom (12));
        eqHiSlider.setBounds (eqArea);
    }

    // Bottom row: Distortion | Reverb
    {
        auto distArea = area.removeFromLeft (halfW).reduced (2);
        distTitle.setBounds (distArea.removeFromTop (14));
        int kw = distArea.getWidth() / 2;
        auto c1 = distArea.removeFromLeft (kw);
        distDriveLabel.setBounds (c1.removeFromBottom (12));
        distDriveSlider.setBounds (c1);
        distMixLabel.setBounds (distArea.removeFromBottom (12));
        distMixSlider.setBounds (distArea);
    }
    {
        auto revArea = area.reduced (2);
        reverbTitle.setBounds (revArea.removeFromTop (14));
        int kw = revArea.getWidth() / 2;
        auto c1 = revArea.removeFromLeft (kw);
        reverbSizeLabel.setBounds (c1.removeFromBottom (12));
        reverbSizeSlider.setBounds (c1);
        reverbDecayLabel.setBounds (revArea.removeFromBottom (12));
        reverbDecaySlider.setBounds (revArea);
    }
}
