#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PadComponent.h"
#include "SampleEditorPanel.h"
#include "FxPanel.h"
#include "MasterPanel.h"
#include "LookAndFeel.h"

class InstaDrumsEditor : public juce::AudioProcessorEditor,
                         private juce::Timer
{
public:
    explicit InstaDrumsEditor (InstaDrumsProcessor&);
    ~InstaDrumsEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    InstaDrumsProcessor& processor;
    InstaDrumsLookAndFeel customLookAndFeel;

    // Pad grid (left side)
    juce::OwnedArray<PadComponent> padComponents;
    static constexpr int padColumns = 4;

    // Right side panels
    SampleEditorPanel sampleEditor;
    FxPanel fxPanel;

    // Bottom
    MasterPanel masterPanel;

    // Top bar buttons
    juce::TextButton loadSampleButton { "LOAD SAMPLE" };
    juce::TextButton saveKitButton    { "SAVE KIT" };
    juce::TextButton loadKitButton    { "LOAD KIT" };
    juce::TextButton loadFolderButton { "LOAD FOLDER" };

    juce::Label titleLabel { {}, "INSTADRUMS" };
    juce::Label versionLabel { {}, "v1.1" };
    juce::Label padsLabel { {}, "DRUM SAMPLER" };

    // State
    int selectedPadIndex = 0;

    void rebuildPadGrid();
    void selectPad (int index);
    void timerCallback() override;

    std::unique_ptr<juce::FileChooser> fileChooser;

    // Resizable
    juce::ComponentBoundsConstrainer constrainer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InstaDrumsEditor)
};
