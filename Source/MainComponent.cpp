#include "MainComponent.h"
#include <JuceHeader.h> 
#include <cmath>
#include <cstdint>


// Constructor
MainComponent::MainComponent()
{
    setAudioChannels(2, 2);

    // Configure gain slider
    configureSlider(gainSlider, gainLabel, "Gain", 0.0, 2.0, 1.0);

    // Configure Bypass switch
    bypassToggle.setButtonText("Bypass");
    bypassToggle.addListener(this);
    addAndMakeVisible(bypassToggle);

    // Configure recording button
    recordButton.setButtonText("Start Recording");
    recordButton.addListener(this);
    addAndMakeVisible(recordButton);

    // Configure EQ button
    eqButton.setButtonText("EQ");
    eqButton.addListener(this);
    addAndMakeVisible(eqButton);

    // Configure sweep test button
    sweepTestButton.setButtonText("Sweep Test");
    sweepTestButton.addListener(this);
    addAndMakeVisible(sweepTestButton);

    // Create shared signal generator engine (for internal source)
    signalGenEngine = std::make_unique<SignalGenAudioEngine>();

    // Configure delay and feedback sliders
    configureSlider(delaySlider, delayLabel, "Delay", 0.0, maxDelayTime, 0.5);
    configureSlider(feedbackSlider, feedbackLabel, "Feedback", 0.0, 0.95, 0.5);

    // Configure spectrum analyzer
    addAndMakeVisible(inputSpectrumAnalyzer);
    addAndMakeVisible(outputSpectrumAnalyzer);
    
    showInputSpectrumToggle.setButtonText("Show Input Spectrum");
    showInputSpectrumToggle.setToggleState(true, juce::dontSendNotification);
    showInputSpectrumToggle.addListener(this);
    addAndMakeVisible(showInputSpectrumToggle);
    
    showOutputSpectrumToggle.setButtonText("Show Output Spectrum");
    showOutputSpectrumToggle.setToggleState(true, juce::dontSendNotification);
    showOutputSpectrumToggle.addListener(this);
    addAndMakeVisible(showOutputSpectrumToggle);
    
    inputSpectrumLabel.setText("Input Spectrum", juce::dontSendNotification);
    outputSpectrumLabel.setText("Output Spectrum", juce::dontSendNotification);
    addAndMakeVisible(inputSpectrumLabel);
    addAndMakeVisible(outputSpectrumLabel);

    // Configure waveform analyzer
    addAndMakeVisible(inputWaveformAnalyzer);
    addAndMakeVisible(outputWaveformAnalyzer);
    
    showInputWaveformToggle.setButtonText("Show Input Waveform");
    showInputWaveformToggle.setToggleState(true, juce::dontSendNotification);
    showInputWaveformToggle.addListener(this);
    addAndMakeVisible(showInputWaveformToggle);
    
    showOutputWaveformToggle.setButtonText("Show Output Waveform");
    showOutputWaveformToggle.setToggleState(true, juce::dontSendNotification);
    showOutputWaveformToggle.addListener(this);
    addAndMakeVisible(showOutputWaveformToggle);
    
    inputWaveformLabel.setText("Input Waveform", juce::dontSendNotification);
    outputWaveformLabel.setText("Output Waveform", juce::dontSendNotification);
    addAndMakeVisible(inputWaveformLabel);
    addAndMakeVisible(outputWaveformLabel);

    // Configure device selectors
    inputDeviceSelector.addListener(this);
    outputDeviceSelector.addListener(this);
    addAndMakeVisible(inputDeviceSelector);
    addAndMakeVisible(outputDeviceSelector);

    inputDeviceLabel.setText("Input Device", juce::dontSendNotification);
    outputDeviceLabel.setText("Output Device", juce::dontSendNotification);
    addAndMakeVisible(inputDeviceLabel);
    addAndMakeVisible(outputDeviceLabel);

    updateDeviceLists();
    setSize(1600, 950);

    // Configure effect selector with JUCE effects
    effectSelector.addItem("Delay", 1);
    effectSelector.addItem("Flanger", 2);
    effectSelector.addItem("Plate Reverb", 3);
    effectSelector.addItem("Pitch Shift", 4);
    effectSelector.addItem("Cathedral Reverb", 5);
    effectSelector.addItem("JUCE Delay", 6);
    effectSelector.addItem("JUCE Chorus", 7);
    effectSelector.addItem("JUCE Reverb", 8);
    effectSelector.addItem("JUCE Phaser", 9);
    effectSelector.addItem("JUCE Compressor", 10);
    effectSelector.addItem("JUCE Distortion", 11);
    effectSelector.addItem("Noise Suppression", 12);
    effectSelector.setSelectedId(1, juce::dontSendNotification);
    effectSelector.addListener(this);
    addAndMakeVisible(effectSelector);
    effectSelectorLabel.setText("Effect", juce::dontSendNotification);
    effectSelectorLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(effectSelectorLabel);
    
    // Configure effect enable toggle
    effectEnableToggle.setButtonText("Enable Effect");
    effectEnableToggle.setToggleState(true, juce::dontSendNotification);
    effectEnableToggle.addListener(this);
    addAndMakeVisible(effectEnableToggle);
    
    // Configure original effect parameter sliders
    configureSlider(flangerRateSlider, flangerRateLabel, "Rate", 0.0, 1.0, 0.5);
    configureSlider(flangerDepthSlider, flangerDepthLabel, "Depth", 0.0, 1.0, 0.5);
    configureSlider(plateDecaySlider, plateDecayLabel, "Decay", 0.0, 1.0, 0.5);
    configureSlider(plateToneSlider, plateToneLabel, "Tone", 0.0, 1.0, 0.5);
    configureSlider(pitchSemitoneSlider, pitchSemitoneLabel, "Semitone", 0.0, 1.0, 0.5);
    configureSlider(pitchDetuneSlider, pitchDetuneLabel, "Detune", 0.0, 1.0, 0.5);
    
    // Cathedral reverb controls
    configureSlider(cathedralDecaySlider, cathedralDecayLabel, "Decay", 0.0, 1.0, 0.5);
    configureSlider(cathedralToneSlider, cathedralToneLabel, "Tone", 0.0, 1.0, 0.5);
    
    // Configure JUCE effect parameter sliders
    configureSlider(juceDelayTimeSlider, juceDelayTimeLabel, "Delay Time", 0.0, 2000.0, 500.0);
    configureSlider(juceDelayFeedbackSlider, juceDelayFeedbackLabel, "Feedback", 0.0, 0.95, 0.3);
    configureSlider(juceDelayMixSlider, juceDelayMixLabel, "Mix", 0.0, 1.0, 0.5);
    
    configureSlider(juceChorusRateSlider, juceChorusRateLabel, "Rate", 0.1, 10.0, 1.0);
    configureSlider(juceChorusDepthSlider, juceChorusDepthLabel, "Depth", 0.0, 1.0, 0.25);
    configureSlider(juceChorusCentreDelaySlider, juceChorusCentreDelayLabel, "Centre Delay", 1.0, 50.0, 7.0);
    configureSlider(juceChorusFeedbackSlider, juceChorusFeedbackLabel, "Feedback", 0.0, 0.95, 0.0);
    configureSlider(juceChorusMixSlider, juceChorusMixLabel, "Mix", 0.0, 1.0, 0.5);
    
    configureSlider(juceReverbRoomSizeSlider, juceReverbRoomSizeLabel, "Room Size", 0.0, 1.0, 0.5);
    configureSlider(juceReverbDampingSlider, juceReverbDampingLabel, "Damping", 0.0, 1.0, 0.5);
    configureSlider(juceReverbWetLevelSlider, juceReverbWetLevelLabel, "Wet Level", 0.0, 1.0, 0.33);
    configureSlider(juceReverbDryLevelSlider, juceReverbDryLevelLabel, "Dry Level", 0.0, 1.0, 0.4);
    configureSlider(juceReverbWidthSlider, juceReverbWidthLabel, "Width", 0.0, 1.0, 1.0);
    
    configureSlider(jucePhaserRateSlider, jucePhaserRateLabel, "Rate", 0.1, 10.0, 1.0);
    configureSlider(jucePhaserDepthSlider, jucePhaserDepthLabel, "Depth", 0.0, 1.0, 1.0);
    configureSlider(jucePhaserCentreFreqSlider, jucePhaserCentreFreqLabel, "Centre Freq", 200.0, 2000.0, 1300.0);
    configureSlider(jucePhaserFeedbackSlider, jucePhaserFeedbackLabel, "Feedback", 0.0, 0.95, 0.0);
    configureSlider(jucePhaserMixSlider, jucePhaserMixLabel, "Mix", 0.0, 1.0, 0.5);
    
    configureSlider(juceCompressorThresholdSlider, juceCompressorThresholdLabel, "Threshold", -60.0, 0.0, -10.0);
    configureSlider(juceCompressorRatioSlider, juceCompressorRatioLabel, "Ratio", 1.0, 20.0, 4.0);
    configureSlider(juceCompressorAttackSlider, juceCompressorAttackLabel, "Attack", 0.1, 100.0, 2.0);
    configureSlider(juceCompressorReleaseSlider, juceCompressorReleaseLabel, "Release", 1.0, 1000.0, 100.0);

    // Configure Noise Suppression parameter sliders
    configureSlider(noiseSuppressionVadThresholdSlider, noiseSuppressionVadThresholdLabel, "VAD Threshold", 0.0, 1.0, 0.6);
    configureSlider(noiseSuppressionVadGracePeriodSlider, noiseSuppressionVadGracePeriodLabel, "VAD Grace Period (ms)", 0, 500, 20);
    configureSlider(noiseSuppressionVadRetroactiveGracePeriodSlider, noiseSuppressionVadRetroactiveGracePeriodLabel, "VAD Retroactive Grace (ms)", 0, 10, 0);

    configureSlider(juceDistortionDriveSlider, juceDistortionDriveLabel, "Drive", 0.0, 1.0, 0.5);
    configureSlider(juceDistortionRangeSlider, juceDistortionRangeLabel, "Range", 0.0, 1.0, 0.5);
    configureSlider(juceDistortionBlendSlider, juceDistortionBlendLabel, "Blend", 0.0, 1.0, 0.5);
    
    // Initialize audio parameters
    gain = 1.0f;
    delayTime = 0.5f;
    feedbackGain = 0.5f;
    flangerRate = 0.5f;
    flangerDepth = 0.5f;
    plateDecay = 0.5f;
    plateTone = 0.5f;
    pitchSemitone = 0.5f;
    pitchDetune = 0.5f;
    
    // Initialize JUCE effect parameters
    juceDelayTime = 500.0f;
    juceDelayFeedback = 0.3f;
    juceDelayMix = 0.5f;
    
    juceChorusRate = 1.0f;
    juceChorusDepth = 0.25f;
    juceChorusCentreDelay = 7.0f;
    juceChorusFeedback = 0.0f;
    juceChorusMix = 0.5f;
    
    juceReverbRoomSize = 0.5f;
    juceReverbDamping = 0.5f;
    juceReverbWetLevel = 0.33f;
    juceReverbDryLevel = 0.4f;
    juceReverbWidth = 1.0f;
    
    jucePhaserRate = 1.0f;
    jucePhaserDepth = 1.0f;
    jucePhaserCentreFreq = 1300.0f;
    jucePhaserFeedback = 0.0f;
    jucePhaserMix = 0.5f;
    
    juceCompressorThreshold = -10.0f;
    juceCompressorRatio = 4.0f;
    juceCompressorAttack = 2.0f;
    juceCompressorRelease = 100.0f;
    
    juceDistortionDrive = 0.5f;
    juceDistortionRange = 0.5f;
    juceDistortionBlend = 0.5f;
    
    // Initialize effect modules
    FxDelayInit(feedbackGain, delayTime);
    //FxFlangerInit(flangerRate, flangerDepth);
    //FxPlateRevbInit(plateDecay, plateTone);
    //FxPitchInit(pitchSemitone, pitchDetune);
    FxCathedralRevbInit(cathedralDecay, cathedralTone);
    
    // Initialize recorder
    audioRecorder = std::make_unique<AudioRecorder>();
    
    // Initialize EQ processors per channel
    eqProcessorL = std::make_unique<IIRFilterProcessor>();
    eqProcessorR = std::make_unique<IIRFilterProcessor>();
    limiterProcessor = std::make_unique<LimiterProcessor>();
    updateEffectControls();

     // Initialize sweep generator
    sweepGenerator = std::make_unique<SweepTestGenerator>();
}

MainComponent::~MainComponent()
{
    if (eqWindow)
    {
        eqWindow->setVisible(false);
        eqWindow->removeFromDesktop();
        eqWindow.reset();  // Explicitly release
    }
    // Ensure signal generator window and engine are cleaned up
    if (signalGenWindow)
    {
        signalGenWindow->setVisible(false);
        signalGenWindow->removeFromDesktop();
        signalGenWindow.reset();
    }
    if (signalGenEngine && signalGenEngine->isRunning())
    {
        signalGenEngine->stop();
    }
    shutdownAudio();
}

void MainComponent::updateDeviceLists()
{
    auto* type = deviceManager.getCurrentDeviceTypeObject();
    if (type == nullptr)
        return;

    juce::StringArray inputs = type->getDeviceNames(true);
    // Inject a virtual input device for internal signal generator
    if (! inputs.contains("Inside Signal Generator"))
        inputs.insert(0, "Inside Signal Generator");
    juce::StringArray outputs = type->getDeviceNames(false);

    availableInputDevices = inputs;
    availableOutputDevices = outputs;

    inputDeviceSelector.clear();
    outputDeviceSelector.clear();

    inputDeviceSelector.addItem("Select Input", -1);
    outputDeviceSelector.addItem("Select Output", -1);

    for (int i = 0; i < inputs.size(); ++i)
        inputDeviceSelector.addItem(inputs[i], i + 1);

    for (int i = 0; i < outputs.size(); ++i)
        outputDeviceSelector.addItem(outputs[i], i + 1);

    if (auto* current = deviceManager.getCurrentAudioDevice())
    {
        int inIndex = inputs.indexOf(deviceManager.getAudioDeviceSetup().inputDeviceName);
        if (inIndex >= 0)
            inputDeviceSelector.setSelectedItemIndex(inIndex + 1, juce::dontSendNotification);

        int outIndex = outputs.indexOf(deviceManager.getAudioDeviceSetup().outputDeviceName);
        if (outIndex >= 0)
            outputDeviceSelector.setSelectedItemIndex(outIndex + 1, juce::dontSendNotification);
    }
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate = sampleRate;
    
    inputSpectrumAnalyzer.setSampleRate(sampleRate);
    outputSpectrumAnalyzer.setSampleRate(sampleRate);
    
    inputWaveformAnalyzer.setSampleRate(sampleRate);
    outputWaveformAnalyzer.setSampleRate(sampleRate);
    
    //FxFlangerSetSampleRate(sampleRate);

    // Initialize JUCE DSP effects
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlockExpected;
    spec.numChannels = 2;
    
    juceDelay.prepare(spec);
    juceChorus.prepare(spec);
    juceReverb.prepare(spec);
    jucePhaser.prepare(spec);
    juceCompressor.prepare(spec);
    juceDistortion.prepare(spec);
    
    updateJUCEDelayParameters();
    updateJUCEChorusParameters();
    updateJUCEReverbParameters();
    updateJUCEPhaserParameters();
    updateJUCECompressorParameters();
    updateJUCEDistortionParameters();
    
    // Initialize EQ processors with sample rate
    if (eqProcessorL)
        eqProcessorL->setSampleRate(static_cast<float>(sampleRate));
    if (eqProcessorR)
        eqProcessorR->setSampleRate(static_cast<float>(sampleRate));

    // Initialize limiter processor with sample rate
    if (limiterProcessor)
        limiterProcessor->setSampleRate(static_cast<float>(sampleRate));

    // Initialize noise suppression processor
    noiseSuppressionProcessor = std::make_unique<NoiseSuppressionProcessor>();
    noiseSuppressionProcessor->prepareToPlay(sampleRate, 2); // stereo
    noiseSuppressionProcessor->setVadThreshold(noiseSuppressionVadThreshold);
    noiseSuppressionProcessor->setVadGracePeriod(noiseSuppressionVadGracePeriod);
    noiseSuppressionProcessor->setVadRetroactiveGracePeriod(noiseSuppressionVadRetroactiveGracePeriod);

    if (sweepGenerator)
    {
        sweepGenerator->prepare(sampleRate);
    }
    if (signalGenEngine)
    {
        signalGenEngine->prepareToPlay(sampleRate, samplesPerBlockExpected);
        internalSignalBuffer.setSize(2, samplesPerBlockExpected);
        internalSignalBuffer.clear();
        // Do not auto-start; user controls via Start button in Signal Generator UI
    }
    juce::Logger::writeToLog("prepareToPlay: sampleRate=" + juce::String(sampleRate));
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    auto numInputChannels = bufferToFill.buffer->getNumChannels();
    auto numSamples = bufferToFill.numSamples;
    auto buffer = bufferToFill.buffer;

    // If using internal signal generator, render a buffer-aligned block once
    if (useInternalSignalGen && signalGenEngine)
    {
        // Ensure buffer has 2 channels and at least numSamples samples
        if (internalSignalBuffer.getNumChannels() < 2 || internalSignalBuffer.getNumSamples() < numSamples)
            internalSignalBuffer.setSize(2, numSamples, false, true, true);
        else if (internalSignalBuffer.getNumSamples() != numSamples)
            internalSignalBuffer.setSize(2, numSamples, false, true, true);

        internalSignalBuffer.clear();
        signalGenEngine->processBlock(internalSignalBuffer);
    }

    // Process samples in pairs for proper stereo handling
    for (int i = 0; i < numSamples; ++i)
    {
        float leftInput, rightInput;
        // Get input samples for left and right channels
        if (useInternalSignalGen && signalGenEngine)
        {
            leftInput = internalSignalBuffer.getSample(0, i);
            rightInput = internalSignalBuffer.getNumChannels() > 1 ? internalSignalBuffer.getSample(1, i) : leftInput;
        }
        else if (isSweepTesting && sweepGenerator && sweepGenerator->isSweeeping())
        {
            float sweepSample = sweepGenerator->getNextSample();
            leftInput = sweepSample;
            rightInput = sweepSample;
            
            // Check if sweep test has ended
            if (!sweepGenerator->isSweeeping())
            {
                // Sweep test ended, stop test
                juce::MessageManager::callAsync([this]() {
                    stopSweepTest();
                });
            }
        }
        else
        {
            leftInput = buffer->getReadPointer(0, bufferToFill.startSample)[i];
            rightInput = (numInputChannels > 1) ? buffer->getReadPointer(1, bufferToFill.startSample)[i] : leftInput;
        }
       

        // Feed spectrum and waveform analyzers (only left channel to avoid duplication)
        if (showInputSpectrumToggle.getToggleState())
        {
            inputSpectrumAnalyzer.pushNextSampleIntoFifo(leftInput);
        }
        
        if (showInputWaveformToggle.getToggleState())
        {
            inputWaveformAnalyzer.pushSample(leftInput);
        }

        float leftOutput, rightOutput;

        if (isBypassed) 
        {
            leftOutput = leftInput;
            rightOutput = rightInput;
        } 
        else 
        {
            // Process effect with proper stereo handling
            processEffectStereo(leftInput, rightInput, leftOutput, rightOutput);
            
            // Apply gain and EQ processing
            leftOutput = juce::jlimit(-1.0f, 1.0f, leftOutput * gain);
            rightOutput = juce::jlimit(-1.0f, 1.0f, rightOutput * gain);
            
            InputEqProcess(leftOutput, &leftOutput, eqProcessorL.get());
            if (numInputChannels > 1)
                InputEqProcess(rightOutput, &rightOutput, eqProcessorR.get());
        }

        if (isSweepTesting)
        {
            processSweepTestAudio(leftInput, leftOutput);
        }
        // Record audio (only once per sample pair)
        else if (audioRecorder && audioRecorder->isRecording())
        {
            audioRecorder->addSamples(leftInput, leftOutput);
        }

        // Write output samples
        buffer->getWritePointer(0, bufferToFill.startSample)[i] = leftOutput;
        if (numInputChannels > 1)
            buffer->getWritePointer(1, bufferToFill.startSample)[i] = rightOutput;

        // Feed output analyzers (only left channel to avoid duplication)
        if (showOutputSpectrumToggle.getToggleState())
        {
            outputSpectrumAnalyzer.pushNextSampleIntoFifo(leftOutput);
        }
        
        if (showOutputWaveformToggle.getToggleState())
        {
            outputWaveformAnalyzer.pushSample(leftOutput);
        }
    }
}

void MainComponent::releaseResources()
{
    // Release noise suppression processor
    if (noiseSuppressionProcessor)
    {
        noiseSuppressionProcessor->releaseResources();
        noiseSuppressionProcessor.reset();
    }
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

void MainComponent::InputEqProcess(float In1Xn, float *pIn1Yn, IIRFilterProcessor* channelProcessor)
{
    float processedSignal = In1Xn;
    
    //Process through the 7-band EQ first
    if (channelProcessor)
    {
        processedSignal = channelProcessor->processEQ(processedSignal);
    }
    
    //Then process through the limiter
    if (limiterProcessor)
    {
        processedSignal = limiterProcessor->processLimiter(processedSignal);
    }
    
    *pIn1Yn = processedSignal;
}

// Helper function to configure sliders
void MainComponent::configureSlider(juce::Slider& slider, juce::Label& label, const juce::String& labelText, 
                                   double minValue, double maxValue, double initialValue)
{
    slider.setRange(minValue, maxValue, 0.01);
    slider.setValue(initialValue);
    slider.addListener(this);
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 15);
    addAndMakeVisible(slider);
    
    label.setText(labelText, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

void MainComponent::logParameterUpdate(const juce::String& paramName, float value)
{
    juce::Logger::writeToLog(paramName + " updated to: " + juce::String(value, 3));
}

void MainComponent::startSweepTest()
{
     if (isSweepTesting)
        return;
    
    // Stop normal recording
    if (audioRecorder && audioRecorder->isRecording())
    {
        audioRecorder->stopRecording();
        recordButton.setButtonText("Start Recording");
    }
    
    // Create output file
    juce::Time now = juce::Time::getCurrentTime();
    juce::String filename = now.formatted("SweepTest_%Y%m%d_%H%M%S.wav");
    sweepTestFile = juce::File::getCurrentWorkingDirectory().getChildFile(filename);
    
    // Create audio format writer
    juce::WavAudioFormat wavFormat;
    std::unique_ptr<juce::FileOutputStream> fileStream(sweepTestFile.createOutputStream());
    
    if (fileStream != nullptr)
    {
        sweepTestWriter.reset(wavFormat.createWriterFor(fileStream.release(),
                                                        currentSampleRate,
                                                        2, // Stereo
                                                        16, // 16-bit
                                                        {},
                                                        0));
        
        if (sweepTestWriter != nullptr)
        {
            // Prepare buffer (2 seconds of recording)
            int totalSamples = static_cast<int>(currentSampleRate * 2.0);
            sweepTestBuffer.setSize(2, totalSamples);
            sweepTestBuffer.clear();
            sweepTestSampleCount = 0;
            
            // Start sweep
            sweepGenerator->setFrequencyRange(20.0f, 20000.0f);
            sweepGenerator->setDuration(2.0f);
            sweepGenerator->setAmplitude(-15.0f);
            sweepGenerator->startSweep();
            
            isSweepTesting = true;
            sweepTestButton.setButtonText("Stop Test");
            
            DBG("Sweep test started, file: " << sweepTestFile.getFullPathName());
        }
    }
    
    if (!isSweepTesting)
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                              "Sweep Test Error",
                                              "Failed to create output file");
    }
}

void MainComponent::stopSweepTest()
{
    if (!isSweepTesting)
        return;
    
    isSweepTesting = false;
    sweepTestButton.setButtonText("Sweep Test");
    
    if (sweepGenerator)
    {
        sweepGenerator->stopSweep();
    }
    
    finalizeSweepTest();
}

void MainComponent::processSweepTestAudio(float inputSample, float outputSample)
{
    if (!sweepTestWriter || !isSweepTesting)
        return;
    
    // Store input and output simultaneously
    if (sweepTestSampleCount < sweepTestBuffer.getNumSamples())
    {
        // Left channel stores input signal (sweep source)
        sweepTestBuffer.setSample(0, sweepTestSampleCount, inputSample);
        // Right channel stores output signal (processed)
        sweepTestBuffer.setSample(1, sweepTestSampleCount, outputSample);
        
        sweepTestSampleCount++;
        
        // Automatically stop when buffer is full
        if (sweepTestSampleCount >= sweepTestBuffer.getNumSamples())
        {
            juce::MessageManager::callAsync([this]() {
                stopSweepTest();
            });
        }
    }
}

void MainComponent::finalizeSweepTest()
{
    if (sweepTestWriter != nullptr && sweepTestSampleCount > 0)
    {
        // Write audio data
        sweepTestWriter->writeFromAudioSampleBuffer(sweepTestBuffer, 0, sweepTestSampleCount);
        sweepTestWriter.reset();
        
        DBG("Sweep test file saved: " << sweepTestFile.getFullPathName());
        
        // Use static function to open analyzer window
        juce::MessageManager::callAsync([this]() {
            SweepTestAnalyzer::showAnalyzer(sweepTestFile);
        });
    }
    
    // Cleanup
    sweepTestBuffer.clear();
    sweepTestSampleCount = 0;
}