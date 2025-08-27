#pragma once
#include <JuceHeader.h>

class IIRFilterProcessor
{
public:
    struct BiquadFilter
    {
        float b0, b1, b2, a1, a2;
        float x1, x2, y1, y2;
        
        BiquadFilter() : b0(1.0f), b1(0.0f), b2(0.0f), a1(0.0f), a2(0.0f),
                        x1(0.0f), x2(0.0f), y1(0.0f), y2(0.0f) {}
    };

    IIRFilterProcessor();
    ~IIRFilterProcessor() = default;

    void initBiquadFilter(BiquadFilter* filter, float b0, float b1, float b2, float a1, float a2);
    float processBiquadFilter(BiquadFilter* filter, float input);
    void designFilter(BiquadFilter* filter, const juce::String& type, float freq, float gain, float Q, float sampleRate);
    
    // Process audio sample through all 7 EQ bands
    float processEQ(float input);
    
    // Update EQ parameters
    void updateBand(int bandIndex, const juce::String& type, float freq, float gain, float Q);
    void setSampleRate(float sampleRate);
    
    // Reset filter states
    void reset();

private:
    BiquadFilter filters[7];
    float currentSampleRate;
    
    // Default EQ band parameters
    struct EQBandParams
    {
        juce::String type;
        float freq;
        float gain;
        float Q;
    };
    
    EQBandParams bandParams[7];
    void initializeDefaultParams();
};