#include "PluginProcessor.h"
#include "PluginEditor.h"

InstaDrumsProcessor::InstaDrumsProcessor()
    : AudioProcessor (BusesProperties()
                          .withOutput ("Main",  juce::AudioChannelSet::stereo(), true))
{
    formatManager.registerBasicFormats();
    initializeDefaults();
}

InstaDrumsProcessor::~InstaDrumsProcessor() {}

void InstaDrumsProcessor::initializeDefaults()
{
    // GM Drum Map defaults for first 12 pads
    struct PadDefault { int note; const char* name; juce::uint32 colour; };
    static const PadDefault defaults[] = {
        { 36, "Kick",     0xffff4444 },  // Red
        { 38, "Snare",    0xffff8844 },  // Orange
        { 42, "CH Hat",   0xffffff44 },  // Yellow
        { 46, "OH Hat",   0xff88ff44 },  // Green
        { 45, "Low Tom",  0xff44ffaa },  // Teal
        { 48, "Mid Tom",  0xff44ddff },  // Cyan
        { 50, "Hi Tom",   0xff4488ff },  // Blue
        { 49, "Crash",    0xff8844ff },  // Purple
        { 51, "Ride",     0xffcc44ff },  // Magenta
        { 39, "Clap",     0xffff44cc },  // Pink
        { 56, "Cowbell",  0xffff8888 },  // Light red
        { 37, "Rimshot",  0xffaaaaff },  // Light blue
    };

    for (int i = 0; i < defaultNumPads && i < (int) std::size (defaults); ++i)
    {
        pads[i].midiNote = defaults[i].note;
        pads[i].name     = defaults[i].name;
        pads[i].colour   = juce::Colour (defaults[i].colour);
    }
}

void InstaDrumsProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    for (int i = 0; i < numActivePads; ++i)
        pads[i].prepareToPlay (sampleRate, samplesPerBlock);

    // Master FX chain
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, 2 };

    reverb.prepare (spec);
    compressor.prepare (spec);

    juce::dsp::ProcessSpec monoSpec { sampleRate, (juce::uint32) samplesPerBlock, 1 };
    eqLoFilterL.prepare (monoSpec);  eqLoFilterR.prepare (monoSpec);
    eqMidFilterL.prepare (monoSpec); eqMidFilterR.prepare (monoSpec);
    eqHiFilterL.prepare (monoSpec);  eqHiFilterR.prepare (monoSpec);

    reverb.reset();
    compressor.reset();
    eqLoFilterL.reset();  eqLoFilterR.reset();
    eqMidFilterL.reset(); eqMidFilterR.reset();
    eqHiFilterL.reset();  eqHiFilterR.reset();
}

void InstaDrumsProcessor::releaseResources()
{
    for (int i = 0; i < numActivePads; ++i)
        pads[i].releaseResources();
}

void InstaDrumsProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    // Process MIDI messages
    for (const auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            auto* pad = findPadForNote (msg.getNoteNumber());
            if (pad != nullptr)
            {
                // Handle choke groups
                if (pad->chokeGroup >= 0)
                {
                    for (int i = 0; i < numActivePads; ++i)
                    {
                        if (&pads[i] != pad && pads[i].chokeGroup == pad->chokeGroup)
                            pads[i].stop();
                    }
                }
                pad->trigger (msg.getFloatVelocity());
            }
        }
        else if (msg.isNoteOff())
        {
            auto* pad = findPadForNote (msg.getNoteNumber());
            if (pad != nullptr && ! pad->oneShot)
                pad->stop();
        }
    }

    // Render audio from all pads
    for (int i = 0; i < numActivePads; ++i)
        pads[i].renderNextBlock (buffer, 0, buffer.getNumSamples());

    // Apply master FX chain
    applyMasterFx (buffer);
}

void InstaDrumsProcessor::applyMasterFx (juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();

    // --- Distortion (pre-EQ) ---
    float drive = distDrive.load();
    float dMix  = distMix.load();
    if (drive > 0.001f && dMix > 0.001f)
    {
        float driveGain = 1.0f + drive * 20.0f;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            float* data = buffer.getWritePointer (ch);
            for (int i = 0; i < numSamples; ++i)
            {
                float dry = data[i];
                float wet = std::tanh (dry * driveGain) / std::tanh (driveGain);
                data[i] = dry * (1.0f - dMix) + wet * dMix;
            }
        }
    }

    // --- 3-band EQ ---
    float lo  = eqLo.load();
    float mid = eqMid.load();
    float hi  = eqHi.load();

    if (std::abs (lo) > 0.1f || std::abs (mid) > 0.1f || std::abs (hi) > 0.1f)
    {
        auto loCoeffs  = juce::dsp::IIR::Coefficients<float>::makeLowShelf  (currentSampleRate, 200.0,  0.707f, juce::Decibels::decibelsToGain (lo));
        auto midCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter (currentSampleRate, 1000.0, 1.0f,   juce::Decibels::decibelsToGain (mid));
        auto hiCoeffs  = juce::dsp::IIR::Coefficients<float>::makeHighShelf (currentSampleRate, 5000.0, 0.707f, juce::Decibels::decibelsToGain (hi));

        *eqLoFilterL.coefficients  = *loCoeffs;  *eqLoFilterR.coefficients  = *loCoeffs;
        *eqMidFilterL.coefficients = *midCoeffs; *eqMidFilterR.coefficients = *midCoeffs;
        *eqHiFilterL.coefficients  = *hiCoeffs;  *eqHiFilterR.coefficients  = *hiCoeffs;

        if (buffer.getNumChannels() >= 2)
        {
            float* L = buffer.getWritePointer (0);
            float* R = buffer.getWritePointer (1);
            for (int i = 0; i < numSamples; ++i)
            {
                L[i] = eqLoFilterL.processSample (L[i]);
                L[i] = eqMidFilterL.processSample (L[i]);
                L[i] = eqHiFilterL.processSample (L[i]);
                R[i] = eqLoFilterR.processSample (R[i]);
                R[i] = eqMidFilterR.processSample (R[i]);
                R[i] = eqHiFilterR.processSample (R[i]);
            }
        }
    }

    // --- Compressor ---
    float thresh = compThreshold.load();
    float ratio  = compRatio.load();
    compressor.setThreshold (thresh);
    compressor.setRatio (ratio);
    compressor.setAttack (10.0f);
    compressor.setRelease (100.0f);
    {
        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        compressor.process (ctx);
    }

    // --- Reverb ---
    float rSize  = reverbSize.load();
    float rDecay = reverbDecay.load();
    if (rSize > 0.01f || rDecay > 0.01f)
    {
        juce::dsp::Reverb::Parameters rParams;
        rParams.roomSize = rSize;
        rParams.damping  = 1.0f - rDecay;
        rParams.wetLevel = rSize * 0.5f;
        rParams.dryLevel = 1.0f;
        rParams.width    = 1.0f;
        reverb.setParameters (rParams);

        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        reverb.process (ctx);
    }

    // --- Master Volume + Pan ---
    float mVol = masterVolume.load();
    float mPan = masterPan.load();
    float panPos = (mPan + 1.0f) * 0.5f;
    float mLGain = std::cos (panPos * juce::MathConstants<float>::halfPi) * mVol;
    float mRGain = std::sin (panPos * juce::MathConstants<float>::halfPi) * mVol;

    if (buffer.getNumChannels() >= 2)
    {
        buffer.applyGain (0, 0, numSamples, mLGain);
        buffer.applyGain (1, 0, numSamples, mRGain);
    }
    else
    {
        buffer.applyGain (mVol);
    }

    // --- VU Meter ---
    float rmsL = 0.0f, rmsR = 0.0f;
    if (buffer.getNumChannels() >= 1)
        rmsL = buffer.getRMSLevel (0, 0, numSamples);
    if (buffer.getNumChannels() >= 2)
        rmsR = buffer.getRMSLevel (1, 0, numSamples);

    // Smooth VU (simple exponential)
    vuLevelL.store (vuLevelL.load() * 0.8f + rmsL * 0.2f);
    vuLevelR.store (vuLevelR.load() * 0.8f + rmsR * 0.2f);
}

DrumPad* InstaDrumsProcessor::findPadForNote (int midiNote)
{
    for (int i = 0; i < numActivePads; ++i)
        if (pads[i].midiNote == midiNote)
            return &pads[i];
    return nullptr;
}

void InstaDrumsProcessor::loadSample (int padIndex, const juce::File& file)
{
    if (padIndex < 0 || padIndex >= numActivePads)
        return;

    if (file.isDirectory())
        pads[padIndex].loadLayersFromFolder (file, formatManager);
    else
        pads[padIndex].loadSample (file, formatManager);
}

void InstaDrumsProcessor::addPads (int count)
{
    int newCount = std::min (numActivePads + count, maxPads);
    for (int i = numActivePads; i < newCount; ++i)
    {
        pads[i].name = "Pad " + juce::String (i + 1);
        pads[i].midiNote = 36 + i;  // Sequential mapping
        pads[i].colour = juce::Colour::fromHSV ((float) i / 16.0f, 0.7f, 1.0f, 1.0f);
    }
    numActivePads = newCount;
}

void InstaDrumsProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::XmlElement xml ("InstaDrumsState");
    xml.setAttribute ("numPads", numActivePads);

    for (int i = 0; i < numActivePads; ++i)
    {
        auto* padXml = xml.createNewChildElement ("Pad");
        padXml->setAttribute ("index", i);
        padXml->setAttribute ("name", pads[i].name);
        padXml->setAttribute ("midiNote", pads[i].midiNote);
        padXml->setAttribute ("volume", (double) pads[i].volume);
        padXml->setAttribute ("pan", (double) pads[i].pan);
        padXml->setAttribute ("pitch", (double) pads[i].pitch);
        padXml->setAttribute ("oneShot", pads[i].oneShot);
        padXml->setAttribute ("chokeGroup", pads[i].chokeGroup);
        padXml->setAttribute ("attack", (double) pads[i].attack);
        padXml->setAttribute ("decay", (double) pads[i].decay);
        padXml->setAttribute ("sustain", (double) pads[i].sustain);
        padXml->setAttribute ("release", (double) pads[i].release);
        padXml->setAttribute ("colour", (int) pads[i].colour.getARGB());

        auto lf = pads[i].getLoadedFile();
        if (lf.existsAsFile() || lf.isDirectory())
            padXml->setAttribute ("samplePath", lf.getFullPathName());
    }

    copyXmlToBinary (xml, destData);
}

void InstaDrumsProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml != nullptr && xml->hasTagName ("InstaDrumsState"))
    {
        numActivePads = xml->getIntAttribute ("numPads", defaultNumPads);

        for (auto* padXml : xml->getChildWithTagNameIterator ("Pad"))
        {
            int index = padXml->getIntAttribute ("index", -1);
            if (index < 0 || index >= numActivePads)
                continue;

            pads[index].name       = padXml->getStringAttribute ("name", "Pad");
            pads[index].midiNote   = padXml->getIntAttribute ("midiNote", 36 + index);
            pads[index].volume     = (float) padXml->getDoubleAttribute ("volume", 1.0);
            pads[index].pan        = (float) padXml->getDoubleAttribute ("pan", 0.0);
            pads[index].pitch      = (float) padXml->getDoubleAttribute ("pitch", 0.0);
            pads[index].oneShot    = padXml->getBoolAttribute ("oneShot", true);
            pads[index].chokeGroup = padXml->getIntAttribute ("chokeGroup", -1);
            pads[index].attack     = (float) padXml->getDoubleAttribute ("attack", 0.001);
            pads[index].decay      = (float) padXml->getDoubleAttribute ("decay", 0.1);
            pads[index].sustain    = (float) padXml->getDoubleAttribute ("sustain", 1.0);
            pads[index].release    = (float) padXml->getDoubleAttribute ("release", 0.05);
            pads[index].colour     = juce::Colour ((juce::uint32) padXml->getIntAttribute ("colour", 0xff00ff88));

            juce::String path = padXml->getStringAttribute ("samplePath");
            if (path.isNotEmpty())
            {
                juce::File sampleFile (path);
                if (sampleFile.isDirectory())
                    pads[index].loadLayersFromFolder (sampleFile, formatManager);
                else if (sampleFile.existsAsFile())
                    pads[index].loadSample (sampleFile, formatManager);
            }
        }
    }
}

void InstaDrumsProcessor::loadKitFromFolder (const juce::File& folder)
{
    if (! folder.isDirectory())
        return;

    // Collect audio files from the folder
    juce::Array<juce::File> audioFiles;
    for (auto& f : folder.findChildFiles (juce::File::findFiles, false))
    {
        auto ext = f.getFileExtension().toLowerCase();
        if (ext == ".wav" || ext == ".aiff" || ext == ".aif" || ext == ".flac"
            || ext == ".ogg" || ext == ".mp3")
            audioFiles.add (f);
    }

    audioFiles.sort();

    // Try to match files to pads by name (kick, snare, etc.)
    auto matchPad = [&] (const juce::String& fileName) -> int
    {
        auto lower = fileName.toLowerCase();
        struct NameMatch { const char* keyword; int padIndex; };
        static const NameMatch matches[] = {
            { "kick",    0 }, { "bass",     0 }, { "bd",    0 },
            { "snare",   1 }, { "sn",       1 }, { "sd",    1 },
            { "closedhihat", 2 }, { "closedhi", 2 }, { "chh", 2 }, { "ch hat", 2 },
            { "openhihat",   3 }, { "openhi",   3 }, { "ohh", 3 }, { "oh hat", 3 },
            { "lowtom",  4 }, { "low tom",  4 }, { "lt",    4 },
            { "midtom",  5 }, { "mid tom",  5 }, { "mt",    5 },
            { "hitom",   6 }, { "hi tom",   6 }, { "ht",    6 },
            { "crash",   7 },
            { "ride",    8 },
            { "clap",    9 },
            { "cowbell", 10 }, { "bell",    10 },
            { "rim",     11 },
        };

        for (auto& m : matches)
            if (lower.contains (m.keyword))
                return m.padIndex;
        return -1;
    };

    // First pass: match by name
    juce::Array<bool> assigned;
    assigned.resize (numActivePads);
    for (int i = 0; i < numActivePads; ++i)
        assigned.set (i, false);

    for (auto& file : audioFiles)
    {
        int idx = matchPad (file.getFileNameWithoutExtension());
        if (idx >= 0 && idx < numActivePads && ! assigned[idx])
        {
            pads[idx].loadSample (file, formatManager);
            assigned.set (idx, true);
        }
    }

    // Second pass: assign remaining files to unassigned pads
    int nextPad = 0;
    for (auto& file : audioFiles)
    {
        int idx = matchPad (file.getFileNameWithoutExtension());
        if (idx >= 0 && idx < numActivePads && assigned[idx])
            continue; // Already assigned

        while (nextPad < numActivePads && assigned[nextPad])
            nextPad++;

        if (nextPad < numActivePads)
        {
            pads[nextPad].loadSample (file, formatManager);
            assigned.set (nextPad, true);
            nextPad++;
        }
    }
}

