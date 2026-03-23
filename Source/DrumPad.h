#pragma once
#include <JuceHeader.h>

class DrumPad
{
public:
    struct Sample
    {
        juce::AudioBuffer<float> buffer;
        double sampleRate = 44100.0;
        juce::File file;
    };

    struct VelocityLayer
    {
        float velocityLow  = 0.0f;
        float velocityHigh = 1.0f;
        juce::OwnedArray<Sample> samples;
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

    void loadSample (const juce::File& file, juce::AudioFormatManager& formatManager);
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
    int outputBus = 0;        // 0 = Main, 1 = Kick, 2 = Snare, etc.
    juce::Colour colour { 0xff00ff88 };

    // ADSR
    float attack  = 0.001f;
    float decay   = 0.1f;
    float sustain = 1.0f;
    float release = 0.05f;

    // Per-pad filter
    float filterCutoff = 20000.0f;
    float filterReso   = 0.707f;

    // Per-pad FX parameters
    bool  fxCompEnabled = false;
    float fxCompThreshold = -12.0f;
    float fxCompRatio = 4.0f;

    bool  fxEqEnabled = false;
    float fxEqLo  = 0.0f;
    float fxEqMid = 0.0f;
    float fxEqHi  = 0.0f;

    bool  fxDistEnabled = false;
    float fxDistDrive = 0.0f;
    float fxDistMix   = 0.0f;

    bool  fxReverbEnabled = false;
    float fxReverbSize  = 0.3f;
    float fxReverbDecay = 0.5f;

    // Compressor GR readout (written by audio, read by GUI)
    std::atomic<float> compGainReduction { 0.0f };

    // State
    bool isPlaying() const { return playing; }
    juce::String getLoadedFileName() const { return loadedFileName; }
    juce::File getLoadedFile() const { return loadedFile; }
    int getNumLayers() const { return layers.size(); }

    const juce::AudioBuffer<float>& getSampleBuffer() const;

private:
    juce::OwnedArray<VelocityLayer> layers;
    Sample* activeSample = nullptr;
    juce::AudioBuffer<float> emptyBuffer;
    juce::AudioBuffer<float> tempBuffer; // for per-pad FX processing

    double sampleRate = 44100.0;
    int blockSize = 512;
    double readPosition = 0.0;
    bool playing = false;
    float currentVelocity = 1.0f;

    // Per-pad filter (stereo)
    juce::dsp::IIR::Filter<float> filterL, filterR;
    float lastCutoff = 20000.0f;

    // Per-pad FX processors
    juce::dsp::Compressor<float> padCompressor;
    juce::dsp::IIR::Filter<float> padEqLoL, padEqLoR, padEqMidL, padEqMidR, padEqHiL, padEqHiR;
    juce::dsp::Reverb padReverb;

    // ADSR state
    enum class EnvelopeStage { Idle, Attack, Decay, Sustain, Release };
    EnvelopeStage envStage = EnvelopeStage::Idle;
    float envLevel = 0.0f;

    juce::String loadedFileName;
    juce::File loadedFile;

    void advanceEnvelope();
    VelocityLayer* findLayerForVelocity (float velocity);
    void applyPadFx (juce::AudioBuffer<float>& buf, int numSamples);

    static float velocityTagToLow (const juce::String& tag);
    static float velocityTagToHigh (const juce::String& tag);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrumPad)
};
