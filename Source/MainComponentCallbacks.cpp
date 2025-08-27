#include "MainComponent.h"

void MainComponent::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &gainSlider)
    {
        gain = static_cast<float>(gainSlider.getValue());
    }
    else if (slider == &delaySlider)
    {
        delayTime = static_cast<float>(delaySlider.getValue());
        FxDelayParaUpdate(feedbackGain, delayTime);
        logParameterUpdate("Delay time", delayTime);
    }
    else if (slider == &feedbackSlider)
    {
        feedbackGain = static_cast<float>(feedbackSlider.getValue());
        FxDelayParaUpdate(feedbackGain, delayTime);
        logParameterUpdate("Feedback gain", feedbackGain);
    }
    else if (slider == &flangerRateSlider)
    {
        flangerRate = static_cast<float>(flangerRateSlider.getValue());
        //FxFlangerParaUpdate(flangerRate, flangerDepth);
        logParameterUpdate("Flanger rate", flangerRate);
    }
    else if (slider == &flangerDepthSlider)
    {
        flangerDepth = static_cast<float>(flangerDepthSlider.getValue());
        //FxFlangerParaUpdate(flangerRate, flangerDepth);
        logParameterUpdate("Flanger depth", flangerDepth);
    }
    else if (slider == &plateDecaySlider)
    {
        plateDecay = static_cast<float>(plateDecaySlider.getValue());
        //FxPlateRevbParaUpdate(plateDecay, plateTone);
        logParameterUpdate("Plate reverb decay", plateDecay);
    }
    else if (slider == &plateToneSlider)
    {
        plateTone = static_cast<float>(plateToneSlider.getValue());
        //FxPlateRevbParaUpdate(plateDecay, plateTone);
        logParameterUpdate("Plate reverb tone", plateTone);
    }
    else if (slider == &pitchSemitoneSlider)
    {
        pitchSemitone = static_cast<float>(pitchSemitoneSlider.getValue());
        //FxPitchParaUpdate(pitchSemitone, pitchDetune);
        logParameterUpdate("Pitch semitone", pitchSemitone);
    }
    else if (slider == &pitchDetuneSlider)
    {
        pitchDetune = static_cast<float>(pitchDetuneSlider.getValue());
        //FxPitchParaUpdate(pitchSemitone, pitchDetune);
        logParameterUpdate("Pitch detune", pitchDetune);
    }
    else if (slider == &cathedralDecaySlider)
    {
        cathedralDecay = static_cast<float>(cathedralDecaySlider.getValue());
        FxCathedralRevbParaUpdate(cathedralDecay, cathedralTone);
        logParameterUpdate("Cathedral reverb decay", cathedralDecay);
    }
    else if (slider == &cathedralToneSlider)
    {
        cathedralTone = static_cast<float>(cathedralToneSlider.getValue());
        FxCathedralRevbParaUpdate(cathedralDecay, cathedralTone);
        logParameterUpdate("Cathedral reverb tone", cathedralTone);
    }
    // JUCE effect parameter handling
    else if (slider == &juceDelayTimeSlider)
    {
        juceDelayTime = static_cast<float>(juceDelayTimeSlider.getValue());
        updateJUCEDelayParameters();
        logParameterUpdate("JUCE Delay time", juceDelayTime);
    }
    else if (slider == &juceDelayFeedbackSlider)
    {
        juceDelayFeedback = static_cast<float>(juceDelayFeedbackSlider.getValue());
        updateJUCEDelayParameters();
        logParameterUpdate("JUCE Delay feedback", juceDelayFeedback);
    }
    else if (slider == &juceDelayMixSlider)
    {
        juceDelayMix = static_cast<float>(juceDelayMixSlider.getValue());
        updateJUCEDelayParameters();
        logParameterUpdate("JUCE Delay mix", juceDelayMix);
    }
    else if (slider == &juceChorusRateSlider)
    {
        juceChorusRate = static_cast<float>(juceChorusRateSlider.getValue());
        updateJUCEChorusParameters();
        logParameterUpdate("JUCE Chorus rate", juceChorusRate);
    }
    else if (slider == &juceChorusDepthSlider)
    {
        juceChorusDepth = static_cast<float>(juceChorusDepthSlider.getValue());
        updateJUCEChorusParameters();
        logParameterUpdate("JUCE Chorus depth", juceChorusDepth);
    }
    else if (slider == &juceChorusCentreDelaySlider)
    {
        juceChorusCentreDelay = static_cast<float>(juceChorusCentreDelaySlider.getValue());
        updateJUCEChorusParameters();
        logParameterUpdate("JUCE Chorus centre delay", juceChorusCentreDelay);
    }
    else if (slider == &juceChorusFeedbackSlider)
    {
        juceChorusFeedback = static_cast<float>(juceChorusFeedbackSlider.getValue());
        updateJUCEChorusParameters();
        logParameterUpdate("JUCE Chorus feedback", juceChorusFeedback);
    }
    else if (slider == &juceChorusMixSlider)
    {
        juceChorusMix = static_cast<float>(juceChorusMixSlider.getValue());
        updateJUCEChorusParameters();
        logParameterUpdate("JUCE Chorus mix", juceChorusMix);
    }
    else if (slider == &juceReverbRoomSizeSlider)
    {
        juceReverbRoomSize = static_cast<float>(juceReverbRoomSizeSlider.getValue());
        updateJUCEReverbParameters();
        logParameterUpdate("JUCE Reverb room size", juceReverbRoomSize);
    }
    else if (slider == &juceReverbDampingSlider)
    {
        juceReverbDamping = static_cast<float>(juceReverbDampingSlider.getValue());
        updateJUCEReverbParameters();
        logParameterUpdate("JUCE Reverb damping", juceReverbDamping);
    }
    else if (slider == &juceReverbWetLevelSlider)
    {
        juceReverbWetLevel = static_cast<float>(juceReverbWetLevelSlider.getValue());
        updateJUCEReverbParameters();
        logParameterUpdate("JUCE Reverb wet level", juceReverbWetLevel);
    }
    else if (slider == &juceReverbDryLevelSlider)
    {
        juceReverbDryLevel = static_cast<float>(juceReverbDryLevelSlider.getValue());
        updateJUCEReverbParameters();
        logParameterUpdate("JUCE Reverb dry level", juceReverbDryLevel);
    }
    else if (slider == &juceReverbWidthSlider)
    {
        juceReverbWidth = static_cast<float>(juceReverbWidthSlider.getValue());
        updateJUCEReverbParameters();
        logParameterUpdate("JUCE Reverb width", juceReverbWidth);
    }
    else if (slider == &jucePhaserRateSlider)
    {
        jucePhaserRate = static_cast<float>(jucePhaserRateSlider.getValue());
        updateJUCEPhaserParameters();
        logParameterUpdate("JUCE Phaser rate", jucePhaserRate);
    }
    else if (slider == &jucePhaserDepthSlider)
    {
        jucePhaserDepth = static_cast<float>(jucePhaserDepthSlider.getValue());
        updateJUCEPhaserParameters();
        logParameterUpdate("JUCE Phaser depth", jucePhaserDepth);
    }
    else if (slider == &jucePhaserCentreFreqSlider)
    {
        jucePhaserCentreFreq = static_cast<float>(jucePhaserCentreFreqSlider.getValue());
        updateJUCEPhaserParameters();
        logParameterUpdate("JUCE Phaser centre freq", jucePhaserCentreFreq);
    }
    else if (slider == &jucePhaserFeedbackSlider)
    {
        jucePhaserFeedback = static_cast<float>(jucePhaserFeedbackSlider.getValue());
        updateJUCEPhaserParameters();
        logParameterUpdate("JUCE Phaser feedback", jucePhaserFeedback);
    }
    else if (slider == &jucePhaserMixSlider)
    {
        jucePhaserMix = static_cast<float>(jucePhaserMixSlider.getValue());
        updateJUCEPhaserParameters();
        logParameterUpdate("JUCE Phaser mix", jucePhaserMix);
    }
    else if (slider == &juceCompressorThresholdSlider)
    {
        juceCompressorThreshold = static_cast<float>(juceCompressorThresholdSlider.getValue());
        updateJUCECompressorParameters();
        logParameterUpdate("JUCE Compressor threshold", juceCompressorThreshold);
    }
    else if (slider == &juceCompressorRatioSlider)
    {
        juceCompressorRatio = static_cast<float>(juceCompressorRatioSlider.getValue());
        updateJUCECompressorParameters();
        logParameterUpdate("JUCE Compressor ratio", juceCompressorRatio);
    }
    else if (slider == &juceCompressorAttackSlider)
    {
        juceCompressorAttack = static_cast<float>(juceCompressorAttackSlider.getValue());
        updateJUCECompressorParameters();
        logParameterUpdate("JUCE Compressor attack", juceCompressorAttack);
    }
    else if (slider == &juceCompressorReleaseSlider)
    {
        juceCompressorRelease = static_cast<float>(juceCompressorReleaseSlider.getValue());
        updateJUCECompressorParameters();
        logParameterUpdate("JUCE Compressor release", juceCompressorRelease);
    }
    else if (slider == &juceDistortionDriveSlider)
    {
        juceDistortionDrive = static_cast<float>(juceDistortionDriveSlider.getValue());
        updateJUCEDistortionParameters();
        logParameterUpdate("JUCE Distortion drive", juceDistortionDrive);
    }
    else if (slider == &juceDistortionRangeSlider)
    {
        juceDistortionRange = static_cast<float>(juceDistortionRangeSlider.getValue());
        updateJUCEDistortionParameters();
        logParameterUpdate("JUCE Distortion range", juceDistortionRange);
    }
    else if (slider == &juceDistortionBlendSlider)
    {
        juceDistortionBlend = static_cast<float>(juceDistortionBlendSlider.getValue());
        updateJUCEDistortionParameters();
        logParameterUpdate("JUCE Distortion blend", juceDistortionBlend);
    }
    // Noise Suppression parameter handling
    else if (slider == &noiseSuppressionVadThresholdSlider)
    {
        noiseSuppressionVadThreshold = static_cast<float>(noiseSuppressionVadThresholdSlider.getValue());
        if (noiseSuppressionProcessor)
        {
            noiseSuppressionProcessor->setVadThreshold(noiseSuppressionVadThreshold);
        }
        logParameterUpdate("Noise Suppression VAD Threshold", noiseSuppressionVadThreshold);
    }
    else if (slider == &noiseSuppressionVadGracePeriodSlider)
    {
        noiseSuppressionVadGracePeriod = static_cast<int>(noiseSuppressionVadGracePeriodSlider.getValue());
        if (noiseSuppressionProcessor)
        {
            noiseSuppressionProcessor->setVadGracePeriod(noiseSuppressionVadGracePeriod);
        }
        logParameterUpdate("Noise Suppression VAD Grace Period", static_cast<float>(noiseSuppressionVadGracePeriod));
    }
    else if (slider == &noiseSuppressionVadRetroactiveGracePeriodSlider)
    {
        noiseSuppressionVadRetroactiveGracePeriod = static_cast<int>(noiseSuppressionVadRetroactiveGracePeriodSlider.getValue());
        if (noiseSuppressionProcessor)
        {
            noiseSuppressionProcessor->setVadRetroactiveGracePeriod(noiseSuppressionVadRetroactiveGracePeriod);
        }
        logParameterUpdate("Noise Suppression VAD Retroactive Grace Period", static_cast<float>(noiseSuppressionVadRetroactiveGracePeriod));
    }
}

