#pragma once
#include <JuceHeader.h>
#include "DrumPad.h"
#include "WaveformDisplay.h"

class SampleEditorPanel : public juce::Component,
                          public juce::Slider::Listener
{
public:
    SampleEditorPanel();

    void setPad (DrumPad* pad);
    DrumPad* getCurrentPad() const { return currentPad; }

    void paint (juce::Graphics& g) override;
    void resized() override;
    void sliderValueChanged (juce::Slider* slider) override;

    void updateFromPad();

private:
    DrumPad* currentPad = nullptr;

    WaveformDisplay waveform;

    // ADSR knobs
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::Label  attackLabel, decayLabel, sustainLabel, releaseLabel;

    // Sample controls
    juce::Slider pitchSlider, panSlider, cutoffSlider, resoSlider;
    juce::Label  pitchLabel, panLabel, cutoffLabel, resoLabel;

    juce::Label titleLabel { {}, "Sample Editor" };
    juce::Label padNameLabel { {}, "" };

    void setupKnob (juce::Slider& s, juce::Label& l, const juce::String& name,
                    double min, double max, double val, double step = 0.01);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SampleEditorPanel)
};
