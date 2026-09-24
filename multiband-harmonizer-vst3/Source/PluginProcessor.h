#pragma once

#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP/BandSplitter.h"
#include "DSP/HarmonicShaper.h"

namespace ParamIDs
{
    inline constexpr auto crossoverLow  = "crossoverLow";
    inline constexpr auto crossoverHigh = "crossoverHigh";

    inline constexpr auto lowDrive  = "lowDrive";
    inline constexpr auto midDrive  = "midDrive";
    inline constexpr auto highDrive = "highDrive";

    inline constexpr auto lowGain  = "lowGain";
    inline constexpr auto midGain  = "midGain";
    inline constexpr auto highGain = "highGain";

    inline constexpr auto mix        = "mix";
    inline constexpr auto outputGain = "outputGain";
}

/**
    MultibandHarmonizerAudioProcessor
    ----------------------------------
    把输入信号切分成低/中/高三个频段，每个频段单独做非线性谐波激励
    （HarmonicShaper），再按各自增益混合叠加成"湿"信号，最后用
    mix 参数与原始"干"信号交叉混合，从而制造出多频段的和声/厚度效果。
*/
class MultibandHarmonizerAudioProcessor : public juce::AudioProcessor
{
public:
    MultibandHarmonizerAudioProcessor();
    ~MultibandHarmonizerAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
   #endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts { *this, nullptr, "PARAMETERS", createParameterLayout() };

private:
    ThreeBandSplitter splitter;
    HarmonicShaper lowShaper, midShaper, highShaper;

    juce::AudioBuffer<float> lowBuffer, midBuffer, highBuffer, dryBuffer;

    std::atomic<float>* crossoverLowParam  = nullptr;
    std::atomic<float>* crossoverHighParam = nullptr;
    std::atomic<float>* lowDriveParam      = nullptr;
    std::atomic<float>* midDriveParam      = nullptr;
    std::atomic<float>* highDriveParam     = nullptr;
    std::atomic<float>* lowGainParam       = nullptr;
    std::atomic<float>* midGainParam       = nullptr;
    std::atomic<float>* highGainParam      = nullptr;
    std::atomic<float>* mixParam           = nullptr;
    std::atomic<float>* outputGainParam    = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultibandHarmonizerAudioProcessor)
};
