#pragma once
#include <JuceHeader.h>

class WaveformAnalyzer : public juce::Component, private juce::Timer
{
public:
    WaveformAnalyzer();
    ~WaveformAnalyzer();
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;
    
    void setSampleRate(double newSampleRate);
    void pushSample(float sample);
    
private:
    void timerCallback() override;
    void drawWaveform(juce::Graphics& g, const juce::Rectangle<int>& area);
    void drawGrid(juce::Graphics& g, const juce::Rectangle<int>& area);
    void drawYAxisLabels(juce::Graphics& g, const juce::Rectangle<int>& area);
    void drawXAxisLabels(juce::Graphics& g, const juce::Rectangle<int>& area);
    void drawRMS(juce::Graphics& g, const juce::Rectangle<int>& area);
    float calculateRMS() const;
    float calculatePeak() const; // Declaration of new peak calculation method
    
    // Buffer for storing samples
    static constexpr int maxBufferSize = 8192;  // Maximum buffer size for longest time window
    std::vector<float> sampleBuffer;
    int currentBufferSize = 2048;  // Current active buffer size
    int writePosition = 0;
    
    // Sample rate
    double sampleRate = 44100.0;
    
    // Y-axis scaling
    float yAxisMax = 1.0f;  // Maximum displayable amplitude
    static constexpr float minYAxisMax = 0.1f;   // Minimum zoom level
    static constexpr float maxYAxisMax = 1.0f;   // Maximum zoom level
    static constexpr float zoomFactor = 1.1f;    // Zoom speed
    
    // X-axis scaling and offset
    float xAxisTimeWindow = 0.046f;  // Time window in seconds (default ~2048 samples at 44.1kHz)
    float xAxisTimeOffset = 0.0f;   // Time offset in seconds (0 = center)
    static constexpr float minTimeWindow = 0.001f;  // Minimum time window (1ms)
    static constexpr float maxTimeWindow = 0.2f;    // Maximum time window (200ms)
    
    // Display resolution (samples per pixel)
    int targetSamplesPerPixel = 1;  // Adjust based on window width
    
    // RMS and Peak calculations
    float rmsValue = 0.0f;
    float peakValue = 0.0f; // New peak value variable
    
    void updateBufferSize();
    
    // Colors
    juce::Colour backgroundColour = juce::Colour(0xff1e1e1e);
    juce::Colour gridColour = juce::Colour(0xff404040);
    juce::Colour waveformColour = juce::Colour(0xff00ff00);
    juce::Colour textColour = juce::Colour(0xffcccccc);
    juce::Colour rmsColour = juce::Colour(0xffff4040); // Red for RMS lines (not used now)
    
    // Layout constants
    static constexpr int leftMargin = 50;   // Space for Y-axis labels
    static constexpr int bottomMargin = 30; // Space for X-axis labels
    static constexpr int rightMargin = 30;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformAnalyzer)
};