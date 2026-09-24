#pragma once

#include <memory>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

/**
    简单直观的 GUI：三个频段各一列旋钮（Drive + Gain），
    顶部两个旋钮控制分频点，底部两个旋钮控制干湿比和输出电平。
*/
class MultibandHarmonizerAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit MultibandHarmonizerAudioProcessorEditor(MultibandHarmonizerAudioProcessor&);
    ~MultibandHarmonizerAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    struct KnobWithLabel
    {
        juce::Slider slider { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    void setupKnob(KnobWithLabel& knob, const juce::String& paramID, const juce::String& labelText);

    MultibandHarmonizerAudioProcessor& processorRef;

    juce::Label titleLabel;
    juce::Label lowBandLabel, midBandLabel, highBandLabel;

    KnobWithLabel crossoverLowKnob, crossoverHighKnob;
    KnobWithLabel lowDriveKnob, midDriveKnob, highDriveKnob;
    KnobWithLabel lowGainKnob, midGainKnob, highGainKnob;
    KnobWithLabel mixKnob, outputGainKnob;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultibandHarmonizerAudioProcessorEditor)
};
