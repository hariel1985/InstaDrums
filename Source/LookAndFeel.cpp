#include "LookAndFeel.h"

InstaDrumsLookAndFeel::InstaDrumsLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, bgDark);
    setColour (juce::Label::textColourId, textPrimary);
    setColour (juce::TextButton::buttonColourId, bgMedium);
    setColour (juce::TextButton::textColourOffId, textPrimary);
}

void InstaDrumsLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                               float sliderPos, float rotaryStartAngle,
                                               float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (2.0f);
    auto radius = std::min (bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto centreX = bounds.getCentreX();
    auto centreY = bounds.getCentreY();
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Background arc
    juce::Path bgArc;
    bgArc.addCentredArc (centreX, centreY, radius - 2, radius - 2,
                          0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (bgLight);
    g.strokePath (bgArc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));

    // Value arc
    juce::Path valueArc;
    valueArc.addCentredArc (centreX, centreY, radius - 2, radius - 2,
                             0.0f, rotaryStartAngle, angle, true);
    g.setColour (accent);
    g.strokePath (valueArc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));

    // Pointer
    juce::Path pointer;
    auto pointerLength = radius * 0.5f;
    pointer.addRectangle (-1.5f, -pointerLength, 3.0f, pointerLength);
    pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));
    g.setColour (textPrimary);
    g.fillPath (pointer);
}
