#pragma once
#include <JuceHeader.h>

class InstaDrumsLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // Colour palette
    static inline const juce::Colour bgDark      { 0xff1a1a2e };
    static inline const juce::Colour bgMedium    { 0xff16213e };
    static inline const juce::Colour bgLight     { 0xff0f3460 };
    static inline const juce::Colour textPrimary  { 0xffe0e0e0 };
    static inline const juce::Colour textSecondary { 0xff888899 };
    static inline const juce::Colour accent       { 0xff00ff88 };

    InstaDrumsLookAndFeel();

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider& slider) override;
};
