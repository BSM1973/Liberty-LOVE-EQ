cmake_minimum_required(VERSION 3.24)
project(LibertyLOVE_EQ VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MSVC otherwise interprets u8"..." through the machine's legacy code page.
# Force C++17 source and execution character sets to UTF-8 for every target.
if(MSVC)
  add_compile_options(/utf-8)
endif()


include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.10 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

# Keep the existing desktop formats unchanged. iOS uses an app containing
# the AUv3 extension, built with the Xcode generator.
set(loveeq_formats AU VST3 Standalone)
set(loveeq_mobile_options "")
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(loveeq_formats AUv3 Standalone)
    set(CMAKE_XCODE_GENERATE_SCHEME ON)
    set(LOVE_EQ_IOS_BUILD_VERSION "1" CACHE STRING "Unique iOS build number")
    find_program(LOVE_EQ_HOST_PYTHON NAMES python3 REQUIRED NO_CMAKE_FIND_ROOT_PATH)
    set(loveeq_ios_icon "${CMAKE_CURRENT_BINARY_DIR}/LOVE-EQ-AppIcon.png")
    execute_process(COMMAND "${LOVE_EQ_HOST_PYTHON}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_ios_icon.py" "${loveeq_ios_icon}" COMMAND_ERROR_IS_FATAL ANY)
    set(loveeq_mobile_options
        VERSION 0.5.0
        BUILD_VERSION "${LOVE_EQ_IOS_BUILD_VERSION}"
        ICON_BIG "${loveeq_ios_icon}"
        TARGETED_DEVICE_FAMILY "1,2"
        IPHONE_SCREEN_ORIENTATIONS UIInterfaceOrientationPortrait UIInterfaceOrientationLandscapeLeft UIInterfaceOrientationLandscapeRight
        IPAD_SCREEN_ORIENTATIONS UIInterfaceOrientationPortrait UIInterfaceOrientationPortraitUpsideDown UIInterfaceOrientationLandscapeLeft UIInterfaceOrientationLandscapeRight
        MICROPHONE_PERMISSION_ENABLED TRUE
        MICROPHONE_PERMISSION_TEXT "LOVE EQ uses audio input for standalone equalization.")
endif()

juce_add_plugin(LoveEQ
    COMPANY_NAME "Liberty Music Software"
    BUNDLE_ID com.libertymusicsoftware.loveeq
    PLUGIN_MANUFACTURER_CODE LbMs
    PLUGIN_CODE LoEQ
    PRODUCT_NAME "LOVE EQ"
    FORMATS ${loveeq_formats}
    ${loveeq_mobile_options}
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Fx EQ)

if(CMAKE_SYSTEM_NAME STREQUAL "iOS" AND DEFINED LOVE_EQ_APP_PROFILE AND DEFINED LOVE_EQ_AUV3_PROFILE)
    set_target_properties(LoveEQ_Standalone PROPERTIES
        XCODE_ATTRIBUTE_CODE_SIGN_STYLE Manual
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Apple Distribution"
        XCODE_ATTRIBUTE_PROVISIONING_PROFILE_SPECIFIER "${LOVE_EQ_APP_PROFILE}")
    set_target_properties(LoveEQ_AUv3 PROPERTIES
        XCODE_ATTRIBUTE_CODE_SIGN_STYLE Manual
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Apple Distribution"
        XCODE_ATTRIBUTE_PROVISIONING_PROFILE_SPECIFIER "${LOVE_EQ_AUV3_PROFILE}")
endif()

juce_generate_juce_header(LoveEQ)
target_sources(LoveEQ PRIVATE
    Source/MobilePrecision.h
    Source/EqCore.h
    Source/CurveLab.h
    Source/HarmonicDNA.h
    Source/RelationshipEQ.h
    Source/TimeLensCore.h
    Source/IntentCore.h
    Source/MatchCore.h
    Source/FactoryPresets.h
    Source/PresetBrowser.h
    Source/Utf8Text.h
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

if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
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

# LOVE INTENT: measured, deterministic, safe bounded musical suggestions.
add_executable(loveeq_intent_test tests/intent_test.cpp)
target_compile_features(loveeq_intent_test PRIVATE cxx_std_17)
target_include_directories(loveeq_intent_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source)
add_test(NAME loveeq_intent COMMAND loveeq_intent_test)

# EQ MATCH spectral capture, loudness-independent comparison and safe bounds.
add_executable(loveeq_match_test tests/match_test.cpp)
target_compile_features(loveeq_match_test PRIVATE cxx_std_17)
target_include_directories(loveeq_match_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source)
add_test(NAME loveeq_match COMMAND loveeq_match_test)

# Unlike the independent byte-level test, this test runs the real JUCE String
# conversion. A success here proves the labels won't become PARAMÃ‰TRIQUE.
add_executable(loveeq_juce_utf8_test tests/juce_utf8_runtime_test.cpp)
target_include_directories(loveeq_juce_utf8_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source)
target_compile_features(loveeq_juce_utf8_test PRIVATE cxx_std_17)
target_compile_definitions(loveeq_juce_utf8_test PRIVATE JUCE_USE_CURL=0)
target_link_libraries(loveeq_juce_utf8_test PRIVATE
    juce::juce_core
    juce::juce_recommended_config_flags)
add_test(NAME loveeq_juce_utf8 COMMAND loveeq_juce_utf8_test)

add_executable(loveeq_mobile_precision_test tests/mobile_precision_test.cpp)
target_compile_features(loveeq_mobile_precision_test PRIVATE cxx_std_17)
target_include_directories(loveeq_mobile_precision_test PRIVATE Source)
add_test(NAME loveeq_mobile_precision COMMAND loveeq_mobile_precision_test)

# Opt-in processor integration diagnostic, independent of plug-in wrapper.
option(LOVE_EQ_PROCESSOR_QC "Build processor integration diagnostics" OFF)
if(LOVE_EQ_PROCESSOR_QC)
    add_executable(loveeq_processor_qc tests/mobile_processor_qc.cpp)
    target_include_directories(loveeq_processor_qc PRIVATE Source
        ${CMAKE_CURRENT_BINARY_DIR}/LoveEQ_artefacts/JuceLibraryCode)
    target_compile_definitions(loveeq_processor_qc PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
    target_link_libraries(loveeq_processor_qc PRIVATE LoveEQ juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags)
    add_test(NAME loveeq_processor_qc COMMAND loveeq_processor_qc)
    set_tests_properties(loveeq_processor_qc PROPERTIES TIMEOUT 180)
endif()
endif() # desktop tests; iOS execution requires a device or simulator
