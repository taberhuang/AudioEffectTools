#pragma once

#include <JuceHeader.h>
#include "AudioEngine.h"
#include "SignalGeneratorCore.h"

class SignalGeneratorComponent : public juce::AudioAppComponent,
                                 public juce::Button::Listener,
                                 public juce::Slider::Listener,
                                 public juce::ComboBox::Listener
{
public:
    explicit SignalGeneratorComponent(SignalGenAudioEngine* sharedEngine = nullptr, 
                                     bool ownsAudioDevice = true);
    ~SignalGeneratorComponent() override;

    // AudioAppComponent
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    // Component
    void paint(juce::Graphics& g) override;
    void resized() override;

    // Listeners
    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* comboBox) override;

private:
    // Core components
    SignalGenAudioEngine* audioEngine = nullptr;
    std::unique_ptr<SignalGeneratorCore> core;
    MeterLookAndFeel meterLookAndFeel;
    // Whether to own the audio device and engine lifecycle
    bool ownsAudioDevice = true;
    bool ownsEngine = false;

    // ========== UI Component Organization ==========
    
    // Frequency range control
    struct FrequencyRangePanel {
        juce::GroupComponent group;
        juce::ToggleButton wideRangeButton{"Wide (20 Hz .. 20 kHz)"};
        juce::ToggleButton hfRangeButton{"HF (1 kHz .. 15 kHz)"};
        juce::ToggleButton speechRangeButton{"Speech (300 Hz .. 3 kHz)"};
        juce::ToggleButton lfRangeButton{"LF (50 Hz .. 1 kHz)"};
        juce::ToggleButton customRangeButton{"Custom"};
        
        juce::TextEditor startFreqEditor;
        juce::TextEditor endFreqEditor;
        juce::Label toLabel{"", "to"};
        juce::Label hzLabel{"", "Hz"};
        
        std::vector<juce::ToggleButton*> getRadioButtons() {
            return {&wideRangeButton, &hfRangeButton, &speechRangeButton, 
                   &lfRangeButton, &customRangeButton};
        }
        
        std::vector<juce::Component*> getRadioButtonsAsComponents() {
            return {&wideRangeButton, &hfRangeButton, &speechRangeButton, 
                   &lfRangeButton, &customRangeButton};
        }
    } frequencyRange;

    // Waveform control
    struct WaveformPanel {
        juce::GroupComponent group;
        juce::ToggleButton sineWaveButton{"Sine"};
        juce::ToggleButton squareWaveButton{"Square"};
        juce::ToggleButton triangleWaveButton{"Triangle"};
        juce::ToggleButton sawtoothWaveButton{"Sawtooth"};
        juce::ToggleButton pulseWaveButton{"Pulse"};
        
        juce::GroupComponent dutyCycleGroup;
        juce::Slider dutyCycleSlider;
        
        std::vector<juce::ToggleButton*> getWaveformButtons() {
            return {&sineWaveButton, &squareWaveButton, &triangleWaveButton,
                   &sawtoothWaveButton, &pulseWaveButton};
        }
        
        std::vector<juce::Component*> getWaveformButtonsAsComponents() {
            return {&sineWaveButton, &squareWaveButton, &triangleWaveButton,
                   &sawtoothWaveButton, &pulseWaveButton};
        }
    } waveform;

    // Manual frequency control
    struct ManualFrequencyPanel {
        juce::GroupComponent group;
        juce::Slider leftFrequencySlider;
        juce::Slider rightFrequencySlider;
        juce::ToggleButton lockFrequencyButton{"Lock L + R frequency"};
    } manualFrequency;

    // Sweep control
    struct SweepPanel {
        juce::GroupComponent modeGroup;
        juce::ToggleButton linearSweepButton{"Linear"};
        juce::ToggleButton logSweepButton{"Log"};
        
        juce::GroupComponent speedGroup;
        juce::ToggleButton fastSmoothButton{"Fast (smooth)"};
        juce::ToggleButton fastSteppedButton{"Fast (stepped)"};
        juce::ToggleButton slowButton{"Slow"};
        juce::ToggleButton manualButton{"Manual"};
        juce::ToggleButton whiteNoiseButton{"White noise"};
        juce::ToggleButton pinkNoiseButton{"Pink noise"};
        juce::ToggleButton noSweepButton{"No sweep"};
        juce::Slider sweepSpeedSlider;
        
        std::vector<juce::ToggleButton*> getModeButtons() {
            return {&linearSweepButton, &logSweepButton};
        }
        
        std::vector<juce::ToggleButton*> getSpeedButtons() {
            return {&fastSmoothButton, &fastSteppedButton, &slowButton,
                   &manualButton, &whiteNoiseButton, &pinkNoiseButton, &noSweepButton};
        }
        
        std::vector<juce::Component*> getModeButtonsAsComponents() {
            return {&linearSweepButton, &logSweepButton};
        }
        
        std::vector<juce::Component*> getSpeedButtonsAsComponents() {
            return {&fastSmoothButton, &fastSteppedButton, &slowButton,
                   &manualButton, &whiteNoiseButton, &pinkNoiseButton, &noSweepButton};
        }
    } sweep;

    // Output level control
    struct OutputLevelPanel {
        juce::GroupComponent group;
        juce::Slider leftChannelSlider;
        juce::Slider rightChannelSlider;
        juce::ToggleButton leftChannelToggle{"L"};
        juce::ToggleButton rightChannelToggle{"R"};
        juce::ToggleButton lockOutputLevelButton{"Lock L + R output level"};
        juce::Label leftLevelValueLabel;
        juce::Label rightLevelValueLabel;
        juce::Label dbLabel{"", "dB"};
        std::vector<std::unique_ptr<juce::Label>> levelLabels;
    } outputLevel;

    // Channel control
    struct ChannelPanel {
        juce::GroupComponent group;
        juce::ToggleButton inPhaseButton{"In phase"};
        juce::ToggleButton phase180Button{juce::CharPointer_UTF8("180\xc2\xb0 phase")};
        juce::ToggleButton independentButton{"Independent"};
        
        std::vector<juce::ToggleButton*> getButtons() {
            return {&inPhaseButton, &phase180Button, &independentButton};
        }
        
        std::vector<juce::Component*> getButtonsAsComponents() {
            return {&inPhaseButton, &phase180Button, &independentButton};
        }
    } channels;

    // Control buttons
    juce::TextButton startButton{"Start"};
    juce::TextButton closeButton{"Close"};

    // ========== Private Methods ==========
    
    // Initialization methods
    void setupUI();
    void setupRadioGroups();
    void setupListeners();
    void setDefaultValues();
    
    // Update methods
    void updateFrequencyRange();
    void updateWaveform();
    void updateSweepMode();
    void updateSweepSpeed();
    void updateChannels();
    void updateLevelValueLabels();
    
    // Layout methods
    void layoutMainColumns(juce::Rectangle<int> bounds);
    void layoutFrequencyControls(juce::Rectangle<int> bounds);
    void layoutWaveformControls(juce::Rectangle<int> bounds);
    void layoutSweepControls(juce::Rectangle<int> bounds);
    void layoutOutputControls(juce::Rectangle<int> bounds);
    void layoutChannelControls(juce::Rectangle<int> bounds);
    
    // Helper methods
    void handleFrequencyRangeSelection();
    void handleCustomFrequencyRange();
    void syncFrequencySliders(juce::Slider* source);
    void syncLevelSliders(juce::Slider* source);
};