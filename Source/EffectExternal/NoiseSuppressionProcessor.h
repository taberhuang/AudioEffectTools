#pragma once

#include <JuceHeader.h>
#include "RnNoiseEffectPlugin.h"

/**
 * NoiseSuppressionProcessor类
 * 封装RNNoise降噪算法，为JUCE应用程序提供噪声抑制功能
 */
class NoiseSuppressionProcessor
{
public:
    NoiseSuppressionProcessor();
    ~NoiseSuppressionProcessor();

    /**
     * 初始化处理器
     * @param sampleRate 采样率（必须为48000Hz）
     * @param channels 通道数
     */
    void prepareToPlay(double sampleRate, int channels);

    /**
     * 释放资源
     */
    void releaseResources();

    /**
     * 处理音频块
     * @param buffer 音频缓冲区
     */
    void processBlock(juce::AudioBuffer<float>& buffer) const;

    /**
     * 设置VAD阈值
     * @param threshold 阈值 (0.0-1.0)
     */
    void setVadThreshold(float threshold);

    /**
     * 设置VAD宽限期
     * @param periodMs 宽限期（毫秒）
     */
    void setVadGracePeriod(int periodMs);

    /**
     * 设置VAD追溯宽限期
     * @param periodMs 追溯宽限期（毫秒）
     */
    void setVadRetroactiveGracePeriod(int periodMs);

    /**
     * 获取当前VAD阈值
     */
    float getVadThreshold() const { return vadThreshold; }

    /**
     * 获取当前VAD宽限期
     */
    int getVadGracePeriod() const { return vadGracePeriod; }

    /**
     * 获取当前VAD追溯宽限期
     */
    int getVadRetroactiveGracePeriod() const { return vadRetroactiveGracePeriod; }

    /**
     * 获取统计信息
     */
    RnNoiseStats getStats() const;

    /**
     * 重置统计信息
     */
    void resetStats();

private:
    std::shared_ptr<RnNoiseEffectPlugin> rnNoisePlugin;

    float vadThreshold = 0.6f;        // VAD阈值
    int vadGracePeriod = 20;          // VAD宽限期（10ms为单位）
    int vadRetroactiveGracePeriod = 0; // VAD追溯宽限期

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoiseSuppressionProcessor);
};
