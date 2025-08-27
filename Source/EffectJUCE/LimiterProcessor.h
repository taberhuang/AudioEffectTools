#pragma once
#include <JuceHeader.h>
#include <cmath>

class LimiterProcessor
{
public:
    enum LimiterType
    {
        RMS,
        Peak
    };

    struct LimiterParams
    {
        float threshold = 0.0f;    // dBFS (-60 to 0)
        float ratio = 10.0f;       // 1:1 to 20:1
        float attack = 50.0f;      // ms (1 to 100)
        float release = 200.0f;    // ms (10 to 1000)
        bool enabled = false;
    };

    LimiterProcessor();
    ~LimiterProcessor() = default;

    // Process audio sample
    float processLimiter(float input);
    
    // Update parameters
    void setRMSParams(const LimiterParams& params);
    void setPeakParams(const LimiterParams& params);
    void setSampleRate(float sampleRate);
    
    // Enable/disable limiters
    void setRMSEnabled(bool enabled) { rmsParams.enabled = enabled; }
    void setPeakEnabled(bool enabled) { peakParams.enabled = enabled; }
    
    // Get current parameters
    const LimiterParams& getRMSParams() const { return rmsParams; }
    const LimiterParams& getPeakParams() const { return peakParams; }

private:
    // Limiter state
    struct LimiterState
    {
        float envelope = 0.0f;
        float rmsSum = 0.0f;
        int rmsBufferIndex = 0;
        static constexpr int rmsBufferSize = 512;
        float rmsBuffer[rmsBufferSize] = {0.0f};
    };

    LimiterParams rmsParams;
    LimiterParams peakParams;
    LimiterState rmsState;
    LimiterState peakState;
    
    float currentSampleRate = 44100.0f;
    
    // Processing functions
    float processRMSLimiter(float input);
    float processPeakLimiter(float input);
    
    // Helper functions
    float calculateRMS();
    void updateEnvelope(float& envelope, float targetLevel, float attackTime, float releaseTime);
    
    // dB conversion helpers
    static float dBToLinear(float dB) { return std::pow(10.0f, dB / 20.0f); }
    static float linearTodB(float linear) 
    { 
        return (linear > 0.0f) ? 20.0f * std::log10(linear) : -100.0f; 
    }
};