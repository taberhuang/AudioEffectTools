#include "LimiterProcessor.h"

LimiterProcessor::LimiterProcessor()
{
    // Initialize with default parameters
    rmsParams.threshold = 0.0f;
    rmsParams.ratio = 10.0f;
    rmsParams.attack = 50.0f;
    rmsParams.release = 200.0f;
    rmsParams.enabled = false;
    
    peakParams.threshold = 0.0f;
    peakParams.ratio = 10.0f;
    peakParams.attack = 50.0f;
    peakParams.release = 200.0f;
    peakParams.enabled = false;
    
    // Clear RMS buffer
    std::memset(rmsState.rmsBuffer, 0, sizeof(rmsState.rmsBuffer));
}

void LimiterProcessor::setSampleRate(float sampleRate)
{
    currentSampleRate = sampleRate;
}

void LimiterProcessor::setRMSParams(const LimiterParams& params)
{
    rmsParams = params;
}

void LimiterProcessor::setPeakParams(const LimiterParams& params)
{
    peakParams = params;
}

float LimiterProcessor::processLimiter(float input)
{
    float output = input;
    
    // Debug output every 48000 samples (1 second at 48kHz)
    static int debugCounter = 0;
    debugCounter++;
    
    // Process RMS limiter first if enabled
    if (rmsParams.enabled)
    {
        float beforeRMS = output;
        output = processRMSLimiter(output);
        
        if (debugCounter % 48000 == 0)
        {
            float inputDB = linearTodB(std::abs(beforeRMS));
            float outputDB = linearTodB(std::abs(output));
            float gainReduction = linearTodB(std::abs(beforeRMS)) - linearTodB(std::abs(output));
            DBG("RMS Limiter - Input: " << inputDB << "dB, Output: " << outputDB << "dB, Threshold: " << rmsParams.threshold << "dB, Ratio: " << rmsParams.ratio << ":1, GR: " << gainReduction << "dB");
        }
    }
    
    // Then process peak limiter if enabled
    if (peakParams.enabled)
    {
        float beforePeak = output;
        output = processPeakLimiter(output);
        
        if (debugCounter % 48000 == 0)
        {
            float inputDB = linearTodB(std::abs(beforePeak));
            float outputDB = linearTodB(std::abs(output));
            float gainReduction = linearTodB(std::abs(beforePeak)) - linearTodB(std::abs(output));
            DBG("Peak Limiter - Input: " << inputDB << "dB, Output: " << outputDB << "dB, Threshold: " << peakParams.threshold << "dB, Ratio: " << peakParams.ratio << ":1, GR: " << gainReduction << "dB");
        }
    }
    
    return output;
}

float LimiterProcessor::processRMSLimiter(float input)
{
    // Update RMS buffer
    rmsState.rmsBuffer[rmsState.rmsBufferIndex] = input * input;
    rmsState.rmsBufferIndex = (rmsState.rmsBufferIndex + 1) % LimiterState::rmsBufferSize;
    
    // Calculate current RMS level
    float rmsLevel = calculateRMS();
    float rmsLevelDB = linearTodB(rmsLevel);
    
    // Calculate gain reduction needed (in dB)
    float gainReductionDB = 0.0f;
    if (rmsLevelDB > rmsParams.threshold)
    {
        float excess = rmsLevelDB - rmsParams.threshold;
        // Standard compressor/limiter ratio calculation
        // For ratio = 1, no compression; for ratio = ∞, hard limiting
        if (rmsParams.ratio >= 20.0f)
        {
            // Hard limiting - output stays at threshold
            gainReductionDB = excess;
        }
        else
        {
            // Soft limiting - reduce excess by ratio
            gainReductionDB = excess - (excess / rmsParams.ratio);
        }
    }
    
    // Update envelope with gain reduction amount
    float attackTime = rmsParams.attack / 1000.0f;  // Convert ms to seconds
    float releaseTime = rmsParams.release / 1000.0f;
    updateEnvelope(rmsState.envelope, gainReductionDB, attackTime, releaseTime);
    
    // Apply gain reduction
    float gainLinear = dBToLinear(-rmsState.envelope); // Negative because it's reduction
    return input * gainLinear;
}

float LimiterProcessor::processPeakLimiter(float input)
{
    // Get input level in dB
    float inputLevel = std::abs(input);
    float inputLevelDB = linearTodB(inputLevel);
    
    // Calculate gain reduction needed (in dB)
    float gainReductionDB = 0.0f;
    if (inputLevelDB > peakParams.threshold)
    {
        float excess = inputLevelDB - peakParams.threshold;
        // Standard compressor/limiter ratio calculation
        // For ratio = 1, no compression; for ratio = ∞, hard limiting
        if (peakParams.ratio >= 20.0f)
        {
            // Hard limiting - output stays at threshold
            gainReductionDB = excess;
        }
        else
        {
            // Soft limiting - reduce excess by ratio
            gainReductionDB = excess - (excess / peakParams.ratio);
        }
    }
    
    // Update envelope with fast attack for peak limiting
    float attackTime = peakParams.attack / 1000.0f;
    float releaseTime = peakParams.release / 1000.0f;
    
    // For peak limiting, use faster attack when signal exceeds threshold
    if (gainReductionDB > peakState.envelope)
    {
        attackTime *= 0.1f; // 10x faster attack for peak catching
    }
    
    updateEnvelope(peakState.envelope, gainReductionDB, attackTime, releaseTime);
    
    // Apply gain reduction
    float gainLinear = dBToLinear(-peakState.envelope); // Negative because it's reduction
    return input * gainLinear;
}

float LimiterProcessor::calculateRMS()
{
    float sum = 0.0f;
    for (int i = 0; i < LimiterState::rmsBufferSize; ++i)
    {
        sum += rmsState.rmsBuffer[i];
    }
    return std::sqrt(sum / LimiterState::rmsBufferSize);
}

void LimiterProcessor::updateEnvelope(float& envelope, float targetGainReduction, float attackTime, float releaseTime)
{
    float attackCoeff = std::exp(-1.0f / (attackTime * currentSampleRate));
    float releaseCoeff = std::exp(-1.0f / (releaseTime * currentSampleRate));
    
    if (targetGainReduction > envelope)
    {
        // Attack phase (increasing gain reduction)
        envelope = targetGainReduction + (envelope - targetGainReduction) * attackCoeff;
    }
    else
    {
        // Release phase (decreasing gain reduction)
        envelope = targetGainReduction + (envelope - targetGainReduction) * releaseCoeff;
    }
}