void InstaDrumsProcessor::saveKitPreset (const juce::File& file)
{
    juce::XmlElement xml ("InstaDrumsKit");
    xml.setAttribute ("version", "1.0");
    xml.setAttribute ("numPads", numActivePads);

    for (int i = 0; i < numActivePads; ++i)
    {
        auto* padXml = xml.createNewChildElement ("Pad");
        padXml->setAttribute ("index", i);
        padXml->setAttribute ("name", pads[i].name);
        padXml->setAttribute ("midiNote", pads[i].midiNote);
        padXml->setAttribute ("volume", (double) pads[i].volume);
        padXml->setAttribute ("pan", (double) pads[i].pan);
        padXml->setAttribute ("pitch", (double) pads[i].pitch);
        padXml->setAttribute ("oneShot", pads[i].oneShot);
        padXml->setAttribute ("chokeGroup", pads[i].chokeGroup);
        padXml->setAttribute ("attack", (double) pads[i].attack);
        padXml->setAttribute ("decay", (double) pads[i].decay);
        padXml->setAttribute ("sustain", (double) pads[i].sustain);
        padXml->setAttribute ("release", (double) pads[i].release);
        padXml->setAttribute ("colour", (int) pads[i].colour.getARGB());

        auto lf = pads[i].getLoadedFile();
        if (lf.existsAsFile() || lf.isDirectory())
            padXml->setAttribute ("samplePath", lf.getFullPathName());
    }

    xml.writeTo (file);
}

void InstaDrumsProcessor::loadKitPreset (const juce::File& file)
{
    auto xml = juce::XmlDocument::parse (file);
    if (xml == nullptr || ! xml->hasTagName ("InstaDrumsKit"))
        return;

    numActivePads = xml->getIntAttribute ("numPads", defaultNumPads);

    for (auto* padXml : xml->getChildWithTagNameIterator ("Pad"))
    {
        int index = padXml->getIntAttribute ("index", -1);
        if (index < 0 || index >= numActivePads)
            continue;

        pads[index].name       = padXml->getStringAttribute ("name", pads[index].name);
        pads[index].midiNote   = padXml->getIntAttribute ("midiNote", pads[index].midiNote);
        pads[index].volume     = (float) padXml->getDoubleAttribute ("volume", 1.0);
        pads[index].pan        = (float) padXml->getDoubleAttribute ("pan", 0.0);
        pads[index].pitch      = (float) padXml->getDoubleAttribute ("pitch", 0.0);
        pads[index].oneShot    = padXml->getBoolAttribute ("oneShot", true);
        pads[index].chokeGroup = padXml->getIntAttribute ("chokeGroup", -1);
        pads[index].attack     = (float) padXml->getDoubleAttribute ("attack", 0.001);
        pads[index].decay      = (float) padXml->getDoubleAttribute ("decay", 0.1);
        pads[index].sustain    = (float) padXml->getDoubleAttribute ("sustain", 1.0);
        pads[index].release    = (float) padXml->getDoubleAttribute ("release", 0.05);
        pads[index].colour     = juce::Colour ((juce::uint32) padXml->getIntAttribute ("colour", (int) pads[index].colour.getARGB()));

        juce::String path = padXml->getStringAttribute ("samplePath");
        if (path.isNotEmpty())
        {
            juce::File sampleFile (path);
            if (sampleFile.existsAsFile())
                pads[index].loadSample (sampleFile, formatManager);
        }
    }
}

juce::AudioProcessorEditor* InstaDrumsProcessor::createEditor()
{
    return new InstaDrumsEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new InstaDrumsProcessor();
}
