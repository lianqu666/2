#include "PluginEditor.h"

namespace
{
    constexpr int knobSize = 80;
    constexpr int columnWidth = 110;
}

MultibandHarmonizerAudioProcessorEditor::MultibandHarmonizerAudioProcessorEditor(MultibandHarmonizerAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    titleLabel.setText("Multiband Harmonizer", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    lowBandLabel.setText("LOW", juce::dontSendNotification);
    midBandLabel.setText("MID", juce::dontSendNotification);
    highBandLabel.setText("HIGH", juce::dontSendNotification);
    for (auto* l : { &lowBandLabel, &midBandLabel, &highBandLabel })
    {
        l->setJustificationType(juce::Justification::centred);
        l->setFont(juce::Font(15.0f, juce::Font::bold));
        addAndMakeVisible(l);
    }

    setupKnob(crossoverLowKnob, ParamIDs::crossoverLow, "Low/Mid Xover");
    setupKnob(crossoverHighKnob, ParamIDs::crossoverHigh, "Mid/High Xover");

    setupKnob(lowDriveKnob, ParamIDs::lowDrive, "Drive");
    setupKnob(midDriveKnob, ParamIDs::midDrive, "Drive");
    setupKnob(highDriveKnob, ParamIDs::highDrive, "Drive");

    setupKnob(lowGainKnob, ParamIDs::lowGain, "Gain");
    setupKnob(midGainKnob, ParamIDs::midGain, "Gain");
    setupKnob(highGainKnob, ParamIDs::highGain, "Gain");

    setupKnob(mixKnob, ParamIDs::mix, "Dry/Wet");
    setupKnob(outputGainKnob, ParamIDs::outputGain, "Output");

    setSize(4 * columnWidth + 40, 420);
}

MultibandHarmonizerAudioProcessorEditor::~MultibandHarmonizerAudioProcessorEditor() = default;

void MultibandHarmonizerAudioProcessorEditor::setupKnob(KnobWithLabel& knob, const juce::String& paramID, const juce::String& labelText)
{
    knob.slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 18);
    addAndMakeVisible(knob.slider);

    knob.label.setText(labelText, juce::dontSendNotification);
    knob.label.setJustificationType(juce::Justification::centred);
    knob.label.setFont(juce::Font(13.0f));
    addAndMakeVisible(knob.label);

    knob.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.apvts, paramID, knob.slider);
}

void MultibandHarmonizerAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2b2b2b));

    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.drawLine(10.0f, 100.0f, (float) getWidth() - 10.0f, 100.0f, 1.0f);
    g.drawLine(10.0f, 300.0f, (float) getWidth() - 10.0f, 300.0f, 1.0f);
}

void MultibandHarmonizerAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(10);

    titleLabel.setBounds(bounds.removeFromTop(30));

    // 顶部：两个分频点旋钮
    auto crossoverRow = bounds.removeFromTop(90);
    auto half = crossoverRow.getWidth() / 2;
    crossoverLowKnob.slider.setBounds(crossoverRow.removeFromLeft(half).withSizeKeepingCentre(knobSize, knobSize + 20));
    crossoverHighKnob.slider.setBounds(crossoverRow.withSizeKeepingCentre(knobSize, knobSize + 20));

    crossoverLowKnob.label.setBounds(crossoverLowKnob.slider.getBounds().translated(0, -14).withHeight(14));
    crossoverHighKnob.label.setBounds(crossoverHighKnob.slider.getBounds().translated(0, -14).withHeight(14));

    bounds.removeFromTop(10);

    // 中部：三个频段各一列（band 标签 + drive + gain）
    auto bandsArea = bounds.removeFromTop(190);
    auto colWidth = bandsArea.getWidth() / 3;

    auto layoutBand = [&](juce::Rectangle<int> col, juce::Label& bandLabel, KnobWithLabel& drive, KnobWithLabel& gain)
    {
        bandLabel.setBounds(col.removeFromTop(20));
        auto driveArea = col.removeFromTop(90);
        drive.slider.setBounds(driveArea.withSizeKeepingCentre(knobSize, knobSize));
        drive.label.setBounds(drive.slider.getBounds().translated(0, knobSize - 10).withHeight(14));

        auto gainArea = col.removeFromTop(90);
        gain.slider.setBounds(gainArea.withSizeKeepingCentre(knobSize, knobSize));
        gain.label.setBounds(gain.slider.getBounds().translated(0, knobSize - 10).withHeight(14));
    };

    layoutBand(bandsArea.removeFromLeft(colWidth), lowBandLabel, lowDriveKnob, lowGainKnob);
    layoutBand(bandsArea.removeFromLeft(colWidth), midBandLabel, midDriveKnob, midGainKnob);
    layoutBand(bandsArea, highBandLabel, highDriveKnob, highGainKnob);

    bounds.removeFromTop(10);

    // 底部：Mix + Output
    auto bottomRow = bounds.removeFromTop(110);
    auto bottomHalf = bottomRow.getWidth() / 2;
    mixKnob.slider.setBounds(bottomRow.removeFromLeft(bottomHalf).withSizeKeepingCentre(knobSize, knobSize));
    outputGainKnob.slider.setBounds(bottomRow.withSizeKeepingCentre(knobSize, knobSize));

    mixKnob.label.setBounds(mixKnob.slider.getBounds().translated(0, knobSize - 10).withHeight(14));
    outputGainKnob.label.setBounds(outputGainKnob.slider.getBounds().translated(0, knobSize - 10).withHeight(14));
}
