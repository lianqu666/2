cmake_minimum_required(VERSION 3.22)

project(MultibandHarmonizer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --------------------------------------------------------------------------
# JUCE：优先使用本地 vendored 副本（放在 ./JUCE 目录），否则通过
# FetchContent 自动从 GitHub 拉取。第一次 configure 需要联网，之后会缓存
# 在 build/_deps 里，不用每次都下载。
# --------------------------------------------------------------------------
if(EXISTS "${CMAKE_SOURCE_DIR}/JUCE/CMakeLists.txt")
    message(STATUS "Using local JUCE checkout at ${CMAKE_SOURCE_DIR}/JUCE")
    add_subdirectory(JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 7.0.9
        GIT_SHALLOW TRUE
    )
    message(STATUS "Fetching JUCE 7.0.9 from GitHub (first configure may take a while)...")
    FetchContent_MakeAvailable(JUCE)
endif()

# --------------------------------------------------------------------------
# 插件目标
# --------------------------------------------------------------------------
juce_add_plugin(MultibandHarmonizer
    COMPANY_NAME                "YourCompany"
    PLUGIN_MANUFACTURER_CODE    Yrcp
    PLUGIN_CODE                 Mbh1
    FORMATS                     VST3 Standalone
    PRODUCT_NAME                "Multiband Harmonizer"
    VST3_CATEGORIES             Fx Distortion
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     TRUE
)

target_sources(MultibandHarmonizer
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

target_include_directories(MultibandHarmonizer
    PRIVATE
        ${CMAKE_SOURCE_DIR}/Source
)

target_compile_definitions(MultibandHarmonizer
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(MultibandHarmonizer
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

juce_generate_juce_header(MultibandHarmonizer)
