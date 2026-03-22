#include "PadComponent.h"
#include "LookAndFeel.h"

PadComponent::PadComponent (DrumPad& pad, std::function<void(int, const juce::File&)> loadCallback, int padIndex)
    : drumPad (pad), onLoadSample (std::move (loadCallback)), index (padIndex)
{
}

void PadComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (2.0f);
    float cornerSize = 6.0f;

    // Background
    float alpha = isPressed ? 0.85f : (isDragOver ? 0.65f : (selected ? 0.55f : 0.3f));
    g.setColour (drumPad.colour.withAlpha (alpha));
    g.fillRoundedRectangle (bounds, cornerSize);

    // Border — selected = bright accent, normal = pad colour
    if (selected)
    {
        g.setColour (juce::Colour (0xff00aaff)); // blue selection
        g.drawRoundedRectangle (bounds, cornerSize, 2.5f);
    }
    else
    {
        g.setColour (drumPad.colour.withAlpha (0.6f));
        g.drawRoundedRectangle (bounds, cornerSize, 1.0f);
    }

    // Pad number (top-left, small)
    g.setColour (InstaDrumsLookAndFeel::textSecondary);
    g.setFont (juce::Font (juce::FontOptions (9.0f, juce::Font::bold)));
    g.drawText (juce::String (index + 1), bounds.reduced (4, 3), juce::Justification::topLeft);

    // Waveform thumbnail (center area)
    if (drumPad.hasSample())
    {
        auto waveArea = bounds.reduced (4, 16);
        drawWaveformThumbnail (g, waveArea);
    }

    // Pad name (bottom)
    g.setColour (InstaDrumsLookAndFeel::textPrimary);
    g.setFont (juce::Font (juce::FontOptions (10.0f, juce::Font::bold)));
    g.drawText (drumPad.name, bounds.reduced (4, 2), juce::Justification::centredBottom);

    // Playing flash
    if (drumPad.isPlaying())
    {
        g.setColour (juce::Colours::white.withAlpha (0.12f));
        g.fillRoundedRectangle (bounds, cornerSize);
    }
}

void PadComponent::drawWaveformThumbnail (juce::Graphics& g, juce::Rectangle<float> area)
{
    auto& buf = drumPad.getSampleBuffer();
    if (buf.getNumSamples() == 0) return;

    const float* data = buf.getReadPointer (0);
    const int numSamples = buf.getNumSamples();
    const float w = area.getWidth();
    const float h = area.getHeight();
    const float midY = area.getCentreY();

    juce::Path path;
    int blockSize = std::max (1, numSamples / (int) w);

    for (int x = 0; x < (int) w; ++x)
    {
        int si = (int) ((float) x / w * numSamples);
        si = juce::jlimit (0, numSamples - 1, si);

        float maxVal = 0.0f;
        for (int j = 0; j < blockSize && (si + j) < numSamples; ++j)
            maxVal = std::max (maxVal, std::abs (data[si + j]));

        float topY = midY - maxVal * (h * 0.45f);
        float botY = midY + maxVal * (h * 0.45f);

        if (x == 0)
            path.startNewSubPath (area.getX() + (float) x, topY);

        path.lineTo (area.getX() + (float) x, topY);
    }
    // Mirror bottom
    for (int x = (int) w - 1; x >= 0; --x)
    {
        int si = (int) ((float) x / w * numSamples);
        si = juce::jlimit (0, numSamples - 1, si);
        float maxVal = 0.0f;
        for (int j = 0; j < blockSize && (si + j) < numSamples; ++j)
            maxVal = std::max (maxVal, std::abs (data[si + j]));
        float botY = midY + maxVal * (h * 0.45f);
        path.lineTo (area.getX() + (float) x, botY);
    }
    path.closeSubPath();

    // Greenish waveform tint
    g.setColour (juce::Colour (0xff44cc88).withAlpha (0.4f));
    g.fillPath (path);
    g.setColour (juce::Colour (0xff44cc88).withAlpha (0.7f));
    g.strokePath (path, juce::PathStrokeType (0.8f));
}

void PadComponent::resized() {}

void PadComponent::mouseDown (const juce::MouseEvent& event)
{
    if (event.mods.isRightButtonDown())
        return;

    // Select this pad
    if (onSelected)
        onSelected (index);

    isPressed = true;
    drumPad.trigger (1.0f);
    repaint();
}

void PadComponent::mouseUp (const juce::MouseEvent& event)
{
    if (event.mods.isRightButtonDown())
        return;

    isPressed = false;
    if (! drumPad.oneShot)
        drumPad.stop();
    repaint();
}

bool PadComponent::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (auto& f : files)
    {
        juce::File file (f);
        if (file.isDirectory()) return true;
        auto ext = file.getFileExtension().toLowerCase();
        if (ext == ".wav" || ext == ".aiff" || ext == ".aif" || ext == ".flac"
            || ext == ".ogg" || ext == ".mp3")
            return true;
    }
    return false;
}

void PadComponent::filesDropped (const juce::StringArray& files, int, int)
{
    isDragOver = false;
    if (! files.isEmpty() && onLoadSample)
        onLoadSample (index, juce::File (files[0]));
    repaint();
}

void PadComponent::fileDragEnter (const juce::StringArray&, int, int)
{
    isDragOver = true;
    repaint();
}

void PadComponent::fileDragExit (const juce::StringArray&)
{
    isDragOver = false;
    repaint();
}