void MainComponent::buttonClicked(juce::Button* button)
{
    // SignalGen button removed; integration is via input device selection
   if (button == &bypassToggle)
    {
        isBypassed = bypassToggle.getToggleState();
        juce::Logger::writeToLog("Bypass " + juce::String(isBypassed ? "ON" : "OFF"));
    }
    else if (button == &recordButton)
    {
        if (audioRecorder)
        {
            if (!audioRecorder->isRecording())
            {
                audioRecorder->startRecording(currentSampleRate);
                recordButton.setButtonText("Stop Recording");
            }
            else
            {
                audioRecorder->stopRecording();
                recordButton.setButtonText("Start Recording");
            }
        }
    }
    else if (button == &eqButton)
    {
        openEQWindow();
    }
    else if (button == &sweepTestButton)
    {
        if (!isSweepTesting)
        {
            startSweepTest();
        }
        else
        {
            stopSweepTest();
        }
    }
    else if (button == &showInputSpectrumToggle)
    {
        inputSpectrumAnalyzer.setVisible(showInputSpectrumToggle.getToggleState());
    }
    else if (button == &showOutputSpectrumToggle)
    {
        outputSpectrumAnalyzer.setVisible(showOutputSpectrumToggle.getToggleState());
    }
    else if (button == &showInputWaveformToggle)
    {
        inputWaveformAnalyzer.setVisible(showInputWaveformToggle.getToggleState());
    }
    else if (button == &showOutputWaveformToggle)
    {
        outputWaveformAnalyzer.setVisible(showOutputWaveformToggle.getToggleState());
    }
    else if (button == &effectEnableToggle)
    {
        isEffectEnabled = effectEnableToggle.getToggleState();
        logParameterUpdate("Effect", isEffectEnabled ? 1.0f : 0.0f);
    }
}

void MainComponent::openEQWindow()
{
    if (!eqWindow)
    {
    // Create EQ window (using left channel EQ as control source)
        eqWindow = std::make_unique<EQWindow>(eqProcessorL.get());
        
        if (eqWindow)
        {
            // Synchronize right channel EQ parameters with left channel EQ
            eqWindow->setSecondEQProcessor(eqProcessorR.get());
        }

        if (limiterProcessor)
        {
            eqWindow->setLimiterProcessor(limiterProcessor.get());
        }
        
        // Set callback for closing
        eqWindow->onCloseButtonPressed = [this]()
        {
            closeEQWindow();
        };
        
        // Show window
        eqWindow->setVisible(true);
        eqWindow->toFront(true);
        
        DBG("EQ window opened");
    }
    else
    {
        // Window already exists, toggle visibility
        if (eqWindow->isVisible())
        {
            closeEQWindow();
        }
        else
        {
            eqWindow->setVisible(true);
            eqWindow->toFront(true);
        }
    }
}
void MainComponent::closeEQWindow()
{
    if (eqWindow)
    {
        // First hide the window
        eqWindow->setVisible(false);
        
        // Use MessageManager for asynchronous deletion to avoid direct deletion within event handling
        juce::MessageManager::callAsync([this]()
        {
            if (eqWindow)
            {
                eqWindow->removeFromDesktop();
                eqWindow.reset();
                DBG("EQ window closed and destroyed");
            }
        });
    }
}

