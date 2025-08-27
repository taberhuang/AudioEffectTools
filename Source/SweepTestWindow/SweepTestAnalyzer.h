// SweepTestAnalyzer.h
#pragma once

#include <JuceHeader.h>
#include <memory>
#include <vector>

class FrequencySpectrumDisplay : public juce::Component
{
public:
    FrequencySpectrumDisplay(const juce::String& title);
    ~FrequencySpectrumDisplay();
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Set spectrum data (magnitude in dB)
    void setSpectrumData(const std::vector<float>& magnitudes, 
                         const std::vector<float>& frequencies);
    
    // Clear display
    void clear();
    
private:
    juce::String displayTitle;
    std::vector<float> magnitudeData;
    std::vector<float> frequencyData;
    
    // Draw grid and labels
    void drawGrid(juce::Graphics& g, const juce::Rectangle<float>& plotArea);
    void drawFrequencyLabels(juce::Graphics& g, const juce::Rectangle<float>& plotArea);
    void drawMagnitudeLabels(juce::Graphics& g, const juce::Rectangle<float>& plotArea);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FrequencySpectrumDisplay)
};

// ============================================

class SweepTestAnalyzer : public juce::DocumentWindow
{
public:
    SweepTestAnalyzer(const juce::File& audioFile);
    ~SweepTestAnalyzer();
    
    void closeButtonPressed() override;
    
    // Static management functions
    static void showAnalyzer(const juce::File& audioFile);
    static void deleteAllAnalyzers();
    
private:
    class AnalyzerContent : public juce::Component
    {
    public:
        AnalyzerContent(const juce::File& audioFile);
        ~AnalyzerContent();
        
        void paint(juce::Graphics& g) override;
        void resized() override;
        
    private:
        // Spectrum display components
        std::unique_ptr<FrequencySpectrumDisplay> leftChannelDisplay;
        std::unique_ptr<FrequencySpectrumDisplay> rightChannelDisplay;
        std::unique_ptr<FrequencySpectrumDisplay> inputSpectrumDisplay;
        std::unique_ptr<FrequencySpectrumDisplay> outputSpectrumDisplay;
        
        // File information labels
        juce::Label fileInfoLabel;
        juce::Label analysisInfoLabel;
        
        // FFT parameters
        static constexpr int fftOrder = 12;
        static constexpr int fftSize = 1 << fftOrder;
        
        // Analyze audio file
        void analyzeAudioFile(const juce::File& file);

        // New: Sweep-specific analysis function
        void performSweepAnalysis(const float* audioData, int numSamples, 
                                  std::vector<float>& magnitudes, 
                                  std::vector<float>& frequencies, 
                                  double sampleRate);

        // Regular FFT analysis
        void performFFT(const float* audioData, int numSamples, 
                        std::vector<float>& magnitudes, 
                        std::vector<float>& frequencies, 
                        double sampleRate);
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalyzerContent)
    };
    
    std::unique_ptr<AnalyzerContent> content;
    
    // Static members for managing all open analyzer windows
    static juce::Array<SweepTestAnalyzer*> openAnalyzers;
    static juce::CriticalSection analyzerLock;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SweepTestAnalyzer)
};
