#pragma once
#include <JuceHeader.h>
#include "VuMeter.h"

class MasterPanel : public juce::Component
{
public:
    MasterPanel();

    void paint (juce::Graphics& g) override;
    void resized() override;

    float getMasterVolume() const { return (float) volumeSlider.getValue(); }
    float getMasterTune()   const { return (float) tuneSlider.getValue(); }
    float getMasterPan()    const { return (float) panSlider.getValue(); }

    VuMeter& getVuMeter() { return vuMeter; }

private:
    juce::Slider volumeSlider, tuneSlider, panSlider;
    juce::Label  volumeLabel, tuneLabel, panLabel;
    juce::Label  masterTitle { {}, "MASTER" };
    VuMeter vuMeter;

    void setupKnob (juce::Slider& s, juce::Label& l, const juce::String& name,
                    double min, double max, double val, double step = 0.01);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MasterPanel)
};
