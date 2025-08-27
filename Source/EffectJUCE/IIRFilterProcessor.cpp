#include "IIRFilterProcessor.h"
#include <cmath>

IIRFilterProcessor::IIRFilterProcessor() : currentSampleRate(44100.0f)
{
    initializeDefaultParams();
    reset();
}

void IIRFilterProcessor::initializeDefaultParams()
{
    // Initialize default 7-band EQ parameters
    bandParams[0] = {"PEQ", 100.0f, 0.0f, 0.7f};   // Band 1: 100 Hz
    bandParams[1] = {"PEQ", 200.0f, 0.0f, 0.7f};   // Band 2: 200 Hz
    bandParams[2] = {"PEQ", 400.0f, 0.0f, 0.7f};   // Band 3: 400 Hz
    bandParams[3] = {"PEQ", 800.0f, 0.0f, 0.7f};   // Band 4: 800 Hz
    bandParams[4] = {"PEQ", 1600.0f, 0.0f, 0.7f};  // Band 5: 1600 Hz
    bandParams[5] = {"PEQ", 3200.0f, 0.0f, 0.7f};  // Band 6: 3200 Hz
    bandParams[6] = {"PEQ", 6400.0f, 0.0f, 0.7f};  // Band 7: 6400 Hz
    
    // Design all filters with default parameters
    for (int i = 0; i < 7; ++i)
    {
        designFilter(&filters[i], bandParams[i].type, bandParams[i].freq, 
                    bandParams[i].gain, bandParams[i].Q, currentSampleRate);
    }
}

void IIRFilterProcessor::initBiquadFilter(BiquadFilter* filter, float b0, float b1, float b2, float a1, float a2)
{
    filter->b0 = b0;
    filter->b1 = b1;
    filter->b2 = b2;
    filter->a1 = a1;
    filter->a2 = a2;
    filter->x1 = 0.0f;
    filter->x2 = 0.0f;
    filter->y1 = 0.0f;
    filter->y2 = 0.0f;
}

float IIRFilterProcessor::processBiquadFilter(BiquadFilter* filter, float input)
{
    float output = filter->b0 * input
                 + filter->b1 * filter->x1
                 + filter->b2 * filter->x2
                 - filter->a1 * filter->y1
                 - filter->a2 * filter->y2;

    filter->x2 = filter->x1;
    filter->x1 = input;
    filter->y2 = filter->y1;
    filter->y1 = output;

    return output;
}

