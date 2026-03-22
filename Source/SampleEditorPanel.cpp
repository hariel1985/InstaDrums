#include "SampleEditorPanel.h"
#include "LookAndFeel.h"

SampleEditorPanel::SampleEditorPanel()
{
    titleLabel.setFont (juce::FontOptions (14.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, InstaDrumsLookAndFeel::textSecondary);
    addAndMakeVisible (titleLabel);

    padNameLabel.setFont (juce::FontOptions (13.0f, juce::Font::bold));
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
}

void SampleEditorPanel::setupKnob (juce::Slider& s, juce::Label& l, const juce::String& name,
                                    double min, double max, double val, double step)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    s.setRange (min, max, step);
    s.setValue (val, juce::dontSendNotification);
    s.addListener (this);
    addAndMakeVisible (s);

    l.setText (name, juce::dontSendNotification);
    l.setFont (juce::FontOptions (9.0f));
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

    // Update ADSR overlay
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
    auto area = getLocalBounds().reduced (6);

    // Header
    auto header = area.removeFromTop (20);
    titleLabel.setBounds (header.removeFromLeft (100));
    padNameLabel.setBounds (header);

    area.removeFromTop (2);

    // Waveform (top portion ~40%)
    int waveHeight = std::max (60, (int) (area.getHeight() * 0.38f));
    waveform.setBounds (area.removeFromTop (waveHeight));

    area.removeFromTop (4);

    // ADSR knobs row
    int knobH = std::max (40, (int) (area.getHeight() * 0.45f));
    auto adsrRow = area.removeFromTop (knobH);
    int knobW = adsrRow.getWidth() / 4;
    {
        juce::Slider* s[] = { &attackSlider, &decaySlider, &sustainSlider, &releaseSlider };
        juce::Label*  l[] = { &attackLabel,  &decayLabel,  &sustainLabel,  &releaseLabel };
        for (int i = 0; i < 4; ++i)
        {
            auto col = adsrRow.removeFromLeft (knobW);
            l[i]->setBounds (col.removeFromBottom (14));
            s[i]->setBounds (col);
        }
    }

    area.removeFromTop (2);

    // Bottom row: Pitch, Pan, Cutoff, Reso
    auto bottomRow = area;
    knobW = bottomRow.getWidth() / 4;
    {
        juce::Slider* s[] = { &pitchSlider, &panSlider, &cutoffSlider, &resoSlider };
        juce::Label*  l[] = { &pitchLabel,  &panLabel,  &cutoffLabel,  &resoLabel };
        for (int i = 0; i < 4; ++i)
        {
            auto col = bottomRow.removeFromLeft (knobW);
            l[i]->setBounds (col.removeFromBottom (14));
            s[i]->setBounds (col);
        }
    }
}
