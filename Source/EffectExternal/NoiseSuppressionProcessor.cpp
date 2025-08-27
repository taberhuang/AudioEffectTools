#include "NoiseSuppressionProcessor.h"

NoiseSuppressionProcessor::NoiseSuppressionProcessor()
{
}

NoiseSuppressionProcessor::~NoiseSuppressionProcessor()
{
    releaseResources();
}

void NoiseSuppressionProcessor::prepareToPlay(double sampleRate, int channels)
{
    // RNNoise要求采样率为48000Hz
    if (sampleRate != 48000.0)
    {
        juce::Logger::writeToLog("Warning: RNNoise requires 48000Hz sample rate, current: " +
                                juce::String(sampleRate));
    }

    rnNoisePlugin = std::make_shared<RnNoiseEffectPlugin>(static_cast<uint32_t>(channels));
    rnNoisePlugin->init();
}

void NoiseSuppressionProcessor::releaseResources()
{
    if (rnNoisePlugin)
    {
        rnNoisePlugin->deinit();
        rnNoisePlugin.reset();
    }
}

void NoiseSuppressionProcessor::processBlock(juce::AudioBuffer<float>& buffer) const
{
    if (!rnNoisePlugin)
        return;

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // 准备输入输出指针数组
    const float* in[8] = { nullptr };
    float* out[8] = { nullptr };

    for (int channel = 0; channel < numChannels && channel < 8; ++channel)
    {
        in[channel] = buffer.getReadPointer(channel);
        out[channel] = buffer.getWritePointer(channel);
    }

    // 调用RNNoise处理
    rnNoisePlugin->process(in, out, static_cast<size_t>(numSamples),
                          vadThreshold,
                          static_cast<uint32_t>(vadGracePeriod),
                          static_cast<uint32_t>(vadRetroactiveGracePeriod));
}

void NoiseSuppressionProcessor::setVadThreshold(float threshold)
{
    vadThreshold = juce::jlimit(0.0f, 1.0f, threshold);
}

void NoiseSuppressionProcessor::setVadGracePeriod(int periodMs)
{
    vadGracePeriod = juce::jmax(0, periodMs);
}

void NoiseSuppressionProcessor::setVadRetroactiveGracePeriod(int periodMs)
{
    vadRetroactiveGracePeriod = juce::jmax(0, periodMs);
}

RnNoiseStats NoiseSuppressionProcessor::getStats() const
{
    if (rnNoisePlugin)
    {
        return rnNoisePlugin->getStats();
    }
    return RnNoiseStats{};
}

void NoiseSuppressionProcessor::resetStats()
{
    if (rnNoisePlugin)
    {
        rnNoisePlugin->resetStats();
    }
}