void IIRFilterProcessor::designFilter(BiquadFilter* filter, const juce::String& type, float freq, float gain, float Q, float sampleRate)
{
    if (freq <= 20.0f || freq >= sampleRate / 2.0f || Q <= 0.0f)
    {
        // Invalid parameters, set to bypass
        initBiquadFilter(filter, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        return;
    }

    float omega = 2.0f * juce::MathConstants<float>::pi * freq / sampleRate;
    float sn = std::sin(omega);
    float cs = std::cos(omega);
    float alpha = sn / (2.0f * Q);
    float A = std::pow(10.0f, gain / 40.0f);  // Important correction: PEQ should use /40 not /20

    float a0, b0, b1, b2, a1, a2;

    if (type == "PEQ")
    {
        if (std::abs(gain) <= 0.01f)
        {
            // Bypass filter when gain is near zero
            b0 = 1.0f; b1 = 0.0f; b2 = 0.0f;
            a0 = 1.0f; a1 = 0.0f; a2 = 0.0f;
        }
        else
        {
            // Standard Peaking EQ (Bell) formula
            b0 = 1.0f + alpha * A;
            b1 = -2.0f * cs;
            b2 = 1.0f - alpha * A;
            a0 = 1.0f + alpha / A;
            a1 = -2.0f * cs;
            a2 = 1.0f - alpha / A;
        }
    }
    else if (type == "LSH") // Low Shelf
    {
        // Low shelf correctly uses /20
        float A_shelf = std::pow(10.0f, gain / 20.0f);
        
        a0 = (A_shelf + 1.0f) + (A_shelf - 1.0f) * cs + 2.0f * std::sqrt(A_shelf) * alpha;
        b0 = A_shelf * ((A_shelf + 1.0f) - (A_shelf - 1.0f) * cs + 2.0f * std::sqrt(A_shelf) * alpha);
        b1 = 2.0f * A_shelf * ((A_shelf - 1.0f) - (A_shelf + 1.0f) * cs);
        b2 = A_shelf * ((A_shelf + 1.0f) - (A_shelf - 1.0f) * cs - 2.0f * std::sqrt(A_shelf) * alpha);
        a1 = -2.0f * ((A_shelf - 1.0f) + (A_shelf + 1.0f) * cs);
        a2 = (A_shelf + 1.0f) + (A_shelf - 1.0f) * cs - 2.0f * std::sqrt(A_shelf) * alpha;
    }
    else if (type == "HSH") // High Shelf
    {
        // High shelf correctly uses /20
        float A_shelf = std::pow(10.0f, gain / 20.0f);
        
        a0 = (A_shelf + 1.0f) - (A_shelf - 1.0f) * cs + 2.0f * std::sqrt(A_shelf) * alpha;
        b0 = A_shelf * ((A_shelf + 1.0f) + (A_shelf - 1.0f) * cs + 2.0f * std::sqrt(A_shelf) * alpha);
        b1 = -2.0f * A_shelf * ((A_shelf - 1.0f) + (A_shelf + 1.0f) * cs);
        b2 = A_shelf * ((A_shelf + 1.0f) + (A_shelf - 1.0f) * cs - 2.0f * std::sqrt(A_shelf) * alpha);
        a1 = 2.0f * ((A_shelf - 1.0f) - (A_shelf + 1.0f) * cs);
        a2 = (A_shelf + 1.0f) - (A_shelf - 1.0f) * cs - 2.0f * std::sqrt(A_shelf) * alpha;
    }
    else if (type == "LPF") // Low Pass
    {
        a0 = 1.0f + alpha;
        b0 = (1.0f - cs) / 2.0f;
        b1 = 1.0f - cs;
        b2 = (1.0f - cs) / 2.0f;
        a1 = -2.0f * cs;
        a2 = 1.0f - alpha;
    }
    else if (type == "HPF") // High Pass
    {
        a0 = 1.0f + alpha;
        b0 = (1.0f + cs) / 2.0f;
        b1 = -(1.0f + cs);
        b2 = (1.0f + cs) / 2.0f;
        a1 = -2.0f * cs;
        a2 = 1.0f - alpha;
    }
    else
    {
        // Default to bypass
        a0 = 1.0f; b0 = 1.0f; b1 = 0.0f; b2 = 0.0f; a1 = 0.0f; a2 = 0.0f;
    }

    if (std::abs(a0) < 1e-6f)
    {
        // Avoid division by zero
        initBiquadFilter(filter, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        return;
    }

    // Normalize coefficients
    b0 /= a0; b1 /= a0; b2 /= a0; a1 /= a0; a2 /= a0;

    initBiquadFilter(filter, b0, b1, b2, a1, a2);
}

float IIRFilterProcessor::processEQ(float input)
{
    float output = input;
    
    //DBG("Input: " << input);
    
    for (int i = 0; i < 7; ++i)
    {
        float before = output;
        output = processBiquadFilter(&filters[i], output);
        //DBG("Band " << i << ": " << before << " -> " << output << " (ratio: " << output/before << ")");
    }
    
    //DBG("Final output: " << output << " (ratio: " << output/input << ")");
    
    return output;
}

void IIRFilterProcessor::updateBand(int bandIndex, const juce::String& type, float freq, float gain, float Q)
{
    if (bandIndex < 0 || bandIndex >= 7)
        return;
        
    bandParams[bandIndex].type = type;
    bandParams[bandIndex].freq = freq;
    bandParams[bandIndex].gain = gain;
    bandParams[bandIndex].Q = Q;
    
    designFilter(&filters[bandIndex], type, freq, gain, Q, currentSampleRate);
}

void IIRFilterProcessor::setSampleRate(float sampleRate)
{
    currentSampleRate = sampleRate;
    
    // Redesign all filters with new sample rate
    for (int i = 0; i < 7; ++i)
    {
        designFilter(&filters[i], bandParams[i].type, bandParams[i].freq, 
                    bandParams[i].gain, bandParams[i].Q, currentSampleRate);
    }
}

void IIRFilterProcessor::reset()
{
    for (int i = 0; i < 7; ++i)
    {
        filters[i].x1 = filters[i].x2 = 0.0f;
        filters[i].y1 = filters[i].y2 = 0.0f;
    }
}