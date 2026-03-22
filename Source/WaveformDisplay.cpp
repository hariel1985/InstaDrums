#include "WaveformDisplay.h"
#include "LookAndFeel.h"

WaveformDisplay::WaveformDisplay() {}

void WaveformDisplay::setBuffer (const juce::AudioBuffer<float>* buffer, double sampleRate)
{
    audioBuffer = buffer;
    bufferSampleRate = sampleRate;
    repaint();
}

void WaveformDisplay::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background with subtle gradient
    {
        juce::ColourGradient bgGrad (InstaDrumsLookAndFeel::bgDark.darker (0.4f), 0, bounds.getY(),
                                      InstaDrumsLookAndFeel::bgDark.darker (0.2f), 0, bounds.getBottom(), false);
        g.setGradientFill (bgGrad);
        g.fillRoundedRectangle (bounds, 4.0f);
    }

    // Border
    g.setColour (InstaDrumsLookAndFeel::bgLight.withAlpha (0.3f));
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);

    // Grid lines (vertical)
    g.setColour (InstaDrumsLookAndFeel::bgLight.withAlpha (0.12f));
    for (int i = 1; i < 8; ++i)
    {
        float xLine = bounds.getX() + bounds.getWidth() * (float) i / 8.0f;
        g.drawVerticalLine ((int) xLine, bounds.getY(), bounds.getBottom());
    }

    // Center line (horizontal)
    g.setColour (InstaDrumsLookAndFeel::bgLight.withAlpha (0.2f));
    float midY = bounds.getCentreY();
    g.drawHorizontalLine ((int) midY, bounds.getX(), bounds.getRight());

    if (audioBuffer == nullptr || audioBuffer->getNumSamples() == 0)
        return;

    const int numSamples = audioBuffer->getNumSamples();
    const int startSample = (int) (startPos * numSamples);
    const int endSample = (int) (endPos * numSamples);
    const int visibleSamples = std::max (1, endSample - startSample);
    const float width = bounds.getWidth();
    const float height = bounds.getHeight();
    // midY already declared above

    // Draw waveform
    juce::Path wavePath;
    const float* data = audioBuffer->getReadPointer (0);

    for (int x = 0; x < (int) width; ++x)
    {
        int sampleIndex = startSample + (int) ((float) x / width * visibleSamples);
        sampleIndex = juce::jlimit (0, numSamples - 1, sampleIndex);

        // Find min/max in a small range for better visualization
        int blockSize = std::max (1, visibleSamples / (int) width);
        float minVal = 1.0f, maxVal = -1.0f;
        for (int j = 0; j < blockSize && (sampleIndex + j) < numSamples; ++j)
        {
            float v = data[sampleIndex + j];
            minVal = std::min (minVal, v);
            maxVal = std::max (maxVal, v);
        }

        float topY = midY - maxVal * (height * 0.45f);
        float botY = midY - minVal * (height * 0.45f);

        if (x == 0)
            wavePath.startNewSubPath ((float) x + bounds.getX(), topY);

        wavePath.lineTo ((float) x + bounds.getX(), topY);

        if (x == (int) width - 1)
        {
            // Close the path by going back along bottom
            for (int bx = (int) width - 1; bx >= 0; --bx)
            {
                int si = startSample + (int) ((float) bx / width * visibleSamples);
                si = juce::jlimit (0, numSamples - 1, si);
                float mn = 1.0f;
                for (int j = 0; j < blockSize && (si + j) < numSamples; ++j)
                    mn = std::min (mn, data[si + j]);
                float by = midY - mn * (height * 0.45f);
                wavePath.lineTo ((float) bx + bounds.getX(), by);
            }
            wavePath.closeSubPath();
        }
    }

    // Fill waveform
    g.setColour (waveColour.withAlpha (0.5f));
    g.fillPath (wavePath);
    g.setColour (waveColour.withAlpha (0.9f));
    g.strokePath (wavePath, juce::PathStrokeType (1.0f));

    // Draw ADSR overlay
    if (showADSR)
    {
        float totalSeconds = (float) numSamples / (float) bufferSampleRate;
        float ax = adsrA / totalSeconds;
        float dx = adsrD / totalSeconds;
        float sx = 0.4f; // sustain portion
        float rx = adsrR / totalSeconds;
        float total = ax + dx + sx + rx;

        // Normalize to width
        juce::Path adsrPath;
        float x0 = bounds.getX();
        float w = bounds.getWidth();

        adsrPath.startNewSubPath (x0, bounds.getBottom());
        adsrPath.lineTo (x0 + (ax / total) * w, bounds.getY() + 4); // attack peak
        adsrPath.lineTo (x0 + ((ax + dx) / total) * w, midY - (adsrS - 0.5f) * height * 0.8f); // decay to sustain
        adsrPath.lineTo (x0 + ((ax + dx + sx) / total) * w, midY - (adsrS - 0.5f) * height * 0.8f); // sustain hold
        adsrPath.lineTo (x0 + w, bounds.getBottom()); // release to 0

        g.setColour (InstaDrumsLookAndFeel::accent.withAlpha (0.7f));
        g.strokePath (adsrPath, juce::PathStrokeType (2.0f));
    }
}
