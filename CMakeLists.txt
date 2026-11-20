cmake_minimum_required(VERSION 3.24)
project(LibertyLOVE_EQ VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.10 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(LoveEQ
    COMPANY_NAME "Liberty Music Software"
    BUNDLE_ID com.libertymusicsoftware.loveeq
    PLUGIN_MANUFACTURER_CODE LbMs
    PLUGIN_CODE LoEQ
    PRODUCT_NAME "LOVE EQ"
    FORMATS AU VST3 Standalone
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Fx EQ)

juce_generate_juce_header(LoveEQ)
target_sources(LoveEQ PRIVATE
    Source/EqCore.h
    Source/CurveLab.h
    Source/HarmonicDNA.h
    Source/RelationshipEQ.h
    Source/TimeLensCore.h
    Source/FactoryPresets.h
    Source/PresetBrowser.h
    Source/UserPresetLibrary.h
    Source/PluginProcessor.h
    Source/PluginProcessor.cpp
    Source/PluginEditor.h
    Source/PluginEditor.cpp)
target_compile_definitions(LoveEQ PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(LoveEQ PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

enable_testing()
add_executable(loveeq_core_test tests/eqcore_test.cpp)
target_compile_features(loveeq_core_test PRIVATE cxx_std_17)
add_test(NAME loveeq_core COMMAND loveeq_core_test)
add_executable(loveeq_presets_test tests/presets_test.cpp)
target_compile_features(loveeq_presets_test PRIVATE cxx_std_17)
add_test(NAME loveeq_presets COMMAND loveeq_presets_test)

add_executable(loveeq_graphic_test tests/graphic_test.cpp)
target_include_directories(loveeq_graphic_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source)
target_compile_features(loveeq_graphic_test PRIVATE cxx_std_17)
add_test(NAME loveeq_graphic COMMAND loveeq_graphic_test)

add_executable(loveeq_utf8_test tests/utf8_test.cpp)
target_compile_features(loveeq_utf8_test PRIVATE cxx_std_17)
add_test(NAME loveeq_utf8 COMMAND loveeq_utf8_test)

# Independent musical sketch / peak detection tests.
add_executable(loveeq_curve_lab_test tests/curvelab_test.cpp)
target_compile_features(loveeq_curve_lab_test PRIVATE cxx_std_17)
target_include_directories(loveeq_curve_lab_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source)
add_test(NAME loveeq_curve_lab COMMAND loveeq_curve_lab_test)

# Pure C++ monophonic harmonic analysis tests, independent of JUCE.
add_executable(loveeq_harmonic_dna_test tests/harmonic_dna_test.cpp)
target_compile_features(loveeq_harmonic_dna_test PRIVATE cxx_std_17)
target_include_directories(loveeq_harmonic_dna_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source)
add_test(NAME loveeq_harmonic_dna COMMAND loveeq_harmonic_dna_test)

# Multi-instance real-time spectrum registry, harmonic guard and release logic.
add_executable(loveeq_relationship_test tests/relationship_test.cpp)
target_compile_features(loveeq_relationship_test PRIVATE cxx_std_17)
target_include_directories(loveeq_relationship_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source)
add_test(NAME loveeq_relationship COMMAND loveeq_relationship_test)

# Attack / Body / Tail envelope segmentation (JUCE-free, real-time tests).
add_executable(loveeq_time_lens_test tests/time_lens_test.cpp)
target_compile_features(loveeq_time_lens_test PRIVATE cxx_std_17)
target_include_directories(loveeq_time_lens_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source)
add_test(NAME loveeq_time_lens COMMAND loveeq_time_lens_test)
