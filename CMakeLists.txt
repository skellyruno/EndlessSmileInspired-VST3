cmake_minimum_required(VERSION 3.22)
project(EndlessSmileInspired VERSION 1.4.0)
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)
juce_add_plugin(EndlessSmileInspired
    COMPANY_NAME "Skelly Audio"
    BUNDLE_ID "audio.skelly.endlesssmileinspired"
    PLUGIN_MANUFACTURER_CODE Skly
    PLUGIN_CODE EsI1
    FORMATS VST3
    PRODUCT_NAME "Skell Smile"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)
juce_add_binary_data(EndlessSmileAssets SOURCES Assets/dial_well.png Assets/knob_cap_big.png)
target_sources(EndlessSmileInspired PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

# Generate JuceHeader.h for source files that include <JuceHeader.h>.
juce_generate_juce_header(EndlessSmileInspired)
target_compile_features(EndlessSmileInspired PRIVATE cxx_std_17)
target_compile_definitions(EndlessSmileInspired PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(EndlessSmileInspired PRIVATE
    EndlessSmileAssets juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
