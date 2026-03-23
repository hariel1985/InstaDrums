cmake_minimum_required(VERSION 3.22)
project(InstaDrums VERSION 1.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../JUCE ${CMAKE_CURRENT_BINARY_DIR}/JUCE)

juce_add_plugin(InstaDrums
    COMPANY_NAME "InstaDrums"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    PLUGIN_MANUFACTURER_CODE Inst
    PLUGIN_CODE Idrm
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "InstaDrums"
    COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(InstaDrums)

juce_add_binary_data(InstaDrumsData SOURCES
    Resources/Rajdhani-Regular.ttf
    Resources/Rajdhani-Medium.ttf
    Resources/Rajdhani-Bold.ttf
)

target_sources(InstaDrums
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/DrumPad.cpp
        Source/PadComponent.cpp
        Source/LookAndFeel.cpp
        Source/WaveformDisplay.cpp
        Source/SampleEditorPanel.cpp
        Source/FxPanel.cpp
        Source/MasterPanel.cpp
)

target_compile_definitions(InstaDrums
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(InstaDrums
    PRIVATE
        InstaDrumsData
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_dsp
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)
