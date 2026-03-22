#pragma once
#include <JuceHeader.h>

class FxPanel : public juce::Component
{
public:
    FxPanel();

    void paint (juce::Graphics& g) override;
    void resized() override;

    // FX parameter getters (for processor to read)
    float getCompThreshold() const { return (float) compThreshSlider.getValue(); }
    float getCompRatio()     const { return (float) compRatioSlider.getValue(); }
    float getEqLo()          const { return (float) eqLoSlider.getValue(); }
    float getEqMid()         const { return (float) eqMidSlider.getValue(); }
    float getEqHi()          const { return (float) eqHiSlider.getValue(); }
    float getDistDrive()     const { return (float) distDriveSlider.getValue(); }
    float getDistMix()       const { return (float) distMixSlider.getValue(); }
    float getReverbSize()    const { return (float) reverbSizeSlider.getValue(); }
    float getReverbDecay()   const { return (float) reverbDecaySlider.getValue(); }

private:
    // Compressor
    juce::Slider compThreshSlider, compRatioSlider;
    juce::Label  compThreshLabel, compRatioLabel, compTitle;

    // EQ
    juce::Slider eqLoSlider, eqMidSlider, eqHiSlider;
    juce::Label  eqLoLabel, eqMidLabel, eqHiLabel, eqTitle;

    // Distortion
    juce::Slider distDriveSlider, distMixSlider;
    juce::Label  distDriveLabel, distMixLabel, distTitle;

    // Reverb
    juce::Slider reverbSizeSlider, reverbDecaySlider;
    juce::Label  reverbSizeLabel, reverbDecayLabel, reverbTitle;

    void setupKnob (juce::Slider& s, juce::Label& l, const juce::String& name,
                    double min, double max, double val, double step = 0.01);
    void setupTitle (juce::Label& l, const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FxPanel)
};
