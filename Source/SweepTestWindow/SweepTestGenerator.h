#pragma once

#include <JuceHeader.h>
#include <memory>
#include <atomic>

class SweepTestGenerator
{
public:
    SweepTestGenerator();
    ~SweepTestGenerator();
    
    // Initialize sweep parameters
    void prepare(double sampleRate);
    
    // Start/stop sweep test
    void startSweep();
    void stopSweep();
    bool isSweeeping() const { return isSweeping.load(); }
    
    // Generate next sweep sample
    float getNextSample();
    
    // Get current frequency (for display)
    float getCurrentFrequency() const { return currentFrequency.load(); }
    
    // Get progress (0.0 - 1.0)
    float getProgress() const;
    
    // Set parameters
    void setFrequencyRange(float startFreq, float endFreq);
    void setDuration(float seconds);
    void setAmplitude(float dB);
    
private:
    // Parameters
    float startFrequency = 20.0f;
    float endFrequency = 20000.0f;
    float sweepDuration = 2.0f;
    float amplitude = 0.0f; // Linear value converted from -15dB
    double sampleRate = 44100.0;
    
    // State
    std::atomic<bool> isSweeping{false};
    std::atomic<float> currentFrequency{20.0f};
    int64_t currentSample = 0;
    int64_t totalSamples = 0;
    double phase = 0.0;
    
    // Calculate frequency at current time (logarithmic sweep)
    float calculateFrequencyAtTime(double timeRatio);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SweepTestGenerator)
};