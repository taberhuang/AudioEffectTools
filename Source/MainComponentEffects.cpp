#include "MainComponent.h"

void MainComponent::processEffectStereo(float leftInput, float rightInput, 
                                       float& leftOutput, float& rightOutput)
{
    // Check if effect is enabled
    if (!isEffectEnabled)
    {
        // Effect is disabled, bypass processing
        leftOutput = leftInput;
        rightOutput = rightInput;
        return;
    }
    
    switch (currentEffect)
    {
        case Delay:
            {
                FxDelayProcess(leftInput, rightInput, &leftOutput, &rightOutput);
            }
            break;
            
        case Flanger:
            {
                //FxFlangerProcess(leftInput, rightInput, &leftOutput, &rightOutput);
                leftOutput = leftInput;
                rightOutput = rightInput;
            }
            break;
            
        case PlateReverb:
            {
                //FxPlateRevbProcess(leftInput, rightInput, &leftOutput, &rightOutput);
                leftOutput = leftInput;
                rightOutput = rightInput;
            }
            break;
            
        case PitchShift:
            {
                //FxPitchProcess(leftInput, rightInput, &leftOutput, &rightOutput);
                leftOutput = leftInput;
                rightOutput = rightInput;
            }
            break;
            
        case CathedralReverb:
            {
                FxCathedralRevbProcess(leftInput, rightInput, &leftOutput, &rightOutput);
            }
            break;
            
        // Handle JUCE effects with proper stereo processing
        case JUCEDelay:
            {
                processJUCEDelayStereo(leftInput, rightInput, leftOutput, rightOutput);
            }
            break;
            
        case JUCEFlanger:
            {
                processJUCEFlangerStereo(leftInput, rightInput, leftOutput, rightOutput);
            }
            break;
            
        case JUCEChorus:
            {
                processJUCEChorusStereo(leftInput, rightInput, leftOutput, rightOutput);
            }
            break;
            
        case JUCEPlateReverb:
            {
                processJUCEPlateReverbStereo(leftInput, rightInput, leftOutput, rightOutput);
            }
            break;
            
        case JUCEReverb:
            {
                processJUCEReverbStereo(leftInput, rightInput, leftOutput, rightOutput);
            }
            break;
            
        case JUCEPhaser:
            {
                processJUCEPhaserStereo(leftInput, rightInput, leftOutput, rightOutput);
            }
            break;
            
        case JUCECompressor:
            {
                processJUCECompressorStereo(leftInput, rightInput, leftOutput, rightOutput);
            }
            break;
            
        case JUCEPitchShift:
            {
                processJUCEPitchShiftStereo(leftInput, rightInput, leftOutput, rightOutput);
            }
            break;
            
        case JUCEDistortion:
            {
                processJUCEDistortionStereo(leftInput, rightInput, leftOutput, rightOutput);
            }
            break;

        case NoiseSuppression:
            {
                if (noiseSuppressionProcessor)
                {
                    // 创建临时的音频缓冲区来处理立体声
                    juce::AudioBuffer<float> tempBuffer(2, 1);
                    tempBuffer.setSample(0, 0, leftInput);
                    tempBuffer.setSample(1, 0, rightInput);

                    // 处理音频
                    noiseSuppressionProcessor->processBlock(tempBuffer);

                    // 获取输出
                    leftOutput = tempBuffer.getSample(0, 0);
                    rightOutput = tempBuffer.getSample(1, 0);
                }
                else
                {
                    leftOutput = leftInput;
                    rightOutput = rightInput;
                }
            }
            break;

        default:
            leftOutput = leftInput;
            rightOutput = rightInput;
            break;
    }
}

float MainComponent::processJUCEDelay(float input)
{
    // Simple delay implementation using circular buffer
    static std::vector<float> delayBuffer(static_cast<size_t>(currentSampleRate * 2.0)); // 2 second max delay
    static size_t writeIndex = 0;
    
    if (delayBuffer.empty())
        return input;
    
    // Calculate delay in samples
    size_t delaySamples = static_cast<size_t>(juceDelayTime * currentSampleRate / 1000.0);
    delaySamples = std::min(delaySamples, delayBuffer.size() - 1);
    
    // Calculate read index
    size_t readIndex = (writeIndex + delayBuffer.size() - delaySamples) % delayBuffer.size();
    
    // Get delayed sample
    float delayedSample = delayBuffer[readIndex];
    
    // Apply feedback
    float feedbackSample = input + delayedSample * juceDelayFeedback;
    
    // Store in delay buffer
    delayBuffer[writeIndex] = feedbackSample;
    writeIndex = (writeIndex + 1) % delayBuffer.size();
    
    // Mix dry and wet signals
    return input * (1.0f - juceDelayMix) + delayedSample * juceDelayMix;
}

float MainComponent::processJUCEFlanger(float input)
{
    // Validate input to prevent NaN/infinity issues
    if (!std::isfinite(input))
        input = 0.0f;
    
    // Create stereo audio buffer for JUCE DSP processing
    static juce::AudioBuffer<float> buffer(2, 1);
    buffer.clear();
    buffer.setSample(0, 0, input);
    buffer.setSample(1, 0, input); // Set both channels
    
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    juceFlanger.process(context);
    
    // Get output and validate
    float output = buffer.getSample(0, 0);
    if (!std::isfinite(output))
        output = 0.0f;
    
    return output;
}

float MainComponent::processJUCEChorus(float input)
{
    // Validate input to prevent NaN/infinity issues
    if (!std::isfinite(input))
        input = 0.0f;
    
    // Create stereo audio buffer for JUCE DSP processing
    static juce::AudioBuffer<float> buffer(2, 1);
    buffer.clear();
    buffer.setSample(0, 0, input);
    buffer.setSample(1, 0, input); // Set both channels
    
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    juceChorus.process(context);
    
    // Get output and validate
    float output = buffer.getSample(0, 0);
    if (!std::isfinite(output))
        output = 0.0f;
    
    return output;
}

float MainComponent::processJUCEPlateReverb(float input)
{
    // Validate input to prevent NaN/infinity issues
    if (!std::isfinite(input))
        input = 0.0f;
    
    // Create stereo audio buffer for JUCE DSP processing
    static juce::AudioBuffer<float> buffer(2, 1);
    buffer.clear();
    buffer.setSample(0, 0, input);
    buffer.setSample(1, 0, input); // Set both channels
    
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    jucePlateReverb.process(context);
    
    // Get output and validate
    float output = buffer.getSample(0, 0);
    if (!std::isfinite(output))
        output = 0.0f;
    
    return output;
}

float MainComponent::processJUCEReverb(float input)
{
    // Validate input to prevent NaN/infinity issues
    if (!std::isfinite(input))
        input = 0.0f;
    
    // Create stereo audio buffer for JUCE DSP processing
    static juce::AudioBuffer<float> buffer(2, 1);
    buffer.clear();
    buffer.setSample(0, 0, input);
    buffer.setSample(1, 0, input); // Set both channels
    
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    juceReverb.process(context);
    
    // Get output and validate
    float output = buffer.getSample(0, 0);
    if (!std::isfinite(output))
        output = 0.0f;
    
    return output;
}

float MainComponent::processJUCEPhaser(float input)
{
    // Validate input to prevent NaN/infinity issues
    if (!std::isfinite(input))
        input = 0.0f;
    
    // Create stereo audio buffer for JUCE DSP processing
    static juce::AudioBuffer<float> buffer(2, 1);
    buffer.clear();
    buffer.setSample(0, 0, input);
    buffer.setSample(1, 0, input); // Set both channels
    
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    jucePhaser.process(context);
    
    // Get output and validate
    float output = buffer.getSample(0, 0);
    if (!std::isfinite(output))
        output = 0.0f;
    
    return output;
}

float MainComponent::processJUCECompressor(float input)
{
    // Validate input to prevent NaN/infinity issues
    if (!std::isfinite(input))
        input = 0.0f;
    
    // Create stereo audio buffer for JUCE DSP processing
    static juce::AudioBuffer<float> buffer(2, 1);
    buffer.clear();
    buffer.setSample(0, 0, input);
    buffer.setSample(1, 0, input); // Set both channels
    
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    juceCompressor.process(context);
    
    // Get output and validate
    float output = buffer.getSample(0, 0);
    if (!std::isfinite(output))
        output = 0.0f;
    
    return output;
}

float MainComponent::processJUCEPitchShift(float input)
{
    // Simple pitch shift implementation using delay and modulation
    static std::vector<float> pitchBuffer(static_cast<size_t>(currentSampleRate * 0.1)); // 100ms buffer
    static size_t writeIndex = 0;
    static float phase = 0.0f;
    
    if (pitchBuffer.empty())
        return input;
    
    // Calculate pitch shift ratio
    float pitchRatio = std::pow(2.0f, jucePitchShift / 12.0f);
    
    // Store input in buffer
    pitchBuffer[writeIndex] = input;
    writeIndex = (writeIndex + 1) % pitchBuffer.size();
    
    // Calculate read position with pitch shift
    float readPos = writeIndex - (pitchBuffer.size() / 2.0f) * pitchRatio;
    if (readPos < 0) readPos += pitchBuffer.size();
    
    // Linear interpolation for fractional delay
    size_t readIndex1 = static_cast<size_t>(readPos) % pitchBuffer.size();
    size_t readIndex2 = (readIndex1 + 1) % pitchBuffer.size();
    float fraction = readPos - std::floor(readPos);
    
    float sample1 = pitchBuffer[readIndex1];
    float sample2 = pitchBuffer[readIndex2];
    float output = sample1 + fraction * (sample2 - sample1);
    
    return output;
}

float MainComponent::processJUCEDistortion(float input)
{
    // Validate input to prevent NaN/infinity issues
    if (!std::isfinite(input))
        input = 0.0f;
    
    // Create stereo audio buffer for JUCE DSP processing
    static juce::AudioBuffer<float> buffer(2, 1);
    buffer.clear();
    buffer.setSample(0, 0, input);
    buffer.setSample(1, 0, input); // Set both channels
    
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    juceDistortion.process(context);
    
    // Get output and validate
    float output = buffer.getSample(0, 0);
    if (!std::isfinite(output))
        output = 0.0f;
    
    return output;
}

void MainComponent::updateJUCEFlangerParameters()
{
    juceFlanger.setRate(juceFlangerRate);
    juceFlanger.setDepth(juceFlangerDepth);
    juceFlanger.setCentreDelay(juceFlangerCentreDelay);
    juceFlanger.setFeedback(juceFlangerFeedback);
    juceFlanger.setMix(juceFlangerMix);
}

void MainComponent::updateJUCEChorusParameters()
{
    juceChorus.setRate(juceChorusRate);
    juceChorus.setDepth(juceChorusDepth);
    juceChorus.setCentreDelay(juceChorusCentreDelay);
    juceChorus.setFeedback(juceChorusFeedback);
    juceChorus.setMix(juceChorusMix);
}

void MainComponent::updateJUCEPlateReverbParameters()
{
    juce::Reverb::Parameters params;
    params.roomSize = jucePlateReverbRoomSize;
    params.damping = jucePlateReverbDamping;
    params.wetLevel = jucePlateReverbWetLevel;
    params.dryLevel = 1.0f - jucePlateReverbWetLevel; // Complement of wet level
    params.width = 1.0f;
    params.freezeMode = 0.0f;
    
    jucePlateReverb.setParameters(params);
}

void MainComponent::updateJUCEReverbParameters()
{
    juce::Reverb::Parameters params;
    params.roomSize = juceReverbRoomSize;
    params.damping = juceReverbDamping;
    params.wetLevel = juceReverbWetLevel;
    params.dryLevel = juceReverbDryLevel;
    params.width = juceReverbWidth;
    params.freezeMode = 0.0f;
    
    juceReverb.setParameters(params);
}

void MainComponent::updateJUCEPhaserParameters()
{
    jucePhaser.setRate(jucePhaserRate);
    jucePhaser.setDepth(jucePhaserDepth);
    jucePhaser.setCentreFrequency(jucePhaserCentreFreq);
    jucePhaser.setFeedback(jucePhaserFeedback);
    jucePhaser.setMix(jucePhaserMix);
}

void MainComponent::updateJUCECompressorParameters()
{
    juceCompressor.setThreshold(juceCompressorThreshold);
    juceCompressor.setRatio(juceCompressorRatio);
    juceCompressor.setAttack(juceCompressorAttack);
    juceCompressor.setRelease(juceCompressorRelease);
}

void MainComponent::updateJUCEDelayParameters()
{
    // JUCE Delay parameters are handled in processJUCEDelay function
    // No additional setup needed for the simple delay implementation
}

void MainComponent::updateJUCEPitchShiftParameters()
{
    // JUCE Pitch Shift parameters are handled in processJUCEPitchShift function
    // No additional setup needed for the simple pitch shift implementation
}

void MainComponent::updateJUCEDistortionParameters()
{
    // Store current parameter values in static variables for lambda capture
    static float currentDrive = 0.5f;
    static float currentRange = 0.5f;
    static float currentBlend = 0.5f;
    
    // Update static variables with current slider values
    currentDrive = juceDistortionDrive;
    currentRange = juceDistortionRange;
    currentBlend = juceDistortionBlend;
    
    // Configure distortion waveshaper function
    juceDistortion.functionToUse = [](float x) -> float {
        // Access the static variables instead of member variables
        float drive = 1.0f + currentDrive * 10.0f;
        float processed = std::tanh(x * drive);
        return processed * currentRange * currentBlend + x * (1.0f - currentBlend);
    };
}

// Update effect control display
void MainComponent::updateEffectControls()
{
    // First hide all effect controls
    hideAllEffectControls();
    
    // Show corresponding controls based on currently selected effect
    switch (currentEffect)
    {
        case Delay:
            delaySlider.setVisible(true);
            delayLabel.setVisible(true);
            feedbackSlider.setVisible(true);
            feedbackLabel.setVisible(true);
            break;
        
        case Flanger:
            flangerRateSlider.setVisible(true);
            flangerRateLabel.setVisible(true);
            flangerDepthSlider.setVisible(true);
            flangerDepthLabel.setVisible(true);
            break;
        
        case PlateReverb:
            plateDecaySlider.setVisible(true);
            plateDecayLabel.setVisible(true);
            plateToneSlider.setVisible(true);
            plateToneLabel.setVisible(true);
            break;
        
        case PitchShift:
            pitchSemitoneSlider.setVisible(true);
            pitchSemitoneLabel.setVisible(true);
            pitchDetuneSlider.setVisible(true);
            pitchDetuneLabel.setVisible(true);
            break;
        
        case CathedralReverb:
            cathedralDecaySlider.setVisible(true);
            cathedralDecayLabel.setVisible(true);
            cathedralToneSlider.setVisible(true);
            cathedralToneLabel.setVisible(true);
            break;
        
        case JUCEDelay:
            juceDelayTimeSlider.setVisible(true);
            juceDelayTimeLabel.setVisible(true);
            juceDelayFeedbackSlider.setVisible(true);
            juceDelayFeedbackLabel.setVisible(true);
            juceDelayMixSlider.setVisible(true);
            juceDelayMixLabel.setVisible(true);
            break;
        
        case JUCEChorus:
            juceChorusRateSlider.setVisible(true);
            juceChorusRateLabel.setVisible(true);
            juceChorusDepthSlider.setVisible(true);
            juceChorusDepthLabel.setVisible(true);
            juceChorusCentreDelaySlider.setVisible(true);
            juceChorusCentreDelayLabel.setVisible(true);
            juceChorusFeedbackSlider.setVisible(true);
            juceChorusFeedbackLabel.setVisible(true);
            juceChorusMixSlider.setVisible(true);
            juceChorusMixLabel.setVisible(true);
            break;
        
        case JUCEReverb:
            juceReverbRoomSizeSlider.setVisible(true);
            juceReverbRoomSizeLabel.setVisible(true);
            juceReverbDampingSlider.setVisible(true);
            juceReverbDampingLabel.setVisible(true);
            juceReverbWetLevelSlider.setVisible(true);
            juceReverbWetLevelLabel.setVisible(true);
            juceReverbDryLevelSlider.setVisible(true);
            juceReverbDryLevelLabel.setVisible(true);
            juceReverbWidthSlider.setVisible(true);
            juceReverbWidthLabel.setVisible(true);
            break;
        
        case JUCEPhaser:
            jucePhaserRateSlider.setVisible(true);
            jucePhaserRateLabel.setVisible(true);
            jucePhaserDepthSlider.setVisible(true);
            jucePhaserDepthLabel.setVisible(true);
            jucePhaserCentreFreqSlider.setVisible(true);
            jucePhaserCentreFreqLabel.setVisible(true);
            jucePhaserFeedbackSlider.setVisible(true);
            jucePhaserFeedbackLabel.setVisible(true);
            jucePhaserMixSlider.setVisible(true);
            jucePhaserMixLabel.setVisible(true);
            break;
        
        case JUCECompressor:
            juceCompressorThresholdSlider.setVisible(true);
            juceCompressorThresholdLabel.setVisible(true);
            juceCompressorRatioSlider.setVisible(true);
            juceCompressorRatioLabel.setVisible(true);
            juceCompressorAttackSlider.setVisible(true);
            juceCompressorAttackLabel.setVisible(true);
            juceCompressorReleaseSlider.setVisible(true);
            juceCompressorReleaseLabel.setVisible(true);
            break;
        
        case JUCEDistortion:
            juceDistortionDriveSlider.setVisible(true);
            juceDistortionDriveLabel.setVisible(true);
            juceDistortionRangeSlider.setVisible(true);
            juceDistortionRangeLabel.setVisible(true);
            juceDistortionBlendSlider.setVisible(true);
            juceDistortionBlendLabel.setVisible(true);
            break;

        case NoiseSuppression:
            noiseSuppressionVadThresholdSlider.setVisible(true);
            noiseSuppressionVadThresholdLabel.setVisible(true);
            noiseSuppressionVadGracePeriodSlider.setVisible(true);
            noiseSuppressionVadGracePeriodLabel.setVisible(true);
            noiseSuppressionVadRetroactiveGracePeriodSlider.setVisible(true);
            noiseSuppressionVadRetroactiveGracePeriodLabel.setVisible(true);
            break;

        default:
            // All controls are already hidden
            break;
    }
}

// Helper function to hide all effect controls
void MainComponent::hideAllEffectControls()
{
    // Original effect controls
    delaySlider.setVisible(false);
    delayLabel.setVisible(false);
    feedbackSlider.setVisible(false);
    feedbackLabel.setVisible(false);
    
    flangerRateSlider.setVisible(false);
    flangerRateLabel.setVisible(false);
    flangerDepthSlider.setVisible(false);
    flangerDepthLabel.setVisible(false);
    
    plateDecaySlider.setVisible(false);
    plateDecayLabel.setVisible(false);
    plateToneSlider.setVisible(false);
    plateToneLabel.setVisible(false);
    
    pitchSemitoneSlider.setVisible(false);
    pitchSemitoneLabel.setVisible(false);
    pitchDetuneSlider.setVisible(false);
    pitchDetuneLabel.setVisible(false);
    
    cathedralDecaySlider.setVisible(false);
    cathedralDecayLabel.setVisible(false);
    cathedralToneSlider.setVisible(false);
    cathedralToneLabel.setVisible(false);
    
    // JUCE effect controls
    juceDelayTimeSlider.setVisible(false);
    juceDelayTimeLabel.setVisible(false);
    juceDelayFeedbackSlider.setVisible(false);
    juceDelayFeedbackLabel.setVisible(false);
    juceDelayMixSlider.setVisible(false);
    juceDelayMixLabel.setVisible(false);
    
    juceChorusRateSlider.setVisible(false);
    juceChorusRateLabel.setVisible(false);
    juceChorusDepthSlider.setVisible(false);
    juceChorusDepthLabel.setVisible(false);
    juceChorusCentreDelaySlider.setVisible(false);
    juceChorusCentreDelayLabel.setVisible(false);
    juceChorusFeedbackSlider.setVisible(false);
    juceChorusFeedbackLabel.setVisible(false);
    juceChorusMixSlider.setVisible(false);
    juceChorusMixLabel.setVisible(false);
    
    juceReverbRoomSizeSlider.setVisible(false);
    juceReverbRoomSizeLabel.setVisible(false);
    juceReverbDampingSlider.setVisible(false);
    juceReverbDampingLabel.setVisible(false);
    juceReverbWetLevelSlider.setVisible(false);
    juceReverbWetLevelLabel.setVisible(false);
    juceReverbDryLevelSlider.setVisible(false);
    juceReverbDryLevelLabel.setVisible(false);
    juceReverbWidthSlider.setVisible(false);
    juceReverbWidthLabel.setVisible(false);
    
    jucePhaserRateSlider.setVisible(false);
    jucePhaserRateLabel.setVisible(false);
    jucePhaserDepthSlider.setVisible(false);
    jucePhaserDepthLabel.setVisible(false);
    jucePhaserCentreFreqSlider.setVisible(false);
    jucePhaserCentreFreqLabel.setVisible(false);
    jucePhaserFeedbackSlider.setVisible(false);
    jucePhaserFeedbackLabel.setVisible(false);
    jucePhaserMixSlider.setVisible(false);
    jucePhaserMixLabel.setVisible(false);
    
    juceCompressorThresholdSlider.setVisible(false);
    juceCompressorThresholdLabel.setVisible(false);
    juceCompressorRatioSlider.setVisible(false);
    juceCompressorRatioLabel.setVisible(false);
    juceCompressorAttackSlider.setVisible(false);
    juceCompressorAttackLabel.setVisible(false);
    juceCompressorReleaseSlider.setVisible(false);
    juceCompressorReleaseLabel.setVisible(false);
    
    juceDistortionDriveSlider.setVisible(false);
    juceDistortionDriveLabel.setVisible(false);
    juceDistortionRangeSlider.setVisible(false);
    juceDistortionRangeLabel.setVisible(false);
    juceDistortionBlendSlider.setVisible(false);
    juceDistortionBlendLabel.setVisible(false);

    // Noise Suppression controls
    noiseSuppressionVadThresholdSlider.setVisible(false);
    noiseSuppressionVadThresholdLabel.setVisible(false);
    noiseSuppressionVadGracePeriodSlider.setVisible(false);
    noiseSuppressionVadGracePeriodLabel.setVisible(false);
    noiseSuppressionVadRetroactiveGracePeriodSlider.setVisible(false);
    noiseSuppressionVadRetroactiveGracePeriodLabel.setVisible(false);
}