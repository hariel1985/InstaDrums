#include "MasterPanel.h"
#include "LookAndFeel.h"

MasterPanel::MasterPanel()
{
    masterTitle.setColour (juce::Label::textColourId, InstaDrumsLookAndFeel::textSecondary);
    addAndMakeVisible (masterTitle);

    setupKnob (volumeSlider, volumeLabel, "Volume", 0.0, 2.0, 1.0, 0.01);
    setupKnob (tuneSlider,   tuneLabel,   "Tune",  -12.0, 12.0, 0.0, 0.1);
    setupKnob (panSlider,    panLabel,    "Pan",    -1.0, 1.0, 0.0, 0.01);

    addAndMakeVisible (vuMeter);
}

void MasterPanel::setupKnob (juce::Slider& s, juce::Label& l, const juce::String& name,
                              double min, double max, double val, double step)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    s.setRange (min, max, step);
    s.setValue (val, juce::dontSendNotification);
    s.setDoubleClickReturnValue (true, val);
    addAndMakeVisible (s);

    l.setText (name, juce::dontSendNotification);
    l.setColour (juce::Label::textColourId, InstaDrumsLookAndFeel::textSecondary);
    l.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (l);
}

void MasterPanel::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour (InstaDrumsLookAndFeel::bgMedium.darker (0.2f));
    g.fillRoundedRectangle (bounds, 4.0f);
    g.setColour (InstaDrumsLookAndFeel::bgLight.withAlpha (0.3f));
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
}

void MasterPanel::resized()
{
    auto area = getLocalBounds().reduced (6);
    float scale = (float) getHeight() / 52.0f;
    float titleSize = std::max (12.0f, 16.0f * scale);
    float labelSize = std::max (9.0f, 12.0f * scale);
    int labelH = (int) (labelSize + 4);

    masterTitle.setFont (juce::FontOptions (titleSize, juce::Font::bold));
    masterTitle.setBounds (area.removeFromLeft ((int) (65 * scale)).reduced (0, 2));

    vuMeter.setBounds (area.removeFromRight ((int) (28 * scale)).reduced (0, 2));
    area.removeFromRight (4);

    int knobW = area.getWidth() / 3;
    juce::Slider* sliders[] = { &volumeSlider, &tuneSlider, &panSlider };
    juce::Label*  labels[]  = { &volumeLabel,  &tuneLabel,  &panLabel };
    for (int i = 0; i < 3; ++i)
    {
        labels[i]->setFont (juce::FontOptions (labelSize));
        auto col = area.removeFromLeft (knobW);
        labels[i]->setBounds (col.removeFromBottom (labelH));
        sliders[i]->setBounds (col);
    }
}
