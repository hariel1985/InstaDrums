#include "PluginEditor.h"

InstaDrumsEditor::InstaDrumsEditor (InstaDrumsProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&customLookAndFeel);
    juce::LookAndFeel::setDefaultLookAndFeel (&customLookAndFeel);

    // Title
    titleLabel.setFont (customLookAndFeel.getBoldFont (25.0f));
    titleLabel.setColour (juce::Label::textColourId, InstaDrumsLookAndFeel::accent);
    titleLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (titleLabel);

    versionLabel.setFont (juce::FontOptions (12.5f));
    versionLabel.setColour (juce::Label::textColourId, InstaDrumsLookAndFeel::textSecondary);
    versionLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (versionLabel);

    padsLabel.setFont (juce::FontOptions (12.5f, juce::Font::bold));
    padsLabel.setColour (juce::Label::textColourId, InstaDrumsLookAndFeel::textSecondary);
    addAndMakeVisible (padsLabel);

    // Buttons
    auto styleBtn = [this] (juce::TextButton& btn) {
        btn.setColour (juce::TextButton::buttonColourId, InstaDrumsLookAndFeel::bgLight);
        btn.setColour (juce::TextButton::textColourOffId, InstaDrumsLookAndFeel::textPrimary);
        addAndMakeVisible (btn);
    };

    styleBtn (loadSampleButton);
    styleBtn (saveKitButton);
    styleBtn (loadKitButton);
    styleBtn (loadFolderButton);

    loadSampleButton.onClick = [this]
    {
        fileChooser = std::make_unique<juce::FileChooser> ("Load Sample", juce::File{},
            "*.wav;*.aiff;*.aif;*.flac;*.ogg;*.mp3");
        fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this] (const juce::FileChooser& fc) {
                auto file = fc.getResult();
                if (file.existsAsFile())
                {
                    processor.loadSample (selectedPadIndex, file);
                    sampleEditor.updateFromPad();
                }
            });
    };

    saveKitButton.onClick = [this]
    {
        fileChooser = std::make_unique<juce::FileChooser> ("Save Kit", juce::File{}, "*.drumkit");
        fileChooser->launchAsync (juce::FileBrowserComponent::saveMode,
            [this] (const juce::FileChooser& fc) {
                auto file = fc.getResult();
                if (file != juce::File{})
                    processor.saveKitPreset (file.hasFileExtension (".drumkit") ? file : file.withFileExtension ("drumkit"));
            });
    };

    loadKitButton.onClick = [this]
    {
        fileChooser = std::make_unique<juce::FileChooser> ("Load Kit", juce::File{}, "*.drumkit");
        fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this] (const juce::FileChooser& fc) {
                auto file = fc.getResult();
                if (file.existsAsFile())
                {
                    processor.loadKitPreset (file);
                    rebuildPadGrid();
                    selectPad (0);
                }
            });
    };

    loadFolderButton.onClick = [this]
    {
        fileChooser = std::make_unique<juce::FileChooser> ("Select Sample Folder", juce::File{});
        fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
            [this] (const juce::FileChooser& fc) {
                auto folder = fc.getResult();
                if (folder.isDirectory())
                {
                    processor.loadKitFromFolder (folder);
                    rebuildPadGrid();
                    selectPad (0);
                }
            });
    };

    // Panels
    addAndMakeVisible (sampleEditor);
    addAndMakeVisible (fxPanel);
    addAndMakeVisible (masterPanel);

    rebuildPadGrid();
    selectPad (0);

    // Sizing
    constrainer.setMinimumSize (800, 500);
    constrainer.setMaximumSize (1920, 1080);
    setConstrainer (&constrainer);
    setSize (960, 600);
    setResizable (true, true);

    startTimerHz (30);
}

InstaDrumsEditor::~InstaDrumsEditor()
{
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
    setLookAndFeel (nullptr);
}

void InstaDrumsEditor::rebuildPadGrid()
{
    padComponents.clear();

    auto loadCallback = [this] (int padIndex, const juce::File& file) {
        processor.loadSample (padIndex, file);
        if (padIndex == selectedPadIndex)
            sampleEditor.updateFromPad();
    };

    for (int i = 0; i < processor.getNumPads(); ++i)
    {
        auto* pc = new PadComponent (processor.getPad (i), loadCallback, i);
        pc->onSelected = [this] (int idx) { selectPad (idx); };
        addAndMakeVisible (pc);
        padComponents.add (pc);
    }

    resized();
}

void InstaDrumsEditor::selectPad (int index)
{
    selectedPadIndex = index;

    for (int i = 0; i < padComponents.size(); ++i)
        padComponents[i]->setSelected (i == index);

    if (index >= 0 && index < processor.getNumPads())
    {
        sampleEditor.setPad (&processor.getPad (index));
        fxPanel.setPad (&processor.getPad (index));
    }
}

