#pragma once
#include <JuceHeader.h>
#include "DrumPad.h"

class PadComponent : public juce::Component,
                     public juce::FileDragAndDropTarget
{
public:
    PadComponent (DrumPad& pad, std::function<void(int, const juce::File&)> loadCallback, int padIndex);

    void paint (juce::Graphics& g) override;
    void resized() override;

    void mouseDown (const juce::MouseEvent& event) override;
    void mouseUp (const juce::MouseEvent& event) override;

    // Drag & Drop
    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;
    void fileDragEnter (const juce::StringArray& files, int x, int y) override;
    void fileDragExit (const juce::StringArray& files) override;

    void setSelected (bool sel) { selected = sel; repaint(); }
    bool isSelected() const { return selected; }

    // Callback when pad is selected (left click)
    std::function<void(int)> onSelected;

private:
    DrumPad& drumPad;
    std::function<void(int, const juce::File&)> onLoadSample;
    int index;
    bool isPressed = false;
    bool isDragOver = false;
    bool selected = false;

    void drawWaveformThumbnail (juce::Graphics& g, juce::Rectangle<float> area);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PadComponent)
};
