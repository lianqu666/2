#pragma once

#include <cmath>

/**
    HarmonicShaper
    --------------
    每个频段独立使用的非线性波形整形器，用来"制造和声"：
    对信号做非对称软削波，产生偶次/奇次谐波（overtone），
    再和干音混合后听感上会显得更"厚"、更有和声感。

    drive:  0..1，越大谐波越丰富（越"脏"）
    gain:   谐波信号的输出电平补偿
*/
class HarmonicShaper
{
public:
    void setDrive(float newDrive) noexcept { drive = newDrive; }
    void setMakeupGain(float newGain) noexcept { makeupGain = newGain; }

    float processSample(float x) const noexcept
    {
        // drive 越大，输入被放大得越多，进入饱和区，从而产生更多谐波
        const float driven = x * (1.0f + drive * 9.0f);

        // 非对称软削波：tanh 加一点偏置，产生奇偶次谐波混合，听起来更像"和声"
        const float shaped = std::tanh(driven) - 0.15f * drive * std::tanh(driven * driven);

        return shaped * makeupGain;
    }

private:
    float drive = 0.0f;
    float makeupGain = 1.0f;
};
