#include "DrumPad.h"

DrumPad::DrumPad() {}
DrumPad::~DrumPad() {}

void DrumPad::prepareToPlay (double sr, int samplesPerBlock)
{
    sampleRate = sr;
    blockSize = samplesPerBlock;

    tempBuffer.setSize (2, samplesPerBlock);

    // Per-pad filter
    juce::dsp::ProcessSpec monoSpec { sr, (juce::uint32) samplesPerBlock, 1 };
    filterL.prepare (monoSpec); filterR.prepare (monoSpec);
    filterL.reset(); filterR.reset();
    lastCutoff = filterCutoff;

    // Per-pad FX
    juce::dsp::ProcessSpec stereoSpec { sr, (juce::uint32) samplesPerBlock, 2 };
    padCompressor.prepare (stereoSpec);
    padCompressor.reset();
    padReverb.prepare (stereoSpec);
    padReverb.reset();

    padEqLoL.prepare (monoSpec);  padEqLoR.prepare (monoSpec);
    padEqMidL.prepare (monoSpec); padEqMidR.prepare (monoSpec);
    padEqHiL.prepare (monoSpec);  padEqHiR.prepare (monoSpec);
    padEqLoL.reset();  padEqLoR.reset();
    padEqMidL.reset(); padEqMidR.reset();
    padEqHiL.reset();  padEqHiR.reset();
}

void DrumPad::releaseResources()
{
    playing = false;
    activeSample = nullptr;
    envStage = EnvelopeStage::Idle;
    envLevel = 0.0f;
}

// ============================================================
// Velocity tag parsing from Salamander-style filenames
// Tags: Ghost, PP, P, MP, F, FF
// ============================================================

float DrumPad::velocityTagToLow (const juce::String& tag)
{
    if (tag == "Ghost") return 0.0f;
    if (tag == "PP")    return 0.05f;
    if (tag == "P")     return 0.15f;
    if (tag == "MP")    return 0.35f;
    if (tag == "F")     return 0.55f;
    if (tag == "FF")    return 0.80f;
    return 0.0f;
}

float DrumPad::velocityTagToHigh (const juce::String& tag)
{
    if (tag == "Ghost") return 0.05f;
    if (tag == "PP")    return 0.15f;
    if (tag == "P")     return 0.35f;
    if (tag == "MP")    return 0.55f;
    if (tag == "F")     return 0.80f;
    if (tag == "FF")    return 1.0f;
    return 1.0f;
}

// ============================================================
// Single sample loading (one layer, full velocity range)
// ============================================================

void DrumPad::loadSample (const juce::File& file, juce::AudioFormatManager& formatManager)
{
    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
    if (reader == nullptr) return;

    layers.clear();
    activeSample = nullptr;

    auto* layer = new VelocityLayer();
    layer->velocityLow = 0.0f;
    layer->velocityHigh = 1.0f;

    auto* sample = new Sample();
    sample->buffer.setSize ((int) reader->numChannels, (int) reader->lengthInSamples);
    reader->read (&sample->buffer, 0, (int) reader->lengthInSamples, 0, true, true);
    sample->sampleRate = reader->sampleRate;
    sample->file = file;

    layer->samples.add (sample);
    layers.add (layer);

    loadedFileName = file.getFileName();
    loadedFile = file;
    readPosition = 0.0;
    playing = false;
}

// ============================================================
// Velocity layer loading from folder
// Expects filenames like: snare_OH_FF_1.flac, snare_OH_Ghost_3.flac
// Groups by velocity tag, each group becomes round-robin variations
// ============================================================

void DrumPad::loadLayersFromFolder (const juce::File& folder, juce::AudioFormatManager& formatManager)
{
    if (! folder.isDirectory()) return;

    layers.clear();
    activeSample = nullptr;

    // Collect audio files
    juce::Array<juce::File> audioFiles;
    for (auto& f : folder.findChildFiles (juce::File::findFiles, false))
    {
        auto ext = f.getFileExtension().toLowerCase();
        if (ext == ".wav" || ext == ".aiff" || ext == ".aif" || ext == ".flac"
            || ext == ".ogg" || ext == ".mp3")
            audioFiles.add (f);
    }

    if (audioFiles.isEmpty()) return;

    // Known velocity tags to look for in filenames
    static const juce::StringArray velocityTags = { "Ghost", "PP", "P", "MP", "F", "FF" };

    // Group files by velocity tag
    std::map<juce::String, juce::Array<juce::File>> groups;

    for (auto& file : audioFiles)
    {
        auto nameNoExt = file.getFileNameWithoutExtension();
        // Split by underscore and look for velocity tags
        juce::String foundTag = "FF"; // default if no tag found

        auto parts = juce::StringArray::fromTokens (nameNoExt, "_", "");
        for (auto& part : parts)
        {
            if (velocityTags.contains (part))
            {
                foundTag = part;
                break;
            }
        }

        groups[foundTag].add (file);
    }

    // If only one group found with no velocity differentiation, treat as single layer
    if (groups.size() == 1 && groups.begin()->first == "FF")
    {
        // All files are round-robin for a single full-velocity layer
        auto* layer = new VelocityLayer();
        layer->velocityLow = 0.0f;
        layer->velocityHigh = 1.0f;

        for (auto& file : groups.begin()->second)
        {
            std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
            if (reader != nullptr)
            {
                auto* sample = new Sample();
                sample->buffer.setSize ((int) reader->numChannels, (int) reader->lengthInSamples);
                reader->read (&sample->buffer, 0, (int) reader->lengthInSamples, 0, true, true);
                sample->sampleRate = reader->sampleRate;
                sample->file = file;
                layer->samples.add (sample);
            }
        }

        if (! layer->samples.isEmpty())
            layers.add (layer);
    }
    else
    {
        // Multiple velocity groups — create one layer per group
        for (auto& [tag, files] : groups)
        {
            auto* layer = new VelocityLayer();
            layer->velocityLow = velocityTagToLow (tag);
            layer->velocityHigh = velocityTagToHigh (tag);

            files.sort();
            for (auto& file : files)
            {
                std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
                if (reader != nullptr)
                {
                    auto* sample = new Sample();
                    sample->buffer.setSize ((int) reader->numChannels, (int) reader->lengthInSamples);
                    reader->read (&sample->buffer, 0, (int) reader->lengthInSamples, 0, true, true);
                    sample->sampleRate = reader->sampleRate;
                    sample->file = file;
                    layer->samples.add (sample);
                }
            }

            if (! layer->samples.isEmpty())
                layers.add (layer);
        }
    }

    // Sort layers by velocity range
    std::sort (layers.begin(), layers.end(),
               [] (const VelocityLayer* a, const VelocityLayer* b)
               { return a->velocityLow < b->velocityLow; });

    loadedFileName = folder.getFileName() + " (" + juce::String (layers.size()) + " layers)";
    loadedFile = folder;
    readPosition = 0.0;
    playing = false;
}

// ============================================================
// State queries
// ============================================================

bool DrumPad::hasSample() const
{
    for (auto* layer : layers)
        if (! layer->samples.isEmpty())
            return true;
    return false;
}

const juce::AudioBuffer<float>& DrumPad::getSampleBuffer() const
{
    if (activeSample != nullptr)
        return activeSample->buffer;

    // Return first available sample buffer for waveform display
    for (auto* layer : layers)
        if (! layer->samples.isEmpty())
            return layer->samples[0]->buffer;

    return emptyBuffer;
}

// ============================================================
// Velocity layer selection
// ============================================================

DrumPad::VelocityLayer* DrumPad::findLayerForVelocity (float velocity)
{
    // Find the layer whose range contains this velocity
    for (auto* layer : layers)
        if (velocity >= layer->velocityLow && velocity <= layer->velocityHigh)
            return layer;

    // Fallback: closest layer
    VelocityLayer* closest = nullptr;
    float minDist = 2.0f;
    for (auto* layer : layers)
    {
        float mid = (layer->velocityLow + layer->velocityHigh) * 0.5f;
        float dist = std::abs (velocity - mid);
        if (dist < minDist)
        {
            minDist = dist;
            closest = layer;
        }
    }
    return closest;
}

// ============================================================
// Trigger / Stop
// ============================================================

void DrumPad::trigger (float velocity)
{
    if (! hasSample()) return;

    auto* layer = findLayerForVelocity (velocity);
    if (layer == nullptr) return;

    activeSample = layer->getNextSample();
    if (activeSample == nullptr) return;

    currentVelocity = velocity;
    readPosition = 0.0;
    envStage = EnvelopeStage::Attack;
    envLevel = 0.0f;
    playing = true;
}

void DrumPad::stop()
{
    if (playing)
        envStage = EnvelopeStage::Release;
}

// ============================================================
// ADSR Envelope
// ============================================================

void DrumPad::advanceEnvelope()
{
    float attackSamples  = std::max (1.0f, attack * (float) sampleRate);
    float decaySamples   = std::max (1.0f, decay * (float) sampleRate);
    float releaseSamples = std::max (1.0f, release * (float) sampleRate);

    switch (envStage)
    {
        case EnvelopeStage::Attack:
            envLevel += 1.0f / attackSamples;
            if (envLevel >= 1.0f)
            {
                envLevel = 1.0f;
                envStage = EnvelopeStage::Decay;
            }
            break;

        case EnvelopeStage::Decay:
            envLevel -= (1.0f - sustain) / decaySamples;
            if (envLevel <= sustain)
            {
                envLevel = sustain;
                envStage = EnvelopeStage::Sustain;
            }
            break;

        case EnvelopeStage::Sustain:
            envLevel = sustain;
            break;

        case EnvelopeStage::Release:
            envLevel -= envLevel / releaseSamples;
            if (envLevel < 0.001f)
            {
                envLevel = 0.0f;
                envStage = EnvelopeStage::Idle;
                playing = false;
            }
            break;

        case EnvelopeStage::Idle:
            envLevel = 0.0f;
            break;
    }
}

// ============================================================
// Audio rendering
// ============================================================

void DrumPad::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (! playing || activeSample == nullptr)
        return;

    // Ensure temp buffer is large enough
    if (tempBuffer.getNumSamples() < numSamples)
        tempBuffer.setSize (2, numSamples, false, false, true);

    tempBuffer.clear (0, numSamples);

    const auto& sampleBuffer = activeSample->buffer;
    const int sampleLength = sampleBuffer.getNumSamples();
    const int srcChannels = sampleBuffer.getNumChannels();
    const double sourceSR = activeSample->sampleRate;

    double pitchRatio = std::pow (2.0, (double) pitch / 12.0) * (sourceSR / sampleRate);

    // Constant power pan law
    float panPos = (pan + 1.0f) * 0.5f;
    float leftGain  = std::cos (panPos * juce::MathConstants<float>::halfPi);
    float rightGain = std::sin (panPos * juce::MathConstants<float>::halfPi);

    // Update filter coefficients if cutoff changed
    if (std::abs (filterCutoff - lastCutoff) > 1.0f)
    {
        float clampedCutoff = juce::jlimit (20.0f, (float) (sampleRate * 0.49), filterCutoff);
        auto coeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, clampedCutoff, filterReso);
        *filterL.coefficients = *coeffs;
        *filterR.coefficients = *coeffs;
        lastCutoff = filterCutoff;
    }

    bool useFilter = filterCutoff < 19900.0f;

    // Render into temp buffer
    for (int i = 0; i < numSamples; ++i)
    {
        if (! playing) break;

        int pos0 = (int) readPosition;
        if (pos0 >= sampleLength)
        {
            if (oneShot)
            {
                playing = false;
                envStage = EnvelopeStage::Idle;
                envLevel = 0.0f;
                activeSample = nullptr;
                break;
            }
            else
            {
                envStage = EnvelopeStage::Release;
            }
        }

        if (pos0 < sampleLength)
        {
            advanceEnvelope();
            float gain = volume * currentVelocity * envLevel;

            int pos1 = std::min (pos0 + 1, sampleLength - 1);
            float frac = (float) (readPosition - (double) pos0);

            for (int ch = 0; ch < 2; ++ch)
            {
                int srcCh = std::min (ch, srcChannels - 1);
                float s0 = sampleBuffer.getSample (srcCh, pos0);
                float s1 = sampleBuffer.getSample (srcCh, pos1);
                float sampleVal = s0 + frac * (s1 - s0);

                if (useFilter)
                    sampleVal = (ch == 0) ? filterL.processSample (sampleVal)
                                          : filterR.processSample (sampleVal);

                float channelGain = (ch == 0) ? leftGain : rightGain;
                tempBuffer.setSample (ch, i, sampleVal * gain * channelGain);
            }
        }

        readPosition += pitchRatio;
    }

    // Apply per-pad FX chain to temp buffer
    applyPadFx (tempBuffer, numSamples);

    // Mix temp buffer into output
    for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
        outputBuffer.addFrom (ch, startSample, tempBuffer, std::min (ch, 1), 0, numSamples);
}

// ============================================================
// Per-pad FX chain
// ============================================================

void DrumPad::applyPadFx (juce::AudioBuffer<float>& buf, int numSamples)
{
    // --- Distortion ---
    if (fxDistEnabled && fxDistDrive > 0.001f && fxDistMix > 0.001f)
    {
        float driveGain = 1.0f + fxDistDrive * 20.0f;
        for (int ch = 0; ch < buf.getNumChannels(); ++ch)
        {
            float* data = buf.getWritePointer (ch);
            for (int i = 0; i < numSamples; ++i)
            {
                float dry = data[i];
                float wet = std::tanh (dry * driveGain) / std::tanh (driveGain);
                data[i] = dry * (1.0f - fxDistMix) + wet * fxDistMix;
            }
        }
    }

    // --- EQ ---
    if (fxEqEnabled && (std::abs (fxEqLo) > 0.1f || std::abs (fxEqMid) > 0.1f || std::abs (fxEqHi) > 0.1f))
    {
        auto loC  = juce::dsp::IIR::Coefficients<float>::makeLowShelf  (sampleRate, 200.0, 0.707f, juce::Decibels::decibelsToGain (fxEqLo));
        auto midC = juce::dsp::IIR::Coefficients<float>::makePeakFilter (sampleRate, 1000.0, 1.0f, juce::Decibels::decibelsToGain (fxEqMid));
        auto hiC  = juce::dsp::IIR::Coefficients<float>::makeHighShelf (sampleRate, 5000.0, 0.707f, juce::Decibels::decibelsToGain (fxEqHi));

        *padEqLoL.coefficients = *loC;   *padEqLoR.coefficients = *loC;
        *padEqMidL.coefficients = *midC; *padEqMidR.coefficients = *midC;
        *padEqHiL.coefficients = *hiC;   *padEqHiR.coefficients = *hiC;

        float* L = buf.getWritePointer (0);
        float* R = buf.getWritePointer (1);
        for (int i = 0; i < numSamples; ++i)
        {
            L[i] = padEqHiL.processSample (padEqMidL.processSample (padEqLoL.processSample (L[i])));
            R[i] = padEqHiR.processSample (padEqMidR.processSample (padEqLoR.processSample (R[i])));
        }
    }

    // --- Compressor ---
    if (fxCompEnabled)
    {
        float peakLevel = 0.0f;
        for (int ch = 0; ch < buf.getNumChannels(); ++ch)
            peakLevel = std::max (peakLevel, buf.getMagnitude (ch, 0, numSamples));
        float inputDb = juce::Decibels::gainToDecibels (peakLevel, -80.0f);

        float gr = 0.0f;
        if (inputDb > fxCompThreshold && fxCompRatio > 1.0f)
            gr = (inputDb - fxCompThreshold) * (1.0f - 1.0f / fxCompRatio);

        float prevGr = std::abs (compGainReduction.load());
        if (gr > prevGr)
            compGainReduction.store (-(prevGr * 0.3f + gr * 0.7f));
        else
            compGainReduction.store (-(prevGr * 0.92f + gr * 0.08f));

        padCompressor.setThreshold (fxCompThreshold);
        padCompressor.setRatio (fxCompRatio);
        padCompressor.setAttack (10.0f);
        padCompressor.setRelease (100.0f);
        juce::dsp::AudioBlock<float> block (buf);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        padCompressor.process (ctx);
    }
    else
    {
        float prev = std::abs (compGainReduction.load());
        compGainReduction.store (-(prev * 0.9f));
    }

    // --- Reverb ---
    if (fxReverbEnabled && (fxReverbSize > 0.01f || fxReverbDecay > 0.01f))
    {
        juce::dsp::Reverb::Parameters rp;
        rp.roomSize = fxReverbSize;
        rp.damping  = 1.0f - fxReverbDecay;
        rp.wetLevel = fxReverbSize * 0.5f;
        rp.dryLevel = 1.0f;
        rp.width    = 1.0f;
        padReverb.setParameters (rp);
        juce::dsp::AudioBlock<float> block (buf);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        padReverb.process (ctx);
    }
}
