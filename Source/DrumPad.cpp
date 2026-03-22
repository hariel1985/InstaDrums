#include "DrumPad.h"

DrumPad::DrumPad() {}
DrumPad::~DrumPad() {}

void DrumPad::prepareToPlay (double sr, int /*samplesPerBlock*/)
{
    sampleRate = sr;
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

    const auto& sampleBuffer = activeSample->buffer;
    const int sampleLength = sampleBuffer.getNumSamples();
    const int srcChannels = sampleBuffer.getNumChannels();
    const double sourceSR = activeSample->sampleRate;

    double pitchRatio = std::pow (2.0, (double) pitch / 12.0) * (sourceSR / sampleRate);

    // Constant power pan law
    float panPos = (pan + 1.0f) * 0.5f;
    float leftGain  = std::cos (panPos * juce::MathConstants<float>::halfPi);
    float rightGain = std::sin (panPos * juce::MathConstants<float>::halfPi);

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

            for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
            {
                int srcCh = std::min (ch, srcChannels - 1);
                float s0 = sampleBuffer.getSample (srcCh, pos0);
                float s1 = sampleBuffer.getSample (srcCh, pos1);
                float sampleVal = s0 + frac * (s1 - s0);

                float channelGain = (ch == 0) ? leftGain : rightGain;
                outputBuffer.addSample (ch, startSample + i, sampleVal * gain * channelGain);
            }
        }

        readPosition += pitchRatio;
    }
}
