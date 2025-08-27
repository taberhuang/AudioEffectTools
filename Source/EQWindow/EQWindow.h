// EQWindow.h
#pragma once
#include <JuceHeader.h>
#include "FrequencyResponseDisplay.h"
#include "EQControls.h"

// Forward declarations
class IIRFilterProcessor;
class LimiterProcessor;

class EQWindow : public juce::DocumentWindow,
                 public juce::Slider::Listener,
                 public juce::ComboBox::Listener,
                 public juce::Button::Listener,
                 public juce::Timer
{
public:
    EQWindow(IIRFilterProcessor* processor);
    void setSecondEQProcessor(IIRFilterProcessor* processor) { secondEqProcessor = processor; }
    ~EQWindow() override;

    // DocumentWindow overrides
    void closeButtonPressed() override;
    void resized() override;
    
    // Listener overrides
    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* comboBox) override;
    void buttonClicked(juce::Button* button) override;
    void timerCallback() override;
    
    // Setter for limiter processor
    void setLimiterProcessor(LimiterProcessor* processor) { limiterProcessor = processor; }
    
    // Load/Save functionality
    void saveLimiterParams();
    void loadLimiterParams();
    void resetParamsToDefaults();
    
    // Callback for close button
    std::function<void()> onCloseButtonPressed;
    
    // Inner content class
    class EQContent : public juce::Component
    {
    public:
        EQContent(EQWindow* parent);
        ~EQContent();
        
        void paint(juce::Graphics& g) override;
        void resized() override;

        // Components
        juce::OwnedArray<EQBand> eqBands;
        std::unique_ptr<FrequencyResponseDisplay> frequencyDisplay;
        std::unique_ptr<LimiterSection> rmsLimiter;
        std::unique_ptr<LimiterSection> peakLimiter;
        
        // Buttons
        juce::TextButton loadButton;
        juce::TextButton saveButton;
        juce::TextButton resetButton;

    private:
        EQWindow* parentWindow;
        
        void setupEQBands();
        void setupLimiters();
        void setupButtons();
    };

    // Public member for access from FrequencyResponseDisplay
    std::unique_ptr<EQContent> content;

private:
    // Processors
    IIRFilterProcessor* eqProcessor;
    IIRFilterProcessor* secondEqProcessor = nullptr; // mirror updates to right channel
    LimiterProcessor* limiterProcessor = nullptr;
    
    // Viewport for scrolling
    std::unique_ptr<juce::Viewport> viewport;
    
    // Window size constants
    static constexpr int MINIMUM_CONTENT_WIDTH = 1100;
    static constexpr int MINIMUM_CONTENT_HEIGHT = 850;
    
    // Helper methods
    void updateEQProcessor();
    void updateLimiterProcessor();
    void updateDisplay();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EQWindow)
};