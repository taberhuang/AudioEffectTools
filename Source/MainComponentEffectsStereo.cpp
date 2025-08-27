#include "MainComponent.h"

// Stereo processing functions for JUCE effects

void MainComponent::processJUCEDelayStereo(float leftInput, float rightInput, 
                                          float& leftOutput, float& rightOutput)
{
    // Create stereo delay buffers
    static std::vector<float> delayBufferL(static_cast<size_t>(currentSampleRate * 2.0)); // 2 second max delay
    static std::vector<float> delayBufferR(static_cast<size_t>(currentSampleRate * 2.0));
    static size_t writeIndex = 0;
    
    if (delayBufferL.empty() || delayBufferR.empty()) {
        leftOutput = leftInput;
        rightOutput = rightInput;
        return;
    }
    
    // Calculate delay in samples
    size_t delaySamples = static_cast<size_t>(juceDelayTime * currentSampleRate / 1000.0);
    delaySamples = std::min(delaySamples, delayBufferL.size() - 1);
    
    // Calculate read index
    size_t readIndex = (writeIndex + delayBufferL.size() - delaySamples) % delayBufferL.size();
    
    // Get delayed samples
    float delayedLeft = delayBufferL[readIndex];
    float delayedRight = delayBufferR[readIndex];
    
    // Apply feedback
    float feedbackLeft = leftInput + delayedLeft * juceDelayFeedback;
    float feedbackRight = rightInput + delayedRight * juceDelayFeedback;
    
    // Store in delay buffers
    delayBufferL[writeIndex] = feedbackLeft;
    delayBufferR[writeIndex] = feedbackRight;
    writeIndex = (writeIndex + 1) % delayBufferL.size();
    
    // Mix dry and wet signals
    leftOutput = leftInput * (1.0f - juceDelayMix) + delayedLeft * juceDelayMix;
    rightOutput = rightInput * (1.0f - juceDelayMix) + delayedRight * juceDelayMix;
}

void MainComponent::processJUCEFlangerStereo(float leftInput, float rightInput, 
                                            float& leftOutput, float& rightOutput)
{
    // Validate inputs
    if (!std::isfinite(leftInput)) leftInput = 0.0f;
    if (!std::isfinite(rightInput)) rightInput = 0.0f;
    
    // Create stereo audio buffer for JUCE DSP processing
    static juce::AudioBuffer<float> buffer(2, 1);
    buffer.clear();
    buffer.setSample(0, 0, leftInput);
    buffer.setSample(1, 0, rightInput);
    
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    juceFlanger.process(context);
    
    // Get outputs and validate
    leftOutput = buffer.getSample(0, 0);
    rightOutput = buffer.getSample(1, 0);
    
    if (!std::isfinite(leftOutput)) leftOutput = 0.0f;
    if (!std::isfinite(rightOutput)) rightOutput = 0.0f;
}

void MainComponent::processJUCEChorusStereo(float leftInput, float rightInput, 
                                           float& leftOutput, float& rightOutput)
{
    // Validate inputs
    if (!std::isfinite(leftInput)) leftInput = 0.0f;
    if (!std::isfinite(rightInput)) rightInput = 0.0f;
    
    // Create stereo audio buffer for JUCE DSP processing
    static juce::AudioBuffer<float> buffer(2, 1);
    buffer.clear();
    buffer.setSample(0, 0, leftInput);
    buffer.setSample(1, 0, rightInput);
    
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    juceChorus.process(context);
    
    // Get outputs and validate
    leftOutput = buffer.getSample(0, 0);
    rightOutput = buffer.getSample(1, 0);
    
    if (!std::isfinite(leftOutput)) leftOutput = 0.0f;
    if (!std::isfinite(rightOutput)) rightOutput = 0.0f;
}

void MainComponent::processJUCEPlateReverbStereo(float leftInput, float rightInput, 
                                                float& leftOutput, float& rightOutput)
{
    // Validate inputs
    if (!std::isfinite(leftInput)) leftInput = 0.0f;
    if (!std::isfinite(rightInput)) rightInput = 0.0f;
    
    // Create stereo audio buffer for JUCE DSP processing
    static juce::AudioBuffer<float> buffer(2, 1);
    buffer.clear();
    buffer.setSample(0, 0, leftInput);
    buffer.setSample(1, 0, rightInput);
    
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    jucePlateReverb.process(context);
    
    // Get outputs and validate
    leftOutput = buffer.getSample(0, 0);
    rightOutput = buffer.getSample(1, 0);
    
    if (!std::isfinite(leftOutput)) leftOutput = 0.0f;
    if (!std::isfinite(rightOutput)) rightOutput = 0.0f;
}

void MainComponent::processJUCEReverbStereo(float leftInput, float rightInput, 
                                           float& leftOutput, float& rightOutput)
{
    // Validate inputs
    if (!std::isfinite(leftInput)) leftInput = 0.0f;
    if (!std::isfinite(rightInput)) rightInput = 0.0f;
    
    // Create stereo audio buffer for JUCE DSP processing
    static juce::AudioBuffer<float> buffer(2, 1);
    buffer.clear();
    buffer.setSample(0, 0, leftInput);
    buffer.setSample(1, 0, rightInput);
    
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    juceReverb.process(context);
    
    // Get outputs and validate
    leftOutput = buffer.getSample(0, 0);
    rightOutput = buffer.getSample(1, 0);
    
    if (!std::isfinite(leftOutput)) leftOutput = 0.0f;
    if (!std::isfinite(rightOutput)) rightOutput = 0.0f;
}

void MainComponent::processJUCEPhaserStereo(float leftInput, float rightInput, 
                                           float& leftOutput, float& rightOutput)
{
    // Validate inputs
    if (!std::isfinite(leftInput)) leftInput = 0.0f;
    if (!std::isfinite(rightInput)) rightInput = 0.0f;
    
    // Create stereo audio buffer for JUCE DSP processing
    static juce::AudioBuffer<float> buffer(2, 1);
    buffer.clear();
    buffer.setSample(0, 0, leftInput);
    buffer.setSample(1, 0, rightInput);
    
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    jucePhaser.process(context);
    
    // Get outputs and validate
    leftOutput = buffer.getSample(0, 0);
    rightOutput = buffer.getSample(1, 0);
    
    if (!std::isfinite(leftOutput)) leftOutput = 0.0f;
    if (!std::isfinite(rightOutput)) rightOutput = 0.0f;
}

void MainComponent::processJUCECompressorStereo(float leftInput, float rightInput, 
                                               float& leftOutput, float& rightOutput)
{
    // Validate inputs
    if (!std::isfinite(leftInput)) leftInput = 0.0f;
    if (!std::isfinite(rightInput)) rightInput = 0.0f;
    
    // Create stereo audio buffer for JUCE DSP processing
    static juce::AudioBuffer<float> buffer(2, 1);
    buffer.clear();
    buffer.setSample(0, 0, leftInput);
    buffer.setSample(1, 0, rightInput);
    
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    juceCompressor.process(context);
    
    // Get outputs and validate
    leftOutput = buffer.getSample(0, 0);
    rightOutput = buffer.getSample(1, 0);
    
    if (!std::isfinite(leftOutput)) leftOutput = 0.0f;
    if (!std::isfinite(rightOutput)) rightOutput = 0.0f;
}

void MainComponent::processJUCEPitchShiftStereo(float leftInput, float rightInput, 
                                               float& leftOutput, float& rightOutput)
{
    // Create separate pitch shift buffers for left and right channels
    static std::vector<float> pitchBufferL(static_cast<size_t>(currentSampleRate * 0.1)); // 100ms buffer
    static std::vector<float> pitchBufferR(static_cast<size_t>(currentSampleRate * 0.1));
    static size_t writeIndex = 0;
    
    if (pitchBufferL.empty() || pitchBufferR.empty()) {
        leftOutput = leftInput;
        rightOutput = rightInput;
        return;
    }
    
    // Calculate pitch shift ratio
    float pitchRatio = std::pow(2.0f, jucePitchShift / 12.0f);
    
    // Store inputs in buffers
    pitchBufferL[writeIndex] = leftInput;
    pitchBufferR[writeIndex] = rightInput;
    writeIndex = (writeIndex + 1) % pitchBufferL.size();
    
    // Calculate read position with pitch shift
    float readPos = writeIndex - (pitchBufferL.size() / 2.0f) * pitchRatio;
    if (readPos < 0) readPos += pitchBufferL.size();
    
    // Linear interpolation for fractional delay
    size_t readIndex1 = static_cast<size_t>(readPos) % pitchBufferL.size();
    size_t readIndex2 = (readIndex1 + 1) % pitchBufferL.size();
    float fraction = readPos - std::floor(readPos);
    
    // Process left channel
    float sampleL1 = pitchBufferL[readIndex1];
    float sampleL2 = pitchBufferL[readIndex2];
    leftOutput = sampleL1 + fraction * (sampleL2 - sampleL1);
    
    // Process right channel
    float sampleR1 = pitchBufferR[readIndex1];
    float sampleR2 = pitchBufferR[readIndex2];
    rightOutput = sampleR1 + fraction * (sampleR2 - sampleR1);
}

void MainComponent::processJUCEDistortionStereo(float leftInput, float rightInput, 
                                               float& leftOutput, float& rightOutput)
{
    // Validate inputs
    if (!std::isfinite(leftInput)) leftInput = 0.0f;
    if (!std::isfinite(rightInput)) rightInput = 0.0f;
    
    // Create stereo audio buffer for JUCE DSP processing
    static juce::AudioBuffer<float> buffer(2, 1);
    buffer.clear();
    buffer.setSample(0, 0, leftInput);
    buffer.setSample(1, 0, rightInput);
    
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    juceDistortion.process(context);
    
    // Get outputs and validate
    leftOutput = buffer.getSample(0, 0);
    rightOutput = buffer.getSample(1, 0);
    
    if (!std::isfinite(leftOutput)) leftOutput = 0.0f;
    if (!std::isfinite(rightOutput)) rightOutput = 0.0f;
}