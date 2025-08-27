// SweepTestGenerator.cpp
#include "SweepTestGenerator.h"

SweepTestGenerator::SweepTestGenerator()
{
    // Set default amplitude of -15dB
    setAmplitude(-15.0f);
}

SweepTestGenerator::~SweepTestGenerator()
{
    stopSweep();
}

void SweepTestGenerator::prepare(double sr)
{
    sampleRate = sr;
    totalSamples = static_cast<int64_t>(sweepDuration * sampleRate);
}

void SweepTestGenerator::startSweep()
{
    if (!isSweeping.load())
    {
        currentSample = 0;
        phase = 0.0;
        currentFrequency = startFrequency;
        totalSamples = static_cast<int64_t>(sweepDuration * sampleRate);
        isSweeping = true;
        
        DBG("Sweep test started: " << startFrequency << "Hz to " << endFrequency 
            << "Hz, duration: " << sweepDuration << "s, amplitude: " 
            << juce::Decibels::gainToDecibels(amplitude) << "dB");
    }
}

void SweepTestGenerator::stopSweep()
{
    isSweeping = false;
    currentSample = 0;
    phase = 0.0;
    DBG("Sweep test stopped");
}

float SweepTestGenerator::getNextSample()
{
    if (!isSweeping.load())
        return 0.0f;
    
    if (currentSample >= totalSamples)
    {
        stopSweep();
        return 0.0f;
    }
    
    // Calculate current time ratio
    double timeRatio = static_cast<double>(currentSample) / static_cast<double>(totalSamples);
    
    // Calculate current frequency (logarithmic sweep)
    float freq = calculateFrequencyAtTime(timeRatio);
    currentFrequency = freq;
    
    // Generate sine wave
    double deltaPhase = freq * 2.0 * juce::MathConstants<double>::pi / sampleRate;
    phase += deltaPhase;
    
    // Keep phase between 0 and 2π
    if (phase >= 2.0 * juce::MathConstants<double>::pi)
        phase -= 2.0 * juce::MathConstants<double>::pi;
    
    float sample = amplitude * std::sin(phase);
    
    // Apply fade in/out (to avoid clicks)
    const int fadeInSamples = static_cast<int>(0.01 * sampleRate); // 10ms fade in
    const int fadeOutSamples = static_cast<int>(0.01 * sampleRate); // 10ms fade out
    
    if (currentSample < fadeInSamples)
    {
        float fadeIn = static_cast<float>(currentSample) / fadeInSamples;
        sample *= fadeIn;
    }
    else if (currentSample > totalSamples - fadeOutSamples)
    {
        float fadeOut = static_cast<float>(totalSamples - currentSample) / fadeOutSamples;
        sample *= fadeOut;
    }
    
    currentSample++;
    
    return sample;
}

float SweepTestGenerator::getProgress() const
{
    if (totalSamples == 0)
        return 0.0f;
    
    return static_cast<float>(currentSample) / static_cast<float>(totalSamples);
}

void SweepTestGenerator::setFrequencyRange(float startFreq, float endFreq)
{
    startFrequency = juce::jlimit(20.0f, 20000.0f, startFreq);
    endFrequency = juce::jlimit(startFrequency, 20000.0f, endFreq);
}

void SweepTestGenerator::setDuration(float seconds)
{
    sweepDuration = juce::jlimit(0.1f, 10.0f, seconds);
}

void SweepTestGenerator::setAmplitude(float dB)
{
    amplitude = juce::Decibels::decibelsToGain(juce::jlimit(-60.0f, 0.0f, dB));
}

float SweepTestGenerator::calculateFrequencyAtTime(double timeRatio)
{
    // Logarithmic sweep: frequency grows exponentially with time
    // f(t) = f_start * (f_end/f_start)^(t/T)
    double logStart = std::log10(startFrequency);
    double logEnd = std::log10(endFrequency);
    double logFreq = logStart + timeRatio * (logEnd - logStart);
    
    return std::pow(10.0f, logFreq);
}