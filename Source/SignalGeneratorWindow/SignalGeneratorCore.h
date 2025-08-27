#pragma once

#include <JuceHeader.h>
#include "AudioEngine.h"

// All constants and configurations are centrally managed
namespace SignalGenConstants {
    // Window dimensions
    constexpr int WINDOW_WIDTH = 760;
    constexpr int WINDOW_HEIGHT = 620;
    
    // Default values
    constexpr float DEFAULT_FREQUENCY = 1000.0f;
    constexpr float DEFAULT_LEVEL_DB = -17.0f;
    constexpr float DEFAULT_DUTY_CYCLE = 0.5f;
    constexpr float DEFAULT_SWEEP_SPEED = 45.0f;
    
    // Frequency range
    constexpr float MIN_FREQUENCY = 20.0f;
    constexpr float MAX_FREQUENCY = 20000.0f;
    constexpr float FREQUENCY_SKEW_MIDPOINT = 1000.0f;
    
    // Level range
    constexpr float MIN_LEVEL_DB = -60.0f;
    constexpr float MAX_LEVEL_DB = 0.0f;
    constexpr float LEVEL_STEP_DB = 0.1f;
    
    // Duty cycle range
    constexpr float MIN_DUTY_CYCLE = 0.1f;
    constexpr float MAX_DUTY_CYCLE = 0.9f;
    constexpr float DUTY_CYCLE_STEP = 0.01f;
    
    // Sweep speed range
    constexpr float MIN_SWEEP_SPEED = 0.0f;
    constexpr float MAX_SWEEP_SPEED = 100.0f;
    
    // UI element dimensions
    constexpr int METER_WIDTH = 28;
    constexpr int METER_SPACING = 26;
    constexpr int GROUP_MARGIN = 10;
    constexpr int GROUP_HEADER_HEIGHT = 25;
    constexpr int BUTTON_HEIGHT = 30;
    constexpr int SLIDER_TEXT_BOX_WIDTH = 60;
    constexpr int SLIDER_TEXT_BOX_HEIGHT = 20;
    
    // Component heights
    constexpr int FREQUENCY_RANGE_GROUP_HEIGHT = 180;
    constexpr int WAVEFORM_GROUP_HEIGHT = 140;
    constexpr int DUTY_CYCLE_GROUP_HEIGHT = 60;
    constexpr int MANUAL_FREQUENCY_GROUP_MIN_HEIGHT = 150;
    constexpr int SWEEP_MODE_GROUP_HEIGHT = 100;
    constexpr int SWEEP_SPEED_GROUP_HEIGHT = 275;
    constexpr int OUTPUT_LEVEL_GROUP_HEIGHT = 420;
    constexpr int CHANNELS_GROUP_HEIGHT = 140;
    
    // Radio button group IDs
    enum RadioGroupIds {
        FREQ_RANGE_GROUP_ID = 1,
        WAVEFORM_GROUP_ID = 2,
        SWEEP_MODE_GROUP_ID = 3,
        SWEEP_SPEED_GROUP_ID = 4,
        CHANNEL_GROUP_ID = 5
    };
    
    // Preset frequency ranges
    struct FrequencyPreset {
        float startFreq;
        float endFreq;
        const char* displayName;
        const char* startText;
        const char* endText;
    };
    
    constexpr FrequencyPreset FREQUENCY_PRESETS[] = {
        {20.0f, 20000.0f, "Wide (20 Hz .. 20 kHz)", "20", "20000"},
        {1000.0f, 15000.0f, "HF (1 kHz .. 15 kHz)", "1000", "15000"},
        {300.0f, 3000.0f, "Speech (300 Hz .. 3 kHz)", "300", "3000"},
        {50.0f, 1000.0f, "LF (50 Hz .. 1 kHz)", "50", "1000"}
    };
}

// Core business logic class
class SignalGeneratorCore {
public:
    SignalGeneratorCore(SignalGenAudioEngine* engine);
    
    // Frequency related
    void setFrequencyRange(float start, float end);
    void setManualFrequency(float left, float right);
    void setLockFrequencies(bool lock);
    float getGeometricMean(float start, float end) const;
    float clampFrequency(float freq) const;
    
