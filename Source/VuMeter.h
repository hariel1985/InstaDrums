#pragma once
#include <JuceHeader.h>

class VuMeter : public juce::Component
{
public:
    void setLevel (float left, float right)
    {
        // Update peak hold
        if (left > peakL) peakL = left;
        else peakL *= 0.995f;
        if (right > peakR) peakR = right;
        else peakR *= 0.995f;

        // Smooth level (fast attack, medium release)
        levelL = std::max (left, levelL * 0.85f);
        levelR = std::max (right, levelR * 0.85f);
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced (1);
        float barGap = 2.0f;
        float halfW = (bounds.getWidth() - barGap) / 2.0f;
        auto leftBar  = bounds.removeFromLeft (halfW);
        bounds.removeFromLeft (barGap);
        auto rightBar = bounds;

        drawBar (g, leftBar, levelL, peakL);
        drawBar (g, rightBar, levelR, peakR);
    }

private:
    float levelL = 0.0f, levelR = 0.0f;
    float peakL = 0.0f, peakR = 0.0f;

    void drawBar (juce::Graphics& g, juce::Rectangle<float> bar, float level, float peak)
    {
        // Background
        g.setColour (juce::Colour (0xff111122));
        g.fillRoundedRectangle (bar, 2.0f);

        // Scale level to dB-ish display (boost low levels for visibility)
        float displayLevel = std::pow (juce::jlimit (0.0f, 1.0f, level), 0.5f);
        float h = bar.getHeight() * displayLevel;
        auto filled = bar.withTop (bar.getBottom() - h);

        // Segmented colour
        if (displayLevel < 0.6f)
            g.setColour (juce::Colour (0xff00cc44));
        else if (displayLevel < 0.85f)
            g.setColour (juce::Colour (0xffcccc00));
        else
            g.setColour (juce::Colour (0xffff3333));

        g.fillRoundedRectangle (filled, 2.0f);

        // Peak hold line
        float displayPeak = std::pow (juce::jlimit (0.0f, 1.0f, peak), 0.5f);
        if (displayPeak > 0.01f)
        {
            float peakY = bar.getBottom() - bar.getHeight() * displayPeak;
            g.setColour (juce::Colours::white.withAlpha (0.8f));
            g.fillRect (bar.getX(), peakY, bar.getWidth(), 1.5f);
        }
    }
};
