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

        levelL = left;
        levelR = right;
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

        // Scale markers
        g.setColour (juce::Colour (0x44ffffff));
        g.setFont (juce::FontOptions (9.0f));
        float totalH = getLocalBounds().toFloat().getHeight();
        // dB markers: 0dB = top, -6, -12, -24, -48
        float dbLevels[] = { 1.0f, 0.5f, 0.25f, 0.063f, 0.004f };
        const char* dbLabels[] = { "0", "-6", "-12", "-24", "-48" };
        for (int i = 0; i < 5; ++i)
        {
            float yPos = (1.0f - dbLevels[i]) * totalH;
            g.drawHorizontalLine ((int) yPos, 0.0f, (float) getWidth());
        }
    }

private:
    float levelL = 0.0f, levelR = 0.0f;
    float peakL = 0.0f, peakR = 0.0f;

    void drawBar (juce::Graphics& g, juce::Rectangle<float> bar, float level, float peak)
    {
        // Background
        g.setColour (juce::Colour (0xff111122));
        g.fillRoundedRectangle (bar, 2.0f);

        float clampedLevel = juce::jlimit (0.0f, 1.0f, level);
        float h = bar.getHeight() * clampedLevel;
        auto filled = bar.withTop (bar.getBottom() - h);

        // Segmented colour
        if (clampedLevel < 0.6f)
            g.setColour (juce::Colour (0xff00cc44));
        else if (clampedLevel < 0.85f)
            g.setColour (juce::Colour (0xffcccc00));
        else
            g.setColour (juce::Colour (0xffff3333));

        g.fillRoundedRectangle (filled, 2.0f);

        // Peak hold line
        float clampedPeak = juce::jlimit (0.0f, 1.0f, peak);
        if (clampedPeak > 0.01f)
        {
            float peakY = bar.getBottom() - bar.getHeight() * clampedPeak;
            g.setColour (juce::Colours::white.withAlpha (0.8f));
            g.fillRect (bar.getX(), peakY, bar.getWidth(), 1.5f);
        }
    }
};