void InstaDrumsEditor::paint (juce::Graphics& g)
{
    // Background gradient
    juce::ColourGradient bgGrad (InstaDrumsLookAndFeel::bgDark, 0, 0,
                                  InstaDrumsLookAndFeel::bgDark.darker (0.3f), 0, (float) getHeight(), false);
    g.setGradientFill (bgGrad);
    g.fillAll();

    // Noise texture overlay
    customLookAndFeel.drawBackgroundTexture (g, getLocalBounds());

    // Top header bar
    float sc = (float) getHeight() / 600.0f;
    int topH = std::max (28, (int) (36 * sc));
    g.setColour (InstaDrumsLookAndFeel::bgDark.darker (0.4f));
    g.fillRect (0, 0, getWidth(), topH);
    g.setColour (InstaDrumsLookAndFeel::bgLight.withAlpha (0.3f));
    g.drawHorizontalLine (topH, 0, (float) getWidth());

    // Divider lines
    auto bounds = getLocalBounds();
    int rightPanelX = (int) (bounds.getWidth() * 0.52f);
    int masterH = std::max (50, (int) (65 * sc));
    int bottomPanelY = bounds.getHeight() - masterH - 4;

    g.setColour (InstaDrumsLookAndFeel::bgLight.withAlpha (0.4f));
    g.drawVerticalLine (rightPanelX - 2, 30, (float) bottomPanelY);
    g.drawHorizontalLine (bottomPanelY - 1, 0, (float) bounds.getWidth());
}

void InstaDrumsEditor::resized()
{
    auto area = getLocalBounds();
    float scale = (float) getHeight() / 600.0f;

    // Top bar
    int topBarH = std::max (28, (int) (36 * scale));
    auto topBar = area.removeFromTop (topBarH).reduced (8, 4);

    float titleSize = std::max (18.0f, 26.0f * scale);
    titleLabel.setFont (customLookAndFeel.getBoldFont (titleSize));
    titleLabel.setBounds (topBar.removeFromLeft ((int) (180 * scale)));

    float smallSize = std::max (10.0f, 13.0f * scale);
    versionLabel.setFont (juce::FontOptions (smallSize));
    versionLabel.setBounds (topBar.removeFromRight ((int) (50 * scale)));

    int btnW = std::max (60, (int) (90 * scale));
    loadFolderButton.setBounds (topBar.removeFromRight (btnW).reduced (2));
    loadKitButton.setBounds (topBar.removeFromRight ((int) (btnW * 0.8f)).reduced (2));
    saveKitButton.setBounds (topBar.removeFromRight ((int) (btnW * 0.8f)).reduced (2));
    loadSampleButton.setBounds (topBar.removeFromRight (btnW).reduced (2));

    // Bottom master bar
    int masterH = std::max (50, (int) (65 * scale));
    masterPanel.setBounds (area.removeFromBottom (masterH).reduced (4, 2));

    // Left panel: pad grid (~52% width)
    int rightPanelX = (int) (area.getWidth() * 0.52f);
    auto leftArea = area.removeFromLeft (rightPanelX).reduced (6);

    // Pads label
    int padsLabelH = (int) (20 * scale);
    padsLabel.setFont (juce::FontOptions (std::max (11.0f, 14.0f * scale), juce::Font::bold));
    auto padsHeader = leftArea.removeFromTop (padsLabelH);
    padsLabel.setBounds (padsHeader);

    // Pad grid
    int numPads = padComponents.size();
    if (numPads > 0)
    {
        int rows = (numPads + padColumns - 1) / padColumns;
        int padW = leftArea.getWidth() / padColumns;
        int padH = leftArea.getHeight() / rows;

        for (int i = 0; i < numPads; ++i)
        {
            int row = i / padColumns;
            int col = i % padColumns;
            padComponents[i]->setBounds (leftArea.getX() + col * padW,
                                          leftArea.getY() + row * padH,
                                          padW, padH);
        }
    }

    // Right panel: sample editor (top ~55%) + FX (bottom ~45%)
    auto rightArea = area.reduced (4);
    int editorHeight = (int) (rightArea.getHeight() * 0.55f);
    sampleEditor.setBounds (rightArea.removeFromTop (editorHeight).reduced (0, 2));
    fxPanel.setBounds (rightArea.reduced (0, 2));
}

void InstaDrumsEditor::timerCallback()
{
    for (auto* pc : padComponents)
        pc->repaint();

    // Sync FX panel knobs -> selected pad's FX params
    fxPanel.syncToPad();

    // Update per-pad compressor GR meter
    if (selectedPadIndex >= 0 && selectedPadIndex < processor.getNumPads())
        fxPanel.setCompGainReduction (processor.getPad (selectedPadIndex).compGainReduction.load());

    // Sync master panel -> processor
    processor.masterVolume.store        (masterPanel.getMasterVolume());
    processor.masterPan.store           (masterPanel.getMasterPan());
    processor.masterTune.store          (masterPanel.getMasterTune());
    processor.outputLimiterEnabled.store (masterPanel.isLimiterEnabled());

    // Update VU meter
    masterPanel.getVuMeter().setLevel (processor.vuLevelL.load(), processor.vuLevelR.load());
}
