#pragma once
#include <JuceHeader.h>
#include "DrumPad.h"

class InstaDrumsProcessor : public juce::AudioProcessor
{
public:
    static constexpr int defaultNumPads = 12;
    static constexpr int maxPads = 64;

    InstaDrumsProcessor();
    ~InstaDrumsProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Pad management
    int getNumPads() const { return numActivePads; }
    DrumPad& getPad (int index) { return pads[index]; }
    void addPads (int count = 4);
    void loadSample (int padIndex, const juce::File& file);

    juce::AudioFormatManager& getFormatManager() { return formatManager; }

    DrumPad* findPadForNote (int midiNote);

    // Kit management
    void loadKitFromFolder (const juce::File& folder);
    void saveKitPreset (const juce::File& file);
    void loadKitPreset (const juce::File& file);

    // Master FX parameters (read by editor from FxPanel/MasterPanel)
    std::atomic<float> masterVolume { 1.0f };
    std::atomic<float> masterPan    { 0.0f };
    std::atomic<float> masterTune   { 0.0f };

    // FX params
    std::atomic<float> compThreshold { -12.0f };
    std::atomic<float> compRatio     { 4.0f };
    std::atomic<float> eqLo  { 0.0f };
    std::atomic<float> eqMid { 0.0f };
    std::atomic<float> eqHi  { 0.0f };
    std::atomic<float> distDrive { 0.0f };
    std::atomic<float> distMix   { 0.0f };
    std::atomic<float> reverbSize  { 0.3f };
    std::atomic<float> reverbDecay { 0.5f };

    // VU meter levels (written by audio thread, read by GUI)
    std::atomic<float> vuLevelL { 0.0f };
    std::atomic<float> vuLevelR { 0.0f };

private:
    std::array<DrumPad, maxPads> pads;
    int numActivePads = defaultNumPads;
    juce::AudioFormatManager formatManager;

    // Master FX chain
    juce::dsp::Reverb reverb;
    juce::dsp::Compressor<float> compressor;
    juce::dsp::IIR::Filter<float> eqLoFilterL, eqLoFilterR;
    juce::dsp::IIR::Filter<float> eqMidFilterL, eqMidFilterR;
    juce::dsp::IIR::Filter<float> eqHiFilterL, eqHiFilterR;

    double currentSampleRate = 44100.0;

    void initializeDefaults();
    void applyMasterFx (juce::AudioBuffer<float>& buffer);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InstaDrumsProcessor)
};