    // Level related
    void setOutputLevel(float left, float right);
    void setLockLevels(bool lock);
    void setChannelEnabled(bool leftEnabled, bool rightEnabled);
    
    // Waveform related
    void setWaveform(SignalGenAudioEngine::Waveform waveform);
    void setDutyCycle(float dutyCycle);
    
    // Sweep related
    void setSweepMode(SignalGenAudioEngine::SweepMode mode);
    void setSweepSpeed(SignalGenAudioEngine::SweepSpeed speed);
    void setSweepSpeedValue(float value);
    
    // Channel related
    void setChannelMode(SignalGenAudioEngine::ChannelMode mode);
    
    // Control
    void start();
    void stop();
    bool isRunning() const;
    
    // Get status
    bool areFrequenciesLocked() const { return lockFrequencies; }
    bool areLevelsLocked() const { return lockLevels; }
    
private:
    SignalGenAudioEngine* audioEngine;
    bool lockFrequencies = true;
    bool lockLevels = true;
    
    float lastLeftFrequency = SignalGenConstants::DEFAULT_FREQUENCY;
    float lastRightFrequency = SignalGenConstants::DEFAULT_FREQUENCY;
    float lastLeftLevel = SignalGenConstants::DEFAULT_LEVEL_DB;
    float lastRightLevel = SignalGenConstants::DEFAULT_LEVEL_DB;
};

// Layout helper class
class LayoutHelper {
public:
    // Vertically layout buttons within a group
    static void layoutVerticalButtons(juce::GroupComponent& group, 
                                     const std::vector<juce::Component*>& buttons,
                                     int margin = SignalGenConstants::GROUP_MARGIN);
    
    // Horizontally layout buttons within a group
    static void layoutHorizontalButtons(juce::GroupComponent& group,
                                       const std::vector<juce::Component*>& buttons,
                                       int margin = SignalGenConstants::GROUP_MARGIN);
    
    // Create FlexBox layout
    static juce::FlexBox createFlexBox(juce::FlexBox::Direction direction,
                                       juce::FlexBox::JustifyContent justifyContent = juce::FlexBox::JustifyContent::spaceAround);
    
    // Add components to FlexBox
    static void addToFlexBox(juce::FlexBox& flexBox, 
                            const std::vector<juce::Component*>& components,
                            float flex = 1.0f);
    
    // Perform FlexBox layout
    static void performLayout(juce::FlexBox& flexBox, juce::Rectangle<int> bounds);
};

// UI component factory class
class ComponentFactory {
public:
    // Create radio button group
    static std::vector<std::unique_ptr<juce::ToggleButton>> createRadioButtonGroup(
        const std::vector<juce::String>& buttonNames,
        int radioGroupId);
    
    // Create and configure slider
    static std::unique_ptr<juce::Slider> createSlider(
        juce::Slider::SliderStyle style,
        double minValue, double maxValue, double defaultValue,
        double step = 0.0,
        juce::Slider::TextEntryBoxPosition textBoxPosition = juce::Slider::NoTextBox,
        bool readOnly = false,
        int textBoxWidth = SignalGenConstants::SLIDER_TEXT_BOX_WIDTH,
        int textBoxHeight = SignalGenConstants::SLIDER_TEXT_BOX_HEIGHT);
    
    // Create component group
    static std::unique_ptr<juce::GroupComponent> createGroup(const juce::String& title);
    
    // Create label
    static std::unique_ptr<juce::Label> createLabel(const juce::String& text,
                                                    juce::Justification justification = juce::Justification::centred);
    
    // Create text editor
    static std::unique_ptr<juce::TextEditor> createNumericEditor(const juce::String& defaultText,
                                                                 int maxLength = 7);
    
    // Configure button appearance
    static void styleButton(juce::Button& button, 
                           const juce::Colour& colour = juce::Colours::transparentBlack,
                           bool isMomentary = false);
};

// Custom level meter look and feel
class MeterLookAndFeel : public juce::LookAndFeel_V4 {
public:
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float minSliderPos, float sliderMaxPos,
                         const juce::Slider::SliderStyle style, juce::Slider& slider) override;
private:
    void drawMeterBar(juce::Graphics& g, const juce::Rectangle<float>& bounds, float proportion);
};
