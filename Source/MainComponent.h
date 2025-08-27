#pragma once

#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>
#include "SpectrumAnalyzer.h"
#include "WaveformAnalyzer.h"
#include "AudioRecorder.h"
#include "EQWindow/EQWindow.h"
#include "EffectEmbeded/effect_delay.h"
#include "EffectEmbeded/effect_cathedral_reverb.h"
#include "EffectJUCE/IIRFilterProcessor.h"
#include "EffectJUCE/LimiterProcessor.h" 
#include "SweepTestWindow/SweepTestGenerator.h"
#include "SweepTestWindow/SweepTestAnalyzer.h"
#include "SignalGeneratorWindow/SignalGenWindow.h"
#include "EffectExternal/NoiseSuppressionProcessor.h"

class MainComponent : public juce::AudioAppComponent,
    public juce::Slider::Listener,
    public juce::ComboBox::Listener,
    public juce::ToggleButton::Listener
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* comboBox) override;
    void buttonClicked(juce::Button* button) override; // Added for ToggleButton::Listener
    void InputEqProcess(float In1Xn, float* pIn1Yn, IIRFilterProcessor* channelProcessor);
    void handleCommandMessage(int commandId) override;
private:
    // UI controls
    // Effect selection
    juce::ComboBox effectSelector;
    juce::Label effectSelectorLabel;
    juce::ToggleButton effectEnableToggle;
    
    // Effect parameters
    juce::Slider gainSlider, delaySlider, feedbackSlider, flangerRateSlider, flangerDepthSlider;
    juce::Label gainLabel, delayLabel, feedbackLabel, flangerRateLabel, flangerDepthLabel;
    
    // Plate reverb parameters
    juce::Slider plateDecaySlider, plateToneSlider;
    juce::Label plateDecayLabel, plateToneLabel;
    
    // Pitch shift parameters
    juce::Slider pitchSemitoneSlider, pitchDetuneSlider;
    juce::Label pitchSemitoneLabel, pitchDetuneLabel;
    
    // Cathedral reverb parameters
    juce::Slider cathedralDecaySlider, cathedralToneSlider;
    juce::Label cathedralDecayLabel, cathedralToneLabel;
    
    // JUCE Delay parameters
    juce::Slider juceDelayTimeSlider, juceDelayFeedbackSlider, juceDelayMixSlider;
    juce::Label juceDelayTimeLabel, juceDelayFeedbackLabel, juceDelayMixLabel;
    
    // JUCE Chorus parameters
    juce::Slider juceChorusRateSlider, juceChorusDepthSlider, juceChorusCentreDelaySlider, juceChorusFeedbackSlider, juceChorusMixSlider;
    juce::Label juceChorusRateLabel, juceChorusDepthLabel, juceChorusCentreDelayLabel, juceChorusFeedbackLabel, juceChorusMixLabel;
    
    // JUCE Reverb parameters
    juce::Slider juceReverbRoomSizeSlider, juceReverbDampingSlider, juceReverbWetLevelSlider, juceReverbDryLevelSlider, juceReverbWidthSlider;
    juce::Label juceReverbRoomSizeLabel, juceReverbDampingLabel, juceReverbWetLevelLabel, juceReverbDryLevelLabel, juceReverbWidthLabel;
    
    // JUCE Phaser parameters
    juce::Slider jucePhaserRateSlider, jucePhaserDepthSlider, jucePhaserCentreFreqSlider, jucePhaserFeedbackSlider, jucePhaserMixSlider;
    juce::Label jucePhaserRateLabel, jucePhaserDepthLabel, jucePhaserCentreFreqLabel, jucePhaserFeedbackLabel, jucePhaserMixLabel;
    
    // JUCE Compressor parameters
    juce::Slider juceCompressorThresholdSlider, juceCompressorRatioSlider, juceCompressorAttackSlider, juceCompressorReleaseSlider;
    juce::Label juceCompressorThresholdLabel, juceCompressorRatioLabel, juceCompressorAttackLabel, juceCompressorReleaseLabel;
    
    // JUCE Distortion parameters
    // JUCE Flanger parameters
    juce::Slider juceFlangerRateSlider, juceFlangerDepthSlider, juceFlangerCentreDelaySlider, juceFlangerFeedbackSlider, juceFlangerMixSlider;
    juce::Label juceFlangerRateLabel, juceFlangerDepthLabel, juceFlangerCentreDelayLabel, juceFlangerFeedbackLabel, juceFlangerMixLabel;
    
    // JUCE Pitch Shift parameters
    juce::Slider jucePitchShiftSlider;
    juce::Label jucePitchShiftLabel;
    
    // JUCE Plate Reverb parameters
    juce::Slider jucePlateReverbRoomSizeSlider, jucePlateReverbDampingSlider, jucePlateReverbWetLevelSlider;
    juce::Label jucePlateReverbRoomSizeLabel, jucePlateReverbDampingLabel, jucePlateReverbWetLevelLabel;
    
    // JUCE Distortion parameters
    juce::Slider juceDistortionDriveSlider, juceDistortionRangeSlider, juceDistortionBlendSlider;
    juce::Label juceDistortionDriveLabel, juceDistortionRangeLabel, juceDistortionBlendLabel;

    // Noise Suppression parameters
    juce::Slider noiseSuppressionVadThresholdSlider, noiseSuppressionVadGracePeriodSlider, noiseSuppressionVadRetroactiveGracePeriodSlider;
    juce::Label noiseSuppressionVadThresholdLabel, noiseSuppressionVadGracePeriodLabel, noiseSuppressionVadRetroactiveGracePeriodLabel;

    juce::ToggleButton bypassToggle;
    juce::TextButton recordButton;
    juce::TextButton eqButton;
    juce::ComboBox inputDeviceSelector, outputDeviceSelector;
    juce::Label inputDeviceLabel, outputDeviceLabel;

    // Spectrum analyzers
    SpectrumAnalyzer inputSpectrumAnalyzer;  // Input signal spectrum analyzer
    SpectrumAnalyzer outputSpectrumAnalyzer; // Output signal spectrum analyzer
    juce::ToggleButton showInputSpectrumToggle;
    juce::ToggleButton showOutputSpectrumToggle;
    juce::Label inputSpectrumLabel, outputSpectrumLabel;

    // Waveform analyzers
    WaveformAnalyzer inputWaveformAnalyzer;  // Input signal waveform analyzer
    WaveformAnalyzer outputWaveformAnalyzer; // Output signal waveform analyzer
    juce::ToggleButton showInputWaveformToggle;
    juce::ToggleButton showOutputWaveformToggle;
    juce::Label inputWaveformLabel, outputWaveformLabel;

    juce::StringArray availableInputDevices, availableOutputDevices;

    // Audio processing
    float gain = 1.0f;
    float delayTime = 0.5f;
    float feedbackGain = 0.5f;
    float flangerRate = 0.5f;
    float flangerDepth = 0.5f;
    float plateDecay = 0.5f;
    float plateTone = 0.5f;
    float pitchSemitone = 0.5f;
    float pitchDetune = 0.5f;
    float cathedralDecay = 0.5f;
    float cathedralTone = 0.5f;
    
    // JUCE effect parameters
    float juceDelayTime = 0.5f;
    float juceDelayFeedback = 0.3f;
    float juceDelayMix = 0.5f;
    
    float juceChorusRate = 1.0f;
    float juceChorusDepth = 0.25f;
    float juceChorusCentreDelay = 7.0f;
    float juceChorusFeedback = 0.0f;
    float juceChorusMix = 0.5f;
    
    float juceReverbRoomSize = 0.5f;
    float juceReverbDamping = 0.5f;
    float juceReverbWetLevel = 0.33f;
    float juceReverbDryLevel = 0.4f;
    float juceReverbWidth = 1.0f;
    
    float jucePhaserRate = 1.0f;
    float jucePhaserDepth = 1.0f;
    float jucePhaserCentreFreq = 1300.0f;
    float jucePhaserFeedback = 0.0f;
    float jucePhaserMix = 0.5f;
    
    float juceCompressorThreshold = -10.0f;
    float juceCompressorRatio = 4.0f;
    float juceCompressorAttack = 2.0f;
    float juceCompressorRelease = 100.0f;
    
    float juceFlangerRate = 1.0f;
    float juceFlangerDepth = 0.25f;
    float juceFlangerCentreDelay = 7.0f;
    float juceFlangerFeedback = 0.0f;
    float juceFlangerMix = 0.5f;
    
    float jucePitchShift = 0.0f; // In semitones
    
    float jucePlateReverbRoomSize = 0.5f;
    float jucePlateReverbDamping = 0.5f;
    float jucePlateReverbWetLevel = 0.33f;
    
    float juceDistortionDrive = 0.5f;
    float juceDistortionRange = 0.5f;
    float juceDistortionBlend = 0.5f;

    // Noise Suppression parameters
    float noiseSuppressionVadThreshold = 0.6f;
    int noiseSuppressionVadGracePeriod = 20;
    int noiseSuppressionVadRetroactiveGracePeriod = 0;

    double currentSampleRate = 44100.0;
    
    // Current effect
    enum EffectType {
        Delay, Flanger, PlateReverb, PitchShift, CathedralReverb,
        JUCEDelay, JUCEFlanger, JUCEPitchShift, JUCEPlateReverb, JUCEChorus, JUCEReverb, JUCEPhaser, JUCECompressor, JUCEDistortion, NoiseSuppression
    };
    EffectType currentEffect = Delay;

    // JUCE DSP effects
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> juceDelay;
    juce::dsp::Chorus<float> juceFlanger;        // Used for flanger effect
    juce::dsp::Chorus<float> juceChorus;         // Used for chorus effect  
    juce::dsp::Reverb jucePlateReverb;           // Used for plate reverb effect
    juce::dsp::Reverb juceReverb;                // Used for standard reverb effect
    juce::dsp::Phaser<float> jucePhaser;
    juce::dsp::Compressor<float> juceCompressor;
    juce::dsp::WaveShaper<float> juceDistortion;

    // Noise Suppression processor
    std::unique_ptr<NoiseSuppressionProcessor> noiseSuppressionProcessor;

    // DSP context
    juce::dsp::ProcessSpec spec;
    juce::dsp::AudioBlock<float> audioBlock;
    juce::HeapBlock<char> heapBlock;

    // Bypass related
    bool isBypassed = false;
    bool isEffectEnabled = true;

    // Recording related
    std::unique_ptr<AudioRecorder> audioRecorder;
    
    // EQ related
    std::unique_ptr<IIRFilterProcessor> eqProcessorL; // left channel EQ
    std::unique_ptr<IIRFilterProcessor> eqProcessorR; // right channel EQ
    std::unique_ptr<EQWindow> eqWindow;
    std::unique_ptr<LimiterProcessor> limiterProcessor;

    // Sweep test components
    juce::TextButton sweepTestButton;
    std::unique_ptr<SweepTestGenerator> sweepGenerator;
    std::unique_ptr<juce::AudioFormatWriter> sweepTestWriter;
    juce::File sweepTestFile;
    juce::AudioBuffer<float> sweepTestBuffer;
    int sweepTestSampleCount = 0;
    bool isSweepTesting = false;

    // Signal generator integration
    std::unique_ptr<SignalGenWindow> signalGenWindow;
    std::unique_ptr<SignalGenAudioEngine> signalGenEngine;
    bool useInternalSignalGen = false;
    juce::AudioBuffer<float> internalSignalBuffer;
    juce::String previousInputDeviceName; // remember hardware input before entering internal generator


    const float maxDelayTime = 2.0f; // seconds

    void updateDeviceLists();
    void updateEffectControls();
    void hideAllEffectControls();
    void configureSlider(juce::Slider& slider, juce::Label& label, const juce::String& labelText, 
                        double minValue, double maxValue, double initialValue);
    void processEffectStereo(float leftInput, float rightInput, float& leftOutput, float& rightOutput);
    void logParameterUpdate(const juce::String& parameterName, float value);
    
    // JUCE effect processing functions (mono)
    float processJUCEDelay(float inputSample);
    float processJUCEFlanger(float inputSample);
    float processJUCEPitchShift(float inputSample);
    float processJUCEPlateReverb(float inputSample);
    float processJUCEChorus(float inputSample);
    float processJUCEReverb(float inputSample);
    float processJUCEPhaser(float inputSample);
    float processJUCECompressor(float inputSample);
    float processJUCEDistortion(float inputSample);
    
    // JUCE effect processing functions (stereo)
    void processJUCEDelayStereo(float leftInput, float rightInput, float& leftOutput, float& rightOutput);
    void processJUCEFlangerStereo(float leftInput, float rightInput, float& leftOutput, float& rightOutput);
    void processJUCEPitchShiftStereo(float leftInput, float rightInput, float& leftOutput, float& rightOutput);
    void processJUCEPlateReverbStereo(float leftInput, float rightInput, float& leftOutput, float& rightOutput);
    void processJUCEChorusStereo(float leftInput, float rightInput, float& leftOutput, float& rightOutput);
    void processJUCEReverbStereo(float leftInput, float rightInput, float& leftOutput, float& rightOutput);
    void processJUCEPhaserStereo(float leftInput, float rightInput, float& leftOutput, float& rightOutput);
    void processJUCECompressorStereo(float leftInput, float rightInput, float& leftOutput, float& rightOutput);
    void processJUCEDistortionStereo(float leftInput, float rightInput, float& leftOutput, float& rightOutput);
    
    // JUCE effect parameter update functions
    void updateJUCEDelayParameters();
    void updateJUCEFlangerParameters();
    void updateJUCEPitchShiftParameters();
    void updateJUCEPlateReverbParameters();
    void updateJUCEChorusParameters();
    void updateJUCEReverbParameters();
    void updateJUCEPhaserParameters();
    void updateJUCECompressorParameters();
    void updateJUCEDistortionParameters();
    
    // Sweep test methods
    void startSweepTest();
    void stopSweepTest();
    void processSweepTestAudio(float inputSample, float outputSample);
    void finalizeSweepTest();
    void openEQWindow();
    void closeEQWindow();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};