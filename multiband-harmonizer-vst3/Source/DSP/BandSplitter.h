#pragma once

#include <juce_dsp/juce_dsp.h>
#include <array>

/**
    ThreeBandSplitter
    ------------------
    把输入信号拆分成低/中/高三个频段。

    做法：
      - low  = 4 阶 Butterworth 低通 @ crossoverLow
      - high = 4 阶 Butterworth 高通 @ crossoverHigh
      - mid  = original - low - high   （保证三段严格重构回原始信号，
                                          不会因为滤波器相位问题产生能量损失）

    这是一个"原型"实现：用减法得到中频段，实现简单、相位安全，
    足够拿来做和声/激励效果的多频段处理。以后如果需要更陡峭的分频，
    可以把 lowPass/highPass 换成级联更多阶数的 IIR，或者换成
    Linkwitz-Riley 滤波器组。
*/
class ThreeBandSplitter
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        numChannels = spec.numChannels;

        for (auto* chain : { &lowChainL, &lowChainR, &highChainL, &highChainR })
            chain->prepare(spec);

        updateFilters(crossoverLowHz, crossoverHighHz);
    }

    void reset()
    {
        lowChainL.reset();
        lowChainR.reset();
        highChainL.reset();
        highChainR.reset();
    }

    void setCrossoverFrequencies(float lowHz, float highHz)
    {
        lowHz = juce::jlimit(20.0f, 20000.0f, lowHz);
        highHz = juce::jlimit(lowHz + 10.0f, 20000.0f, highHz);

        if (! juce::approximatelyEqual(lowHz, crossoverLowHz) ||
            ! juce::approximatelyEqual(highHz, crossoverHighHz))
        {
            updateFilters(lowHz, highHz);
        }
    }

    // buffer 是原始输入；low/mid/high 是三个输出 buffer，大小需与 buffer 相同。
    void process(const juce::AudioBuffer<float>& input,
                 juce::AudioBuffer<float>& low,
                 juce::AudioBuffer<float>& mid,
                 juce::AudioBuffer<float>& high)
    {
        low.makeCopyOf(input, true);
        high.makeCopyOf(input, true);

        {
            juce::dsp::AudioBlock<float> block(low);
            auto left = block.getSingleChannelBlock(0);
            juce::dsp::ProcessContextReplacing<float> ctxL(left);
            lowChainL.process(ctxL);

            if (low.getNumChannels() > 1)
            {
                auto right = block.getSingleChannelBlock(1);
                juce::dsp::ProcessContextReplacing<float> ctxR(right);
                lowChainR.process(ctxR);
            }
        }

        {
            juce::dsp::AudioBlock<float> block(high);
            auto left = block.getSingleChannelBlock(0);
            juce::dsp::ProcessContextReplacing<float> ctxL(left);
            highChainL.process(ctxL);

            if (high.getNumChannels() > 1)
            {
                auto right = block.getSingleChannelBlock(1);
                juce::dsp::ProcessContextReplacing<float> ctxR(right);
                highChainR.process(ctxR);
            }
        }

        mid.makeCopyOf(input, true);
        for (int ch = 0; ch < mid.getNumChannels(); ++ch)
        {
            auto* midData = mid.getWritePointer(ch);
            auto* lowData = low.getReadPointer(ch);
            auto* highData = high.getReadPointer(ch);

            for (int i = 0; i < mid.getNumSamples(); ++i)
                midData[i] = midData[i] - lowData[i] - highData[i];
        }
    }

private:
    using FilterChain = juce::dsp::ProcessorChain<juce::dsp::IIR::Filter<float>,
                                                   juce::dsp::IIR::Filter<float>>;

    void updateFilters(float lowHz, float highHz)
    {
        crossoverLowHz = lowHz;
        crossoverHighHz = highHz;

        if (sampleRate <= 0.0)
            return;

        // 4 阶 = 两级 2 阶 Butterworth 级联
        auto lowCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, crossoverLowHz);
        auto highCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, crossoverHighHz);

        *lowChainL.get<0>().coefficients = *lowCoeffs;
        *lowChainL.get<1>().coefficients = *lowCoeffs;
        *lowChainR.get<0>().coefficients = *lowCoeffs;
        *lowChainR.get<1>().coefficients = *lowCoeffs;

        *highChainL.get<0>().coefficients = *highCoeffs;
        *highChainL.get<1>().coefficients = *highCoeffs;
        *highChainR.get<0>().coefficients = *highCoeffs;
        *highChainR.get<1>().coefficients = *highCoeffs;
    }

    double sampleRate = 44100.0;
    int numChannels = 2;

    float crossoverLowHz = 300.0f;
    float crossoverHighHz = 3000.0f;

    FilterChain lowChainL, lowChainR;
    FilterChain highChainL, highChainR;
};
