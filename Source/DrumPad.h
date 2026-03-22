#pragma once
#include <JuceHeader.h>

class DrumPad
{
public:
    // A single sample with its audio data and source sample rate
    struct Sample
    {
        juce::AudioBuffer<float> buffer;
        double sampleRate = 44100.0;
        juce::File file;
    };

    // A velocity layer: velocity range + multiple round-robin samples
    struct VelocityLayer
    {
        float velocityLow  = 0.0f;   // 0.0 - 1.0
        float velocityHigh = 1.0f;
        juce::OwnedArray<Sample> samples;  // round-robin variations
        int nextRoundRobin = 0;

        Sample* getNextSample()
        {
            if (samples.isEmpty()) return nullptr;
            auto* s = samples[nextRoundRobin % samples.size()];
            nextRoundRobin = (nextRoundRobin + 1) % samples.size();
            return s;
        }
    };

    DrumPad();
    ~DrumPad();

    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();

    // Single sample loading (backwards compatible)
    void loadSample (const juce::File& file, juce::AudioFormatManager& formatManager);

    // Velocity layer loading from a folder
    void loadLayersFromFolder (const juce::File& folder, juce::AudioFormatManager& formatManager);

    bool hasSample() const;

    void trigger (float velocity = 1.0f);
    void stop();

    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples);

    // Pad properties
    juce::String name;
    int midiNote = 36;
    float volume = 1.0f;
    float pan = 0.0f;
    float pitch = 0.0f;
    bool oneShot = true;
    int chokeGroup = -1;
    juce::Colour colour { 0xff00ff88 };

    // ADSR
    float attack  = 0.001f;
    float decay   = 0.1f;
    float sustain = 1.0f;
    float release = 0.05f;

    // Per-pad filter
    float filterCutoff = 20000.0f;  // Hz
    float filterReso   = 0.707f;    // Q

    // State
    bool isPlaying() const { return playing; }
    juce::String getLoadedFileName() const { return loadedFileName; }
    juce::File getLoadedFile() const { return loadedFile; }
    int getNumLayers() const { return layers.size(); }

    const juce::AudioBuffer<float>& getSampleBuffer() const;

private:
    // Velocity layers (sorted by velocity range)
    juce::OwnedArray<VelocityLayer> layers;

    // Currently playing sample reference
    Sample* activeSample = nullptr;

    // Fallback empty buffer for getSampleBuffer when nothing loaded
    juce::AudioBuffer<float> emptyBuffer;

    double sampleRate = 44100.0;
    double readPosition = 0.0;
    bool playing = false;
    float currentVelocity = 1.0f;

    // Per-pad low-pass filter (stereo)
    juce::dsp::IIR::Filter<float> filterL, filterR;
    float lastCutoff = 20000.0f;

    // ADSR state
    enum class EnvelopeStage { Idle, Attack, Decay, Sustain, Release };
    EnvelopeStage envStage = EnvelopeStage::Idle;
    float envLevel = 0.0f;

    juce::String loadedFileName;
    juce::File loadedFile;

    void advanceEnvelope();
    VelocityLayer* findLayerForVelocity (float velocity);

    // Parse velocity tag from filename (e.g. "snare_OH_FF_1" -> FF)
    static float velocityTagToLow (const juce::String& tag);
    static float velocityTagToHigh (const juce::String& tag);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrumPad)
};
