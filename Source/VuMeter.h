#pragma once
#include <JuceHeader.h>

class VuMeter : public juce::Component
{
public:
    void setLevel (float left, float right)
    {
        levelL = left;
        levelR = right;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced (1);
        float halfW = bounds.getWidth() / 2.0f - 1;
        auto leftBar  = bounds.removeFromLeft (halfW);
        bounds.removeFromLeft (2);
        auto rightBar = bounds;

        drawBar (g, leftBar, levelL);
        drawBar (g, rightBar, levelR);
    }

private:
    float levelL = 0.0f, levelR = 0.0f;

    void drawBar (juce::Graphics& g, juce::Rectangle<float> bar, float level)
    {
        g.setColour (juce::Colour (0xff222233));
        g.fillRoundedRectangle (bar, 2.0f);

        float h = bar.getHeight() * juce::jlimit (0.0f, 1.0f, level);
        auto filled = bar.removeFromBottom (h);

        // Green -> Yellow -> Red gradient
        if (level < 0.6f)
            g.setColour (juce::Colour (0xff00cc44));
        else if (level < 0.85f)
            g.setColour (juce::Colour (0xffcccc00));
        else
            g.setColour (juce::Colour (0xffff3333));

        g.fillRoundedRectangle (filled, 2.0f);
    }
};
