#include "LookAndFeel.h"
#include "BinaryData.h"

InstaDrumsLookAndFeel::InstaDrumsLookAndFeel()
{
    // Load embedded fonts
    typefaceRegular = juce::Typeface::createSystemTypefaceFor (
        BinaryData::RajdhaniRegular_ttf, BinaryData::RajdhaniRegular_ttfSize);
    typefaceMedium = juce::Typeface::createSystemTypefaceFor (
        BinaryData::RajdhaniMedium_ttf, BinaryData::RajdhaniMedium_ttfSize);
    typefaceBold = juce::Typeface::createSystemTypefaceFor (
        BinaryData::RajdhaniBold_ttf, BinaryData::RajdhaniBold_ttfSize);

    setColour (juce::ResizableWindow::backgroundColourId, bgDark);
    setColour (juce::Label::textColourId, textPrimary);
    setColour (juce::TextButton::buttonColourId, bgMedium);
    setColour (juce::TextButton::textColourOffId, textPrimary);

    generateNoiseTexture();
}

juce::Typeface::Ptr InstaDrumsLookAndFeel::getTypefaceForFont (const juce::Font& font)
{
    if (font.isBold())
        return typefaceBold;
    return typefaceRegular;
}

juce::Font InstaDrumsLookAndFeel::getRegularFont (float height) const
{
    return juce::Font (juce::FontOptions (typefaceRegular).withHeight (height));
}

juce::Font InstaDrumsLookAndFeel::getMediumFont (float height) const
{
    return juce::Font (juce::FontOptions (typefaceMedium).withHeight (height));
}

juce::Font InstaDrumsLookAndFeel::getBoldFont (float height) const
{
    return juce::Font (juce::FontOptions (typefaceBold).withHeight (height));
}

// ============================================================
// Background noise texture (subtle carbon fiber / grain effect)
// ============================================================

void InstaDrumsLookAndFeel::generateNoiseTexture()
{
    const int texW = 256, texH = 256;
    noiseTexture = juce::Image (juce::Image::ARGB, texW, texH, true);

    juce::Random rng (42);

    for (int y = 0; y < texH; ++y)
    {
        for (int x = 0; x < texW; ++x)
        {
            // Subtle noise grain
            float noise = rng.nextFloat() * 0.06f;

            // Carbon fiber pattern: diagonal cross-hatch
            bool crossA = ((x + y) % 4 == 0);
            bool crossB = ((x - y + 256) % 4 == 0);
            float pattern = (crossA || crossB) ? 0.03f : 0.0f;

            float alpha = noise + pattern;
            noiseTexture.setPixelAt (x, y, juce::Colour::fromFloatRGBA (1.0f, 1.0f, 1.0f, alpha));
        }
    }
}

void InstaDrumsLookAndFeel::drawBackgroundTexture (juce::Graphics& g, juce::Rectangle<int> area)
{
    // Tile the noise texture
    for (int y = area.getY(); y < area.getBottom(); y += noiseTexture.getHeight())
        for (int x = area.getX(); x < area.getRight(); x += noiseTexture.getWidth())
            g.drawImageAt (noiseTexture, x, y);
}

// ============================================================
// Rotary slider (3D metal knob)
// ============================================================

void InstaDrumsLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                               float sliderPos, float rotaryStartAngle,
                                               float rotaryEndAngle, juce::Slider& slider)
{
    // Scale factor based on knob pixel size (reference: 60px)
    float knobSize = std::min ((float) width, (float) height);
    float s = knobSize / 60.0f;  // 1.0 at 60px, smaller below, larger above
    float margin = std::max (4.0f, 6.0f * s);

    auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (margin);
    auto radius = std::min (bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto cx = bounds.getCentreX();
    auto cy = bounds.getCentreY();
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    auto knobType = slider.getProperties() [knobTypeProperty].toString();
    bool isDark = (knobType == "dark");

    juce::Colour arcColour     = isDark ? juce::Colour (0xff4488ff) : juce::Colour (0xffff8833);
    juce::Colour arcBgColour   = isDark ? juce::Colour (0xff1a2a44) : juce::Colour (0xff2a1a0a);
    juce::Colour bodyTop       = isDark ? juce::Colour (0xff3a3a4a) : juce::Colour (0xff5a4a3a);
    juce::Colour bodyBottom    = isDark ? juce::Colour (0xff1a1a2a) : juce::Colour (0xff2a1a0a);
    juce::Colour rimColour     = isDark ? juce::Colour (0xff555566) : juce::Colour (0xff886644);
    juce::Colour highlightCol  = isDark ? juce::Colour (0x33aabbff) : juce::Colour (0x44ffcc88);
    juce::Colour pointerColour = isDark ? juce::Colour (0xff66aaff) : juce::Colour (0xffffaa44);

    // All thicknesses scale with knob size
    float arcW     = std::max (1.5f, 2.5f * s);   // core arc width
    float glowW1   = std::max (3.0f, 10.0f * s);  // outermost glow
    float glowW2   = std::max (2.5f, 7.0f * s);   // mid glow
    float glowW3   = std::max (2.0f, 4.5f * s);   // inner glow
    float hotW     = std::max (0.8f, 1.2f * s);   // hot center
    float ptrW     = std::max (1.2f, 2.0f * s);   // pointer width
    float bodyRadius = radius * 0.72f;

    // 1. Drop shadow
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.fillEllipse (cx - bodyRadius + 1, cy - bodyRadius + 2, bodyRadius * 2, bodyRadius * 2);

    // 2. Outer arc track
    {
        juce::Path arcBg;
        arcBg.addCentredArc (cx, cy, radius - 1, radius - 1, 0.0f,
                              rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (arcBgColour);
        g.strokePath (arcBg, juce::PathStrokeType (arcW, juce::PathStrokeType::curved,
                                                    juce::PathStrokeType::rounded));
    }

    // 3. Outer arc value with scaled glow
    if (sliderPos > 0.01f)
    {
        juce::Path arcVal;
        arcVal.addCentredArc (cx, cy, radius - 1, radius - 1, 0.0f,
                               rotaryStartAngle, angle, true);

        // Glow layers (scale with knob size)
        g.setColour (arcColour.withAlpha (0.08f));
        g.strokePath (arcVal, juce::PathStrokeType (glowW1, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));
        g.setColour (arcColour.withAlpha (0.15f));
        g.strokePath (arcVal, juce::PathStrokeType (glowW2, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));
        g.setColour (arcColour.withAlpha (0.3f));
        g.strokePath (arcVal, juce::PathStrokeType (glowW3, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));

        // Core arc
        g.setColour (arcColour);
        g.strokePath (arcVal, juce::PathStrokeType (arcW, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));

        // Hot center
        g.setColour (arcColour.brighter (0.6f).withAlpha (0.5f));
        g.strokePath (arcVal, juce::PathStrokeType (hotW, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));
    }

    // 4. Knob body — radial gradient
    {
        juce::ColourGradient bodyGrad (bodyTop, cx, cy - bodyRadius * 0.5f,
                                       bodyBottom, cx, cy + bodyRadius, true);
        g.setGradientFill (bodyGrad);
        g.fillEllipse (cx - bodyRadius, cy - bodyRadius, bodyRadius * 2, bodyRadius * 2);
    }

    // 5. Rim
    g.setColour (rimColour.withAlpha (0.6f));
    g.drawEllipse (cx - bodyRadius, cy - bodyRadius, bodyRadius * 2, bodyRadius * 2, std::max (0.8f, 1.2f * s));

    // 6. Inner shadow
    {
        float innerR = bodyRadius * 0.85f;
        juce::ColourGradient innerGrad (juce::Colours::black.withAlpha (0.15f), cx, cy - innerR * 0.3f,
                                         juce::Colours::transparentBlack, cx, cy + innerR, true);
        g.setGradientFill (innerGrad);
        g.fillEllipse (cx - innerR, cy - innerR, innerR * 2, innerR * 2);
    }

    // 7. Top highlight
    {
        float hlRadius = bodyRadius * 0.55f;
        float hlY = cy - bodyRadius * 0.35f;
        juce::ColourGradient hlGrad (highlightCol, cx, hlY - hlRadius * 0.5f,
                                      juce::Colours::transparentBlack, cx, hlY + hlRadius, true);
        g.setGradientFill (hlGrad);
        g.fillEllipse (cx - hlRadius, hlY - hlRadius * 0.6f, hlRadius * 2, hlRadius * 1.2f);
    }

    // 8. Pointer with scaled glow
    {
        juce::Path pointer;
        float pointerLen = bodyRadius * 0.75f;

        pointer.addRoundedRectangle (-ptrW * 0.5f, -pointerLen, ptrW, pointerLen * 0.55f, ptrW * 0.5f);
        pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (cx, cy));

        // Wide outer glow
        g.setColour (pointerColour.withAlpha (0.1f));
        {
            juce::Path glow3;
            float gw = ptrW * 3.5f;
            glow3.addRoundedRectangle (-gw, -pointerLen, gw * 2, pointerLen * 0.55f, ptrW * 1.5f);
            glow3.applyTransform (juce::AffineTransform::rotation (angle).translated (cx, cy));
            g.fillPath (glow3);
        }

        // Medium glow
        g.setColour (pointerColour.withAlpha (0.25f));
        {
            juce::Path glow2;
            float gw = ptrW * 2.0f;
            glow2.addRoundedRectangle (-gw, -pointerLen, gw * 2, pointerLen * 0.55f, ptrW);
            glow2.applyTransform (juce::AffineTransform::rotation (angle).translated (cx, cy));
            g.fillPath (glow2);
        }

        // Core pointer
        g.setColour (pointerColour);
        g.fillPath (pointer);

        // Hot center
        {
            juce::Path hotCenter;
            float hw = ptrW * 0.3f;
            hotCenter.addRoundedRectangle (-hw, -pointerLen, hw * 2, pointerLen * 0.5f, hw);
            hotCenter.applyTransform (juce::AffineTransform::rotation (angle).translated (cx, cy));
            g.setColour (pointerColour.brighter (0.7f).withAlpha (0.6f));
            g.fillPath (hotCenter);
        }
    }

    // 9. Center cap
    {
        float capR = bodyRadius * 0.18f;
        juce::ColourGradient capGrad (rimColour.brighter (0.3f), cx, cy - capR,
                                       bodyBottom, cx, cy + capR, false);
        g.setGradientFill (capGrad);
        g.fillEllipse (cx - capR, cy - capR, capR * 2, capR * 2);
    }
}

// ============================================================
// Button style
// ============================================================

void InstaDrumsLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                                    const juce::Colour& backgroundColour,
                                                    bool shouldDrawButtonAsHighlighted,
                                                    bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);

    auto baseColour = backgroundColour;
    if (shouldDrawButtonAsDown)
        baseColour = baseColour.brighter (0.2f);
    else if (shouldDrawButtonAsHighlighted)
        baseColour = baseColour.brighter (0.1f);

    juce::ColourGradient grad (baseColour.brighter (0.05f), 0, bounds.getY(),
                                baseColour.darker (0.1f), 0, bounds.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (bounds, 4.0f);

    g.setColour (bgLight.withAlpha (shouldDrawButtonAsHighlighted ? 0.8f : 0.5f));
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
}