void MainComponent::handleCommandMessage(int commandId)
{
    if (commandId == 0x1000)  // Custom message from EQWindow
    {
        closeEQWindow();
    }
}
void MainComponent::comboBoxChanged(juce::ComboBox* comboBox)
{
    if (comboBox == &effectSelector)
    {
        // Handle effect selection
        int selectedId = effectSelector.getSelectedId();
        if (selectedId == 1) // Delay
        {
            currentEffect = Delay;
            juce::Logger::writeToLog("Effect changed to: Delay");
        }
        else if (selectedId == 2) // Flanger
        {
            currentEffect = Flanger;
            juce::Logger::writeToLog("Effect changed to: Flanger");
        }
        else if (selectedId == 3) // Plate Reverb
        {
            currentEffect = PlateReverb;
            juce::Logger::writeToLog("Effect changed to: Plate Reverb");
        }
        else if (selectedId == 4) // Pitch Shift
        {
            currentEffect = PitchShift;
            juce::Logger::writeToLog("Effect changed to: Pitch Shift");
        }
        else if (selectedId == 5) // Cathedral Reverb
        {
            currentEffect = CathedralReverb;
            juce::Logger::writeToLog("Effect changed to: Cathedral Reverb");
        }
        else if (selectedId == 6) // JUCE Delay
        {
            currentEffect = JUCEDelay;
            juce::Logger::writeToLog("Effect changed to: JUCE Delay");
        }
        else if (selectedId == 7) // JUCE Chorus
        {
            currentEffect = JUCEChorus;
            juce::Logger::writeToLog("Effect changed to: JUCE Chorus");
        }
        else if (selectedId == 8) // JUCE Reverb
        {
            currentEffect = JUCEReverb;
            juce::Logger::writeToLog("Effect changed to: JUCE Reverb");
        }
        else if (selectedId == 9) // JUCE Phaser
        {
            currentEffect = JUCEPhaser;
            juce::Logger::writeToLog("Effect changed to: JUCE Phaser");
        }
        else if (selectedId == 10) // JUCE Compressor
        {
            currentEffect = JUCECompressor;
            juce::Logger::writeToLog("Effect changed to: JUCE Compressor");
        }
        else if (selectedId == 11) // JUCE Distortion
        {
            currentEffect = JUCEDistortion;
            juce::Logger::writeToLog("Effect changed to: JUCE Distortion");
        }
        else if (selectedId == 12) // Noise Suppression
        {
            currentEffect = NoiseSuppression;
            juce::Logger::writeToLog("Effect changed to: Noise Suppression");
        }

        // Update control display
        updateEffectControls();
    }
    else
    {
        // Handle device selection
        auto inputIndex = inputDeviceSelector.getSelectedItemIndex() - 1;
        auto outputIndex = outputDeviceSelector.getSelectedItemIndex() - 1;

        // Get user selected input/output device names
        juce::String inputDevice = (inputIndex >= 0 && inputIndex < availableInputDevices.size()) ? availableInputDevices[inputIndex] : juce::String();
        juce::String outputDevice = (outputIndex >= 0 && outputIndex < availableOutputDevices.size()) ? availableOutputDevices[outputIndex] : juce::String();

        // Determine whether the pseudo internal generator is selected regardless of whether hardware changes
        const bool wantInternalGenerator = (inputDevice == "Inside Signal Generator");

        // Always flip internal generator mode based on selection
        if (wantInternalGenerator)
        {
            useInternalSignalGen = true;
            // Remember the current hardware input device name to restore on close
            juce::AudioDeviceManager::AudioDeviceSetup curBefore;
            deviceManager.getAudioDeviceSetup(curBefore);
            previousInputDeviceName = curBefore.inputDeviceName;
            if (!signalGenWindow)
            {
                signalGenWindow = std::make_unique<SignalGenWindow>(signalGenEngine.get());
                // When the SG window is closed, fall back to external input
                signalGenWindow->onClose = [this]() {
                    // Leaving internal generator: switch input device back to the one used before entering
                    useInternalSignalGen = false;
                    if (signalGenEngine && signalGenEngine->isRunning())
                        signalGenEngine->stop();

                    juce::AudioDeviceManager::AudioDeviceSetup setup;
                    deviceManager.getAudioDeviceSetup(setup);
                    if (previousInputDeviceName.isNotEmpty())
                    {
                        setup.inputDeviceName = previousInputDeviceName;
                        deviceManager.setAudioDeviceSetup(setup, true);
                    }
                    // Refresh input combobox to show active device name
                    updateDeviceLists();
                };
            }
            if (!signalGenWindow->isOnDesktop())
                signalGenWindow->addToDesktop();
            signalGenWindow->setVisible(true);
            signalGenWindow->toFront(true);

            // If only output changed, apply it while keeping current hardware input
            juce::AudioDeviceManager::AudioDeviceSetup current;
            deviceManager.getAudioDeviceSetup(current);
            if (!outputDevice.isEmpty() && outputDevice != current.outputDeviceName)
            {
                current.outputDeviceName = outputDevice;
                deviceManager.setAudioDeviceSetup(current, true);
            }
            return; // Do not change hardware input for internal option
        }
        else
        {
            // Exiting internal generator mode: ensure it is stopped and window hidden
            useInternalSignalGen = false;
            if (signalGenEngine && signalGenEngine->isRunning())
                signalGenEngine->stop();
            if (signalGenWindow)
                signalGenWindow->setVisible(false);

            // If a real hardware device is being selected from combobox while not the special option,
            // we also update the remembered device name as the new baseline
            if (!inputDevice.isEmpty())
                previousInputDeviceName = inputDevice;
        }

        // For real device selection, apply changes if different from current
        juce::AudioDeviceManager::AudioDeviceSetup setup;
        deviceManager.getAudioDeviceSetup(setup);
        bool changed = false;
        if (!inputDevice.isEmpty() && inputDevice != setup.inputDeviceName)
        {
            setup.inputDeviceName = inputDevice;
            changed = true;
        }
        if (!outputDevice.isEmpty() && outputDevice != setup.outputDeviceName)
        {
            setup.outputDeviceName = outputDevice;
            changed = true;
        }
        if (changed)
        {
            deviceManager.setAudioDeviceSetup(setup, true);
            juce::Logger::writeToLog("Device setup success.");
        }
    }
}