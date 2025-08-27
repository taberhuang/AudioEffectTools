// EQControls.h
#pragma once
#include <JuceHeader.h>

// Custom LookAndFeel for sliders - Uses singleton pattern
class CustomSliderLookAndFeel : public juce::LookAndFeel_V4
{
public:
    static CustomSliderLookAndFeel& getInstance()
    {
        static CustomSliderLookAndFeel instance;
        return instance;
    }
    
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float minSliderPos, float maxSliderPos,
                         const juce::Slider::SliderStyle style, juce::Slider& slider) override;
    
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                         const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override;
    
    void drawTextEditorOutline(juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override;
    
    int getSliderThumbRadius(juce::Slider& slider) override;

private:
    CustomSliderLookAndFeel();
    ~CustomSliderLookAndFeel() = default;
    
    // Prevent copying
    CustomSliderLookAndFeel(const CustomSliderLookAndFeel&) = delete;
    CustomSliderLookAndFeel& operator=(const CustomSliderLookAndFeel&) = delete;
};

// EQ Band control structure
struct EQBand
{
    juce::ToggleButton enableButton;
    juce::Label typeLabel;
    juce::Label freqLabel;
    juce::Label gainLabel;
    juce::Label qLabel;
    juce::ComboBox typeCombo;
    juce::Slider freqSlider;
    juce::Slider gainSlider;
    juce::Slider qSlider;
    juce::TextEditor freqText;
    juce::TextEditor gainText;
    juce::TextEditor qText;
    
    EQBand();
    ~EQBand();
    void setupCallbacks();
};

// Limiter Section control structure
struct LimiterSection
{
    juce::Label titleLabel;
    juce::ToggleButton enableButton;
    juce::Label thLabel;
    juce::Label ratioLabel;
    juce::Label atLabel;
    juce::Label rtLabel;
    juce::Slider thSlider;
    juce::Slider ratioSlider;
    juce::Slider atSlider;
    juce::Slider rtSlider;
    juce::TextEditor thText;
    juce::TextEditor ratioText;
    juce::TextEditor atText;
    juce::TextEditor rtText;
    
    explicit LimiterSection(const juce::String& title);
    ~LimiterSection();
    void setupCallbacks();
    void setLinearSliders();
};