#include "PluginProcessor.h"
#include "PluginEditor.h"

MultibandHarmonizerAudioProcessor::MultibandHarmonizerAudioProcessor()
    : AudioProcessor(BusesProperties()
                        .withInput("Input", juce::AudioChannelSet::stereo(), true)
                        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    crossoverLowParam  = apvts.getRawParameterValue(ParamIDs::crossoverLow);
    crossoverHighParam = apvts.getRawParameterValue(ParamIDs::crossoverHigh);
    lowDriveParam      = apvts.getRawParameterValue(ParamIDs::lowDrive);
    midDriveParam      = apvts.getRawParameterValue(ParamIDs::midDrive);
    highDriveParam     = apvts.getRawParameterValue(ParamIDs::highDrive);
    lowGainParam       = apvts.getRawParameterValue(ParamIDs::lowGain);
    midGainParam       = apvts.getRawParameterValue(ParamIDs::midGain);
    highGainParam      = apvts.getRawParameterValue(ParamIDs::highGain);
    mixParam           = apvts.getRawParameterValue(ParamIDs::mix);
    outputGainParam    = apvts.getRawParameterValue(ParamIDs::outputGain);
}

MultibandHarmonizerAudioProcessor::~MultibandHarmonizerAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout MultibandHarmonizerAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamIDs::crossoverLow, "Low/Mid Crossover",
        juce::NormalisableRange<float>(20.0f, 2000.0f, 1.0f, 0.35f), 300.0f, "Hz"));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamIDs::crossoverHigh, "Mid/High Crossover",
        juce::NormalisableRange<float>(500.0f, 15000.0f, 1.0f, 0.35f), 3000.0f, "Hz"));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamIDs::lowDrive, "Low Drive", juce::NormalisableRange<float>(0.0f, 1.0f), 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamIDs::midDrive, "Mid Drive", juce::NormalisableRange<float>(0.0f, 1.0f), 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamIDs::highDrive, "High Drive", juce::NormalisableRange<float>(0.0f, 1.0f), 0.3f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamIDs::lowGain, "Low Gain", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f, "dB"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamIDs::midGain, "Mid Gain", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f, "dB"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamIDs::highGain, "High Gain", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f, "dB"));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamIDs::mix, "Dry/Wet Mix", juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamIDs::outputGain, "Output Gain", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f, "dB"));

    return { params.begin(), params.end() };
}

void MultibandHarmonizerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels = (juce::uint32) getTotalNumOutputChannels();

    splitter.prepare(spec);
    splitter.reset();

    const int numChannels = (int) spec.numChannels;
    lowBuffer.setSize(numChannels, samplesPerBlock);
    midBuffer.setSize(numChannels, samplesPerBlock);
    highBuffer.setSize(numChannels, samplesPerBlock);
    dryBuffer.setSize(numChannels, samplesPerBlock);
}

void MultibandHarmonizerAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MultibandHarmonizerAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto mono = juce::AudioChannelSet::mono();
    const auto stereo = juce::AudioChannelSet::stereo();

    if (layouts.getMainOutputChannelSet() != mono && layouts.getMainOutputChannelSet() != stereo)
        return false;

    return layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet();
}
#endif

void MultibandHarmonizerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    // setSize 在尺寸不变时是 no-op（不会重新分配内存），所以每个 block 都
    // 调用一次是安全且廉价的，能保证声道数变化（如从立体声切到单声道）时
    // 三个频段缓冲区始终和当前 buffer 尺寸一致。
    lowBuffer.setSize(numChannels, numSamples, false, false, true);
    midBuffer.setSize(numChannels, numSamples, false, false, true);
    highBuffer.setSize(numChannels, numSamples, false, false, true);
    dryBuffer.setSize(numChannels, numSamples, false, false, true);

    dryBuffer.makeCopyOf(buffer, true);

    splitter.setCrossoverFrequencies(crossoverLowParam->load(), crossoverHighParam->load());
    splitter.process(buffer, lowBuffer, midBuffer, highBuffer);

    lowShaper.setDrive(lowDriveParam->load());
    midShaper.setDrive(midDriveParam->load());
    highShaper.setDrive(highDriveParam->load());

    lowShaper.setMakeupGain(juce::Decibels::decibelsToGain(lowGainParam->load()));
    midShaper.setMakeupGain(juce::Decibels::decibelsToGain(midGainParam->load()));
    highShaper.setMakeupGain(juce::Decibels::decibelsToGain(highGainParam->load()));

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* lowData = lowBuffer.getWritePointer(ch);
        auto* midData = midBuffer.getWritePointer(ch);
        auto* highData = highBuffer.getWritePointer(ch);

        for (int i = 0; i < numSamples; ++i)
        {
            lowData[i] = lowShaper.processSample(lowData[i]);
            midData[i] = midShaper.processSample(midData[i]);
            highData[i] = highShaper.processSample(highData[i]);
        }
    }

    const float mix = mixParam->load();
    const float outGain = juce::Decibels::decibelsToGain(outputGainParam->load());

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* out = buffer.getWritePointer(ch);
        const auto* dry = dryBuffer.getReadPointer(ch);
        const auto* low = lowBuffer.getReadPointer(ch);
        const auto* mid = midBuffer.getReadPointer(ch);
        const auto* high = highBuffer.getReadPointer(ch);

        for (int i = 0; i < numSamples; ++i)
        {
            const float wet = low[i] + mid[i] + high[i];
            out[i] = (dry[i] * (1.0f - mix) + wet * mix) * outGain;
        }
    }
}

juce::AudioProcessorEditor* MultibandHarmonizerAudioProcessor::createEditor()
{
    return new MultibandHarmonizerAudioProcessorEditor(*this);
}

void MultibandHarmonizerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void MultibandHarmonizerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

// 每个插件格式的入口都要有这个工厂函数
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MultibandHarmonizerAudioProcessor();
}
