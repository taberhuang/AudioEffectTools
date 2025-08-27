#pragma once

#include <JuceHeader.h>

class SpectrumAnalyzer : public juce::Component, public juce::Timer
{
public:
    SpectrumAnalyzer();
    ~SpectrumAnalyzer() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    // Push audio data for analysis
    void pushNextSampleIntoFifo(float sample) noexcept;

    // Set whether to show input or output spectrum
    void setShowInput(bool showInputFlag) { this->showInput = showInputFlag; }
    void setShowOutput(bool showOutputFlag) { this->showOutput = showOutputFlag; }
    
    // Set sample rate
    void setSampleRate(double sampleRate) { this->currentSampleRate = sampleRate; }

private:
    enum
    {
        fftOrder = 12,             // FFT order (2^12 = 4096 points) - Better frequency resolution
        fftSize = 1 << fftOrder,   // FFT size
        scopeSize = 1024           // Number of spectrum points to display (increased for better detail)
    };

    // FFT related
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;
    
    float fifo[fftSize];           // Input data buffer
    float fftData[2 * fftSize];    // FFT data buffer
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;
    float scopeData[scopeSize];    // Spectrum display data
    
    // Peak frequency detection
    float peakFrequency = 0.0f;
    float peakLevel = 0.0f;
    double currentSampleRate = 44100.0;
    
    // Smoothing variables for peak frequency detection
    float lastPeakFreq = 0.0f;
    float lastPeakLevel = -100.0f;
    float currentTHD;
    float currentTHDplusN;
    // Display control
    bool showInput = true;
    bool showOutput = true;
    
    // Frequency labels
    void drawFrequencyLabels(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawAmplitudeLabels(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawPeakFrequency(juce::Graphics& g, juce::Rectangle<int> bounds);
    
    // Spectrum processing
    void drawNextFrameOfSpectrum();
    float getFrequencyForBin(int binIndex, double sampleRate);
    void findPeakFrequency();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzer)
};