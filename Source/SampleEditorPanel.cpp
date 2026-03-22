#include "SampleEditorPanel.h"
#include "LookAndFeel.h"

SampleEditorPanel::SampleEditorPanel()
{
    titleLabel.setColour (juce::Label::textColourId, InstaDrumsLookAndFeel::textSecondary);
    addAndMakeVisible (titleLabel);

    padNameLabel.setColour (juce::Label::textColourId, InstaDrumsLookAndFeel::accent);
    addAndMakeVisible (padNameLabel);

    waveform.setShowADSR (true);
    addAndMakeVisible (waveform);

    setupKnob (attackSlider,  attackLabel,  "Attack",  0.0, 1.0, 0.001, 0.001);
    setupKnob (decaySlider,   decayLabel,   "Decay",   0.0, 2.0, 0.1, 0.01);
    setupKnob (sustainSlider, sustainLabel, "Sustain", 0.0, 1.0, 1.0, 0.01);
    setupKnob (releaseSlider, releaseLabel, "Release", 0.0, 2.0, 0.05, 0.01);
    setupKnob (pitchSlider,   pitchLabel,   "Pitch",  -24.0, 24.0, 0.0, 0.1);
    setupKnob (panSlider,     panLabel,     "Pan",    -1.0, 1.0, 0.0, 0.01);
    setupKnob (cutoffSlider,  cutoffLabel,  "Cutoff",  20.0, 20000.0, 20000.0, 1.0);
    setupKnob (resoSlider,    resoLabel,    "Reso",    0.1, 10.0, 0.707, 0.01);

    cutoffSlider.setSkewFactorFromMidPoint (1000.0);

    cutoffSlider.getProperties().set (InstaDrumsLookAndFeel::knobTypeProperty, "dark");
    resoSlider.getProperties().set (InstaDrumsLookAndFeel::knobTypeProperty, "dark");
}

void SampleEditorPanel::setupKnob (juce::Slider& s, juce::Label& l, const juce::String& name,
                                    double min, double max, double val, double step)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    s.setRange (min, max, step);
    s.setValue (val, juce::dontSendNotification);
    s.setDoubleClickReturnValue (true, val);
    s.addListener (this);
    addAndMakeVisible (s);

    l.setText (name, juce::dontSendNotification);
    l.setColour (juce::Label::textColourId, InstaDrumsLookAndFeel::textSecondary);
    l.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (l);
}

void SampleEditorPanel::setPad (DrumPad* pad)
{
    currentPad = pad;
    updateFromPad();
}

void SampleEditorPanel::updateFromPad()
{
    if (currentPad == nullptr) return;

    padNameLabel.setText (currentPad->name, juce::dontSendNotification);

    attackSlider.setValue (currentPad->attack, juce::dontSendNotification);
    decaySlider.setValue (currentPad->decay, juce::dontSendNotification);
    sustainSlider.setValue (currentPad->sustain, juce::dontSendNotification);
    releaseSlider.setValue (currentPad->release, juce::dontSendNotification);
    pitchSlider.setValue (currentPad->pitch, juce::dontSendNotification);
    panSlider.setValue (currentPad->pan, juce::dontSendNotification);
    cutoffSlider.setValue (currentPad->filterCutoff, juce::dontSendNotification);
    resoSlider.setValue (currentPad->filterReso, juce::dontSendNotification);

    auto& buf = currentPad->getSampleBuffer();
    waveform.setBuffer (&buf);
    waveform.setColour (currentPad->colour);
    waveform.setADSR (currentPad->attack, currentPad->decay, currentPad->sustain, currentPad->release);

    repaint();
}

void SampleEditorPanel::sliderValueChanged (juce::Slider* slider)
{
    if (currentPad == nullptr) return;

    if (slider == &attackSlider)       currentPad->attack  = (float) slider->getValue();
    else if (slider == &decaySlider)   currentPad->decay   = (float) slider->getValue();
    else if (slider == &sustainSlider) currentPad->sustain = (float) slider->getValue();
    else if (slider == &releaseSlider) currentPad->release = (float) slider->getValue();
    else if (slider == &pitchSlider)   currentPad->pitch        = (float) slider->getValue();
    else if (slider == &panSlider)     currentPad->pan          = (float) slider->getValue();
    else if (slider == &cutoffSlider)  currentPad->filterCutoff = (float) slider->getValue();
    else if (slider == &resoSlider)    currentPad->filterReso   = (float) slider->getValue();

    waveform.setADSR (currentPad->attack, currentPad->decay, currentPad->sustain, currentPad->release);
}

void SampleEditorPanel::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour (InstaDrumsLookAndFeel::bgMedium);
    g.fillRoundedRectangle (bounds, 6.0f);
    g.setColour (InstaDrumsLookAndFeel::bgLight.withAlpha (0.5f));
    g.drawRoundedRectangle (bounds, 6.0f, 1.0f);
}

void SampleEditorPanel::resized()
{
    auto area = getLocalBounds().reduced (8);

    // Scale factor based on component height (reference: 280px)
    float scale = (float) getHeight() / 280.0f;
    float titleSize = std::max (13.0f, 18.0f * scale);
    float labelSize = std::max (10.0f, 13.0f * scale);
    int labelH = (int) (labelSize + 6);
    int headerH = (int) (titleSize + 8);
    int spacing = std::max (4, (int) (8 * scale));

    // Header
    titleLabel.setFont (juce::FontOptions (titleSize, juce::Font::bold));
    padNameLabel.setFont (juce::FontOptions (titleSize, juce::Font::bold));

    auto header = area.removeFromTop (headerH);
    titleLabel.setBounds (header.removeFromLeft ((int) (120 * scale)));
    padNameLabel.setBounds (header);

    area.removeFromTop (spacing);

    // Waveform
    int waveHeight = std::max (50, (int) (area.getHeight() * 0.36f));
    waveform.setBounds (area.removeFromTop (waveHeight));

    area.removeFromTop (spacing);

    // ADSR knobs row
    int knobH = (area.getHeight() - spacing - labelH * 2) / 2;
    knobH = std::max (30, knobH);

    auto adsrRow = area.removeFromTop (knobH + labelH);
    int knobW = adsrRow.getWidth() / 4;
    {
        juce::Slider* s[] = { &attackSlider, &decaySlider, &sustainSlider, &releaseSlider };
        juce::Label*  l[] = { &attackLabel,  &decayLabel,  &sustainLabel,  &releaseLabel };
        for (int i = 0; i < 4; ++i)
        {
            l[i]->setFont (juce::FontOptions (labelSize));
            auto col = adsrRow.removeFromLeft (knobW);
            l[i]->setBounds (col.removeFromBottom (labelH));
            s[i]->setBounds (col);
        }
    }

    area.removeFromTop (spacing);

    // Bottom row: Pitch, Pan, Cutoff, Reso
    auto bottomRow = area;
    knobW = bottomRow.getWidth() / 4;
    {
        juce::Slider* s[] = { &pitchSlider, &panSlider, &cutoffSlider, &resoSlider };
        juce::Label*  l[] = { &pitchLabel,  &panLabel,  &cutoffLabel,  &resoLabel };
        for (int i = 0; i < 4; ++i)
        {
            l[i]->setFont (juce::FontOptions (labelSize));
            auto col = bottomRow.removeFromLeft (knobW);
            l[i]->setBounds (col.removeFromBottom (labelH));
            s[i]->setBounds (col);
        }
    }
}
