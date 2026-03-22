#include "PluginEditor.h"

InstaDrumsEditor::InstaDrumsEditor (InstaDrumsProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&customLookAndFeel);

    // Title
    titleLabel.setFont (juce::FontOptions (20.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, InstaDrumsLookAndFeel::accent);
    titleLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (titleLabel);

    versionLabel.setFont (juce::FontOptions (10.0f));
    versionLabel.setColour (juce::Label::textColourId, InstaDrumsLookAndFeel::textSecondary);
    versionLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (versionLabel);

    padsLabel.setFont (juce::FontOptions (10.0f, juce::Font::bold));
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
        sampleEditor.setPad (&processor.getPad (index));
}

void InstaDrumsEditor::paint (juce::Graphics& g)
{
    g.fillAll (InstaDrumsLookAndFeel::bgDark);

    // Subtle divider lines
    auto bounds = getLocalBounds();
    int rightPanelX = (int) (bounds.getWidth() * 0.52f);
    int bottomPanelY = bounds.getHeight() - 56;

    g.setColour (InstaDrumsLookAndFeel::bgLight.withAlpha (0.3f));
    g.drawVerticalLine (rightPanelX - 2, 30, (float) bottomPanelY);
    g.drawHorizontalLine (bottomPanelY - 1, 0, (float) bounds.getWidth());
}

void InstaDrumsEditor::resized()
{
    auto area = getLocalBounds();

    // Top bar (30px)
    auto topBar = area.removeFromTop (30).reduced (6, 4);
    titleLabel.setBounds (topBar.removeFromLeft (150));
    versionLabel.setBounds (topBar.removeFromRight (40));
    loadFolderButton.setBounds (topBar.removeFromRight (90).reduced (1));
    loadKitButton.setBounds (topBar.removeFromRight (70).reduced (1));
    saveKitButton.setBounds (topBar.removeFromRight (70).reduced (1));
    loadSampleButton.setBounds (topBar.removeFromRight (95).reduced (1));

    // Bottom master bar (52px)
    masterPanel.setBounds (area.removeFromBottom (52).reduced (4, 2));

    // Left panel: pad grid (~52% width)
    int rightPanelX = (int) (area.getWidth() * 0.52f);
    auto leftArea = area.removeFromLeft (rightPanelX).reduced (4);

    // Pads label
    auto padsHeader = leftArea.removeFromTop (16);
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

    // Update VU meter from processor output levels
    // (simplified: just repaint for now)
    masterPanel.getVuMeter().repaint();
}
