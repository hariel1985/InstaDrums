#pragma once
#include <JuceHeader.h>
#include "DrumPad.h"

class FxPanel : public juce::Component
{
public:
    FxPanel();

    void paint (juce::Graphics& g) override;
    void resized() override;

    // Connect to a pad's FX params
    void setPad (DrumPad* pad);
    void syncToPad();   // write GUI values to pad
    void syncFromPad(); // read pad values to GUI

    // Compressor GR meter
    void setCompGainReduction (float grDb) { compGrDb = grDb; repaint (compGrArea); }

    void paintOverChildren (juce::Graphics& g) override;

private:
    // Compressor
    juce::Slider compThreshSlider, compRatioSlider;
    juce::Label  compThreshLabel, compRatioLabel, compTitle;
    juce::ToggleButton compToggle;

    // EQ
    juce::Slider eqLoSlider, eqMidSlider, eqHiSlider;
    juce::Label  eqLoLabel, eqMidLabel, eqHiLabel, eqTitle;
    juce::ToggleButton eqToggle;

    // Distortion
    juce::Slider distDriveSlider, distMixSlider;
    juce::Label  distDriveLabel, distMixLabel, distTitle;
    juce::ToggleButton distToggle;

    // Reverb
    juce::Slider reverbSizeSlider, reverbDecaySlider;
    juce::Label  reverbSizeLabel, reverbDecayLabel, reverbTitle;
    juce::ToggleButton reverbToggle;

    DrumPad* currentPad = nullptr;

    float compGrDb = 0.0f;
    bool repaintCompGr = false;
    juce::Rectangle<int> compGrArea;  // stored from resized for paintOverChildren

    void setupKnob (juce::Slider& s, juce::Label& l, const juce::String& name,
                    double min, double max, double val, double step = 0.01);
    void setupTitle (juce::Label& l, const juce::String& text);
    void setupToggle (juce::ToggleButton& t);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FxPanel)
};
