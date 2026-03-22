#pragma once
#include <JuceHeader.h>

class WaveformDisplay : public juce::Component
{
public:
    WaveformDisplay();

    void setBuffer (const juce::AudioBuffer<float>* buffer, double sampleRate = 44100.0);
    void setColour (juce::Colour c) { waveColour = c; repaint(); }
    void setStartEnd (float start, float end) { startPos = start; endPos = end; repaint(); }
    void setADSR (float a, float d, float s, float r) { adsrA = a; adsrD = d; adsrS = s; adsrR = r; repaint(); }
    void setShowADSR (bool show) { showADSR = show; repaint(); }

    void paint (juce::Graphics& g) override;

private:
    const juce::AudioBuffer<float>* audioBuffer = nullptr;
    double bufferSampleRate = 44100.0;
    juce::Colour waveColour { 0xffff8844 };
    float startPos = 0.0f, endPos = 1.0f;
    float adsrA = 0.001f, adsrD = 0.1f, adsrS = 1.0f, adsrR = 0.05f;
    bool showADSR = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformDisplay)
};
