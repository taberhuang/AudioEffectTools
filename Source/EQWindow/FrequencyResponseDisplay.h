// FrequencyResponseDisplay.h
#pragma once
#include <JuceHeader.h>

// Forward declarations
class IIRFilterProcessor;
class EQWindow;

class FrequencyResponseDisplay : public juce::Component
{
public:
    explicit FrequencyResponseDisplay(IIRFilterProcessor* processor);
    ~FrequencyResponseDisplay() override;
    
    // Component overrides
    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    void parentHierarchyChanged() override;
    
    // Public methods
    void updateResponse();
    void setBandEnabled(int band, bool enabled);
    
private:
    IIRFilterProcessor* processor;
    juce::Path responseCurve;
    std::vector<juce::Point<float>> bandPoints;
    std::vector<bool> bandEnabled;
    int selectedBand;
    
    // Helper functions for coordinate conversion
    float frequencyToX(float freq, float width);
    float gainToY(float gain, float height);
    float xToFrequency(float x, float width);
    float yToGain(float y, float height);
    
    // Calculate frequency response for a single band
    void calcEqCoef(const juce::String& type, float freq, float gain, float Q, float* pCoef);
    void EqPeakFilter(float freq, float gain, float Q, float* pCoef);
    void EqLowShelf(float freq, float gain, float Q, float* pCoef);
    void EqHighShelf(float freq, float gain, float Q, float* pCoef);
    void EqLowPass(float freq, float Q, float* pCoef);
    void EqHighPass(float freq, float Q, float* pCoef);
    void EqIIRCoef(float c1, float c2, float c3, float c4, float c5, float c6, float* pCoef);
    
    // The JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR macro already includes
    // the declaration of deleted copy constructor and assignment operator, so no manual declaration is needed.
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FrequencyResponseDisplay)
};