#include "SignalGeneratorComponent.h"

using namespace SignalGenConstants;

SignalGeneratorComponent::SignalGeneratorComponent(SignalGenAudioEngine* sharedEngine, bool ownsAudioDevice)
    : ownsAudioDevice(ownsAudioDevice)
{
    // Initialize audio engine
    if (sharedEngine != nullptr)
    {
        audioEngine = sharedEngine;
        ownsEngine = false;
    }
    else
    {
        audioEngine = new SignalGenAudioEngine();
        ownsEngine = true;
    }
    
    // Initialize core logic
    core = std::make_unique<SignalGeneratorCore>(audioEngine);
    
    // Setup UI
    setupUI();
    setupRadioGroups();
    setupListeners();
    setDefaultValues();
    
    // Set window size
    setSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    
    // Set audio channels
    if (ownsAudioDevice)
        setAudioChannels(0, 2);
}

SignalGeneratorComponent::~SignalGeneratorComponent()
{
    if (ownsAudioDevice)
        shutdownAudio();
    
    if (audioEngine && ownsEngine)
    {
        delete audioEngine;
        audioEngine = nullptr;
    }
}

//==============================================================================
// Audio Processing
//==============================================================================

void SignalGeneratorComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    if (audioEngine)
        audioEngine->prepareToPlay(sampleRate, samplesPerBlockExpected);
}

void SignalGeneratorComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (audioEngine)
        audioEngine->processBlock(*bufferToFill.buffer);
    else
        bufferToFill.clearActiveBufferRegion();
}

void SignalGeneratorComponent::releaseResources()
{
    if (audioEngine)
        audioEngine->releaseResources();
}

//==============================================================================
// UI Drawing and Layout
//==============================================================================

void SignalGeneratorComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

void SignalGeneratorComponent::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    layoutMainColumns(bounds);
}

void SignalGeneratorComponent::layoutMainColumns(juce::Rectangle<int> bounds)
{
    juce::FlexBox mainLayout;
    mainLayout.flexDirection = juce::FlexBox::Direction::column;
    
    juce::FlexBox columnsLayout;
    columnsLayout.flexDirection = juce::FlexBox::Direction::row;
    
    // Create three-column layout
    juce::FlexBox column1, column2, column3;
    column1.flexDirection = juce::FlexBox::Direction::column;
    column2.flexDirection = juce::FlexBox::Direction::column;
    column3.flexDirection = juce::FlexBox::Direction::column;
    
    // First column: Frequency and Waveform
    column1.items.add(juce::FlexItem(frequencyRange.group).withHeight(FREQUENCY_RANGE_GROUP_HEIGHT));
    column1.items.add(juce::FlexItem(waveform.group).withHeight(WAVEFORM_GROUP_HEIGHT));
    column1.items.add(juce::FlexItem(waveform.dutyCycleGroup).withHeight(DUTY_CYCLE_GROUP_HEIGHT));
    column1.items.add(juce::FlexItem(manualFrequency.group).withFlex(1).withMinHeight(MANUAL_FREQUENCY_GROUP_MIN_HEIGHT));
    
    // Second column: Sweep control and buttons
    column2.items.add(juce::FlexItem(sweep.modeGroup).withHeight(SWEEP_MODE_GROUP_HEIGHT));
    column2.items.add(juce::FlexItem(sweep.speedGroup).withFlex(1));  // Let sweep speed group occupy remaining space
    
    // Add custom frequency range and control buttons at the bottom
    juce::FlexBox bottomControls;
    bottomControls.flexDirection = juce::FlexBox::Direction::column;
    
    // Custom frequency range editor
    juce::FlexBox customFreqBox;
    customFreqBox.items.add(juce::FlexItem(frequencyRange.startFreqEditor).withWidth(60));
    customFreqBox.items.add(juce::FlexItem(frequencyRange.toLabel).withWidth(30));
    customFreqBox.items.add(juce::FlexItem(frequencyRange.endFreqEditor).withWidth(60));
    customFreqBox.items.add(juce::FlexItem(frequencyRange.hzLabel).withWidth(30));
    bottomControls.items.add(juce::FlexItem(customFreqBox).withHeight(30));
    
    // Control buttons
    juce::FlexBox buttonBox;
    buttonBox.justifyContent = juce::FlexBox::JustifyContent::center;
    buttonBox.items.add(juce::FlexItem(startButton).withWidth(100).withHeight(BUTTON_HEIGHT).withMargin(juce::FlexItem::Margin(0, 10, 0, 0)));
    buttonBox.items.add(juce::FlexItem(closeButton).withWidth(100).withHeight(BUTTON_HEIGHT).withMargin(juce::FlexItem::Margin(0, 0, 0, 10)));
    bottomControls.items.add(juce::FlexItem(buttonBox).withHeight(40).withMargin(juce::FlexItem::Margin(10, 0, 0, 0)));
    
    column2.items.add(juce::FlexItem(bottomControls).withHeight(80));  // Fixed height for bottom controls
    
    // Third column: Output control
    column3.items.add(juce::FlexItem(outputLevel.group).withHeight(OUTPUT_LEVEL_GROUP_HEIGHT));
    column3.items.add(juce::FlexItem(channels.group).withFlex(1));  // Let channels group occupy remaining space
    
    // Add columns to main layout
    columnsLayout.items.add(juce::FlexItem(column1).withFlex(1).withMargin(juce::FlexItem::Margin(0, 5, 0, 0)));
    columnsLayout.items.add(juce::FlexItem(column2).withFlex(1).withMargin(juce::FlexItem::Margin(0, 5, 0, 5)));
    columnsLayout.items.add(juce::FlexItem(column3).withFlex(1).withMargin(juce::FlexItem::Margin(0, 0, 0, 5)));
    
    mainLayout.items.add(juce::FlexItem(columnsLayout).withFlex(1));
    mainLayout.performLayout(bounds);
    
    // Layout internal panels
    layoutFrequencyControls(frequencyRange.group.getBounds());
    layoutWaveformControls(waveform.group.getBounds());
    layoutSweepControls(sweep.speedGroup.getBounds());
    layoutOutputControls(outputLevel.group.getBounds());
    layoutChannelControls(channels.group.getBounds());
}

void SignalGeneratorComponent::layoutFrequencyControls(juce::Rectangle<int> /*bounds*/)
{
    LayoutHelper::layoutVerticalButtons(frequencyRange.group, frequencyRange.getRadioButtonsAsComponents());
}

void SignalGeneratorComponent::layoutWaveformControls(juce::Rectangle<int> /*bounds*/)
{
    auto waveformBounds = waveform.group.getBounds().reduced(GROUP_MARGIN, GROUP_HEADER_HEIGHT);
    auto buttons = waveform.getWaveformButtonsAsComponents();
    int buttonHeight = static_cast<int>(waveformBounds.getHeight() / buttons.size());
    
    for (auto* button : buttons)
    {
        button->setBounds(waveformBounds.removeFromTop(buttonHeight));
    }
    
    // Duty cycle slider
    auto dutyCycleBounds = waveform.dutyCycleGroup.getBounds().reduced(GROUP_MARGIN, GROUP_HEADER_HEIGHT);
    waveform.dutyCycleSlider.setBounds(dutyCycleBounds);
    
    // Manual frequency sliders - increase height by 50%
    auto manualBounds = manualFrequency.group.getBounds().reduced(GROUP_MARGIN, GROUP_HEADER_HEIGHT);
    // Reserve bottom space for text box and lock button
    auto bottomControls = manualBounds.removeFromBottom(50);
    // Sliders occupy remaining space
    auto slidersBounds = manualBounds;
    manualFrequency.leftFrequencySlider.setBounds(slidersBounds.removeFromLeft(slidersBounds.getWidth()/2).reduced(5, 0));
    manualFrequency.rightFrequencySlider.setBounds(slidersBounds.reduced(5, 0));
    
    // Lock button at the bottom
    manualFrequency.lockFrequencyButton.setBounds(bottomControls.removeFromBottom(25));
    
    // Sweep mode
    LayoutHelper::layoutVerticalButtons(sweep.modeGroup, sweep.getModeButtonsAsComponents());
}

void SignalGeneratorComponent::layoutSweepControls(juce::Rectangle<int> /*bounds*/)
{
    auto sweepBounds = sweep.speedGroup.getBounds().reduced(GROUP_MARGIN, GROUP_HEADER_HEIGHT);
    
    // Sweep speed buttons
    for (auto* button : {&sweep.fastSmoothButton, &sweep.fastSteppedButton})
    {
        button->setBounds(sweepBounds.removeFromTop(25));
    }
    
    sweep.sweepSpeedSlider.setBounds(sweepBounds.removeFromTop(30).reduced(5, 0));
    
    for (auto* button : {&sweep.slowButton, &sweep.manualButton, &sweep.whiteNoiseButton, 
                         &sweep.pinkNoiseButton, &sweep.noSweepButton})
    {
        button->setBounds(sweepBounds.removeFromTop(25));
    }
}

void SignalGeneratorComponent::layoutOutputControls(juce::Rectangle<int> bounds)
{
    auto outputBounds = bounds.reduced(GROUP_MARGIN, GROUP_HEADER_HEIGHT);
    auto lockRow = outputBounds.removeFromBottom(55);
    auto labelArea = outputBounds.removeFromLeft(40);
    auto dbArea = labelArea.removeFromBottom(20);
    
    // Level meter layout
    int slidersTotalWidth = METER_WIDTH * 2 + METER_SPACING;
    auto centered = outputBounds.withSizeKeepingCentre(slidersTotalWidth, outputBounds.getHeight());
    
    // Left channel
    auto leftArea = centered.removeFromLeft(METER_WIDTH);
    auto leftSliderArea = leftArea.withHeight(leftArea.getHeight() - 25);
    outputLevel.leftChannelSlider.setBounds(leftSliderArea.reduced(6, 0));
    {
        const int toggleWidth = 45;
        const int toggleHeight = 22;
        const int toggleY = outputBounds.getBottom() ; // Ensure it doesn't overlap with the bottom lock area
        outputLevel.leftChannelToggle.setBounds(leftArea.getCentreX() - toggleWidth / 2, toggleY, toggleWidth, toggleHeight);
    }
    
    centered.removeFromLeft(METER_SPACING);
    
    // Right channel
    auto rightArea = centered.removeFromLeft(METER_WIDTH);
    auto rightSliderArea = rightArea.withHeight(rightArea.getHeight() - 25);
    outputLevel.rightChannelSlider.setBounds(rightSliderArea.reduced(6, 0));
    {
        const int toggleWidth = 45;
        const int toggleHeight = 22;
        const int toggleY = outputBounds.getBottom() ; // Ensure it doesn't overlap with the bottom lock area
        outputLevel.rightChannelToggle.setBounds(rightArea.getCentreX() - toggleWidth / 2, toggleY, toggleWidth, toggleHeight);
    }
    
    // Value labels
    outputLevel.leftLevelValueLabel.setBounds(leftArea.getX(), leftSliderArea.getBottom() + 10, leftArea.getWidth() + 20, 18);
    outputLevel.rightLevelValueLabel.setBounds(rightArea.getX(), rightSliderArea.getBottom() + 10, rightArea.getWidth() + 20, 18);
    
    // dB scale labels
    float labelHeight = static_cast<float>(labelArea.getHeight()) / 13.0f;
    for (size_t i = 0; i < outputLevel.levelLabels.size(); ++i)
    {
        outputLevel.levelLabels[i]->setBounds(labelArea.getX(), 
                                             static_cast<int>(labelArea.getY() + (i * labelHeight)),
                                             35, 
                                             static_cast<int>(labelHeight));
    }
    outputLevel.dbLabel.setBounds(dbArea.getX(), dbArea.getY(), 35, dbArea.getHeight());
    
    // Lock button
    auto lockBounds = lockRow.removeFromLeft(220).translated(0, 30);
    outputLevel.lockOutputLevelButton.setBounds(lockBounds);
}

void SignalGeneratorComponent::layoutChannelControls(juce::Rectangle<int> /*bounds*/)
{
    LayoutHelper::layoutVerticalButtons(channels.group, channels.getButtonsAsComponents());
}

//==============================================================================
// Event Handling
//==============================================================================

void SignalGeneratorComponent::buttonClicked(juce::Button* button)
{
    // Control buttons
    if (button == &startButton)
    {
        if (core->isRunning())
        {
            core->stop();
            startButton.setButtonText("Start");
            ComponentFactory::styleButton(startButton, juce::Colours::green);
        }
        else
        {
            core->start();
            startButton.setButtonText("Stop");
            ComponentFactory::styleButton(startButton, juce::Colours::red);
        }
    }
    else if (button == &closeButton)
    {
        if (auto* w = findParentComponentOfClass<juce::DocumentWindow>())
            w->closeButtonPressed();
        else
            setVisible(false);
    }
    // Frequency range
    else if (button == &frequencyRange.customRangeButton || button == &frequencyRange.wideRangeButton ||
             button == &frequencyRange.hfRangeButton || button == &frequencyRange.speechRangeButton ||
             button == &frequencyRange.lfRangeButton)
    {
        updateFrequencyRange();
    }
    // Waveform
    else if (button == &waveform.sineWaveButton || button == &waveform.squareWaveButton ||
             button == &waveform.triangleWaveButton || button == &waveform.sawtoothWaveButton ||
             button == &waveform.pulseWaveButton)
    {
        updateWaveform();
    }
    // Sweep mode
    else if (button == &sweep.linearSweepButton || button == &sweep.logSweepButton)
    {
        updateSweepMode();
    }
    // Sweep speed
    else if (button == &sweep.fastSmoothButton || button == &sweep.fastSteppedButton ||
             button == &sweep.slowButton || button == &sweep.manualButton ||
             button == &sweep.whiteNoiseButton || button == &sweep.pinkNoiseButton ||
             button == &sweep.noSweepButton)
    {
        updateSweepSpeed();
    }
    // Channel mode
    else if (button == &channels.inPhaseButton || button == &channels.phase180Button ||
             button == &channels.independentButton)
    {
        updateChannels();
    }
    // Lock control
    else if (button == &manualFrequency.lockFrequencyButton)
    {
        core->setLockFrequencies(button->getToggleState());
        if (button->getToggleState())
        {
            manualFrequency.rightFrequencySlider.setValue(manualFrequency.leftFrequencySlider.getValue());
        }
    }
    else if (button == &outputLevel.lockOutputLevelButton)
    {
        core->setLockLevels(button->getToggleState());
        if (button->getToggleState())
        {
            float value = static_cast<float>(outputLevel.leftChannelSlider.getValue());
            outputLevel.rightChannelSlider.setValue(value, juce::dontSendNotification);
            // Immediately sync once to ensure levels are consistent with lock logic, even if a channel is disabled
            core->setOutputLevel(value, value);
        }
    }
    // Channel enable
    else if (button == &outputLevel.leftChannelToggle || button == &outputLevel.rightChannelToggle)
    {
        const bool leftOn = outputLevel.leftChannelToggle.getToggleState();
        const bool rightOn = outputLevel.rightChannelToggle.getToggleState();

        // Ensure immediate reflection in the engine
        core->setChannelEnabled(leftOn, rightOn);
    }
}

void SignalGeneratorComponent::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &manualFrequency.leftFrequencySlider || slider == &manualFrequency.rightFrequencySlider)
    {
        syncFrequencySliders(slider);
    }
    else if (slider == &outputLevel.leftChannelSlider || slider == &outputLevel.rightChannelSlider)
    {
        syncLevelSliders(slider);
        updateLevelValueLabels();
    }
    else if (slider == &sweep.sweepSpeedSlider)
    {
        core->setSweepSpeedValue(static_cast<float>(slider->getValue()));
    }
    else if (slider == &waveform.dutyCycleSlider)
    {
        core->setDutyCycle(static_cast<float>(slider->getValue()));
    }
}

void SignalGeneratorComponent::comboBoxChanged(juce::ComboBox* /*comboBox*/)
{
    // 当前没有ComboBox
}

//==============================================================================
// Initialization Methods
//==============================================================================

void SignalGeneratorComponent::setupUI()
{
    // Frequency range panel
    frequencyRange.group.setText("Frequency Range");
    addAndMakeVisible(frequencyRange.group);
    for (auto* button : frequencyRange.getRadioButtons())
        addAndMakeVisible(button);
    
    // Custom frequency range
    frequencyRange.startFreqEditor.setText("1000");
    frequencyRange.endFreqEditor.setText("1000");
    frequencyRange.startFreqEditor.setInputRestrictions(7, "0123456789.");
    frequencyRange.endFreqEditor.setInputRestrictions(7, "0123456789.");
    frequencyRange.startFreqEditor.onTextChange = [this]() { 
        if (frequencyRange.customRangeButton.getToggleState()) 
            updateFrequencyRange(); 
    };
    frequencyRange.endFreqEditor.onTextChange = [this]() { 
        if (frequencyRange.customRangeButton.getToggleState()) 
            updateFrequencyRange(); 
    };
    addAndMakeVisible(frequencyRange.startFreqEditor);
    addAndMakeVisible(frequencyRange.endFreqEditor);
    addAndMakeVisible(frequencyRange.toLabel);
    addAndMakeVisible(frequencyRange.hzLabel);
    
    // Waveform panel
    waveform.group.setText("Waveform");
    addAndMakeVisible(waveform.group);
    for (auto* button : waveform.getWaveformButtons())
        addAndMakeVisible(button);
    
    // Duty Cycle
    waveform.dutyCycleGroup.setText("Duty Cycle");
    waveform.dutyCycleSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    waveform.dutyCycleSlider.setRange(MIN_DUTY_CYCLE, MAX_DUTY_CYCLE, DUTY_CYCLE_STEP);
    waveform.dutyCycleSlider.setValue(DEFAULT_DUTY_CYCLE);
    waveform.dutyCycleSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    waveform.dutyCycleSlider.setTextValueSuffix("%");
    addAndMakeVisible(waveform.dutyCycleGroup);
    addAndMakeVisible(waveform.dutyCycleSlider);
    
    // Manual frequency panel
    manualFrequency.group.setText("Manual Frequency");
    addAndMakeVisible(manualFrequency.group);
    
    // Configure frequency sliders
    auto setupFrequencySlider = [](juce::Slider& slider) {
        slider.setSliderStyle(juce::Slider::LinearVertical);
        slider.setRange(MIN_FREQUENCY, MAX_FREQUENCY);
        slider.setValue(DEFAULT_FREQUENCY);
        slider.setSkewFactorFromMidPoint(FREQUENCY_SKEW_MIDPOINT);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, SLIDER_TEXT_BOX_WIDTH, SLIDER_TEXT_BOX_HEIGHT);
    };
    
    setupFrequencySlider(manualFrequency.leftFrequencySlider);
    setupFrequencySlider(manualFrequency.rightFrequencySlider);
    addAndMakeVisible(manualFrequency.leftFrequencySlider);
    addAndMakeVisible(manualFrequency.rightFrequencySlider);
    addAndMakeVisible(manualFrequency.lockFrequencyButton);
    
    // Sweep panel
    sweep.modeGroup.setText("Sweep Mode");
    sweep.speedGroup.setText("Sweep Speed");
    addAndMakeVisible(sweep.modeGroup);
    addAndMakeVisible(sweep.speedGroup);
    
    for (auto* button : sweep.getModeButtons())
        addAndMakeVisible(button);
    for (auto* button : sweep.getSpeedButtons())
        addAndMakeVisible(button);
    
    sweep.sweepSpeedSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    sweep.sweepSpeedSlider.setRange(MIN_SWEEP_SPEED, MAX_SWEEP_SPEED);
    sweep.sweepSpeedSlider.setValue(DEFAULT_SWEEP_SPEED);
    sweep.sweepSpeedSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(sweep.sweepSpeedSlider);
    
    // Output level panel
    outputLevel.group.setText("Output Level");
    addAndMakeVisible(outputLevel.group);
    
    // Configure level sliders
    auto setupLevelSlider = [this](juce::Slider& slider) {
        slider.setLookAndFeel(&meterLookAndFeel);
        slider.setSliderStyle(juce::Slider::LinearBarVertical);
        slider.setRange(MIN_LEVEL_DB, MAX_LEVEL_DB, LEVEL_STEP_DB);
        slider.setValue(DEFAULT_LEVEL_DB);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    };
    
    setupLevelSlider(outputLevel.leftChannelSlider);
    setupLevelSlider(outputLevel.rightChannelSlider);
    addAndMakeVisible(outputLevel.leftChannelSlider);
    addAndMakeVisible(outputLevel.rightChannelSlider);
    addAndMakeVisible(outputLevel.leftChannelToggle);
    addAndMakeVisible(outputLevel.rightChannelToggle);
    addAndMakeVisible(outputLevel.lockOutputLevelButton);
    
    // Value labels
    outputLevel.leftLevelValueLabel.setJustificationType(juce::Justification::centred);
    outputLevel.rightLevelValueLabel.setJustificationType(juce::Justification::centred);
    outputLevel.leftLevelValueLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    outputLevel.rightLevelValueLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(outputLevel.leftLevelValueLabel);
    addAndMakeVisible(outputLevel.rightLevelValueLabel);
    
    // Create dB scale labels
    outputLevel.levelLabels.clear();
    for (int i = 0; i <= 12; ++i)
    {
        auto label = ComponentFactory::createLabel(juce::String(i * -5), juce::Justification::centredRight);
        addAndMakeVisible(label.get());
        outputLevel.levelLabels.push_back(std::move(label));
    }
    addAndMakeVisible(outputLevel.dbLabel);
    
    // Channel panel
    channels.group.setText("Channels");
    addAndMakeVisible(channels.group);
    for (auto* button : channels.getButtons())
        addAndMakeVisible(button);
    
    // Control buttons
    startButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    startButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::green);
    addAndMakeVisible(startButton);
    addAndMakeVisible(closeButton);
}

void SignalGeneratorComponent::setupRadioGroups()
{
    // Set radio button groups
    for (auto* button : frequencyRange.getRadioButtons())
        button->setRadioGroupId(FREQ_RANGE_GROUP_ID);
    
    for (auto* button : waveform.getWaveformButtons())
        button->setRadioGroupId(WAVEFORM_GROUP_ID);
    
    for (auto* button : sweep.getModeButtons())
        button->setRadioGroupId(SWEEP_MODE_GROUP_ID);
    
    for (auto* button : sweep.getSpeedButtons())
        button->setRadioGroupId(SWEEP_SPEED_GROUP_ID);
    
    for (auto* button : channels.getButtons())
        button->setRadioGroupId(CHANNEL_GROUP_ID);
}

void SignalGeneratorComponent::setupListeners()
{
    // Frequency range
    for (auto* button : frequencyRange.getRadioButtons())
        button->addListener(this);
    
    // Waveform
    for (auto* button : waveform.getWaveformButtons())
        button->addListener(this);
    waveform.dutyCycleSlider.addListener(this);
    
    // Manual frequency
    manualFrequency.leftFrequencySlider.addListener(this);
    manualFrequency.rightFrequencySlider.addListener(this);
    manualFrequency.lockFrequencyButton.addListener(this);
    
    // Sweep
    for (auto* button : sweep.getModeButtons())
        button->addListener(this);
    for (auto* button : sweep.getSpeedButtons())
        button->addListener(this);
    sweep.sweepSpeedSlider.addListener(this);
    
    // Output level
    outputLevel.leftChannelSlider.addListener(this);
    outputLevel.rightChannelSlider.addListener(this);
    outputLevel.leftChannelToggle.addListener(this);
    outputLevel.rightChannelToggle.addListener(this);
    outputLevel.lockOutputLevelButton.addListener(this);
    
    // Channels
    for (auto* button : channels.getButtons())
        button->addListener(this);
    
    // Control buttons
    startButton.addListener(this);
    closeButton.addListener(this);
}

void SignalGeneratorComponent::setDefaultValues()
{
    // Set default values
    sweep.logSweepButton.setToggleState(true, juce::dontSendNotification);
    sweep.noSweepButton.setToggleState(true, juce::dontSendNotification);
    waveform.sineWaveButton.setToggleState(true, juce::dontSendNotification);
    frequencyRange.wideRangeButton.setToggleState(true, juce::dontSendNotification);
    channels.inPhaseButton.setToggleState(true, juce::dontSendNotification);
    manualFrequency.lockFrequencyButton.setToggleState(true, juce::dontSendNotification);
    outputLevel.lockOutputLevelButton.setToggleState(true, juce::dontSendNotification);
    outputLevel.leftChannelToggle.setToggleState(true, juce::dontSendNotification);
    outputLevel.rightChannelToggle.setToggleState(true, juce::dontSendNotification);
    
    // Initialize settings
    updateFrequencyRange();
    updateWaveform();
    updateSweepMode();
    updateSweepSpeed();
    updateChannels();
    updateLevelValueLabels();
    
    // Set audio engine default values
    core->setChannelEnabled(true, true);
    core->setOutputLevel(DEFAULT_LEVEL_DB, DEFAULT_LEVEL_DB);
}

//==============================================================================
// Update Methods
//==============================================================================

void SignalGeneratorComponent::updateFrequencyRange()
{
    bool isCustom = frequencyRange.customRangeButton.getToggleState();
    float start = DEFAULT_FREQUENCY;
    float end = DEFAULT_FREQUENCY;
    
    if (!isCustom)
    {
    // Find selected preset
    int presetIndex = -1;
    if (frequencyRange.wideRangeButton.getToggleState()) presetIndex = 0;
    else if (frequencyRange.hfRangeButton.getToggleState()) presetIndex = 1;
    else if (frequencyRange.speechRangeButton.getToggleState()) presetIndex = 2;
    else if (frequencyRange.lfRangeButton.getToggleState()) presetIndex = 3;
    
    if (presetIndex >= 0)
    {
        const auto& preset = FREQUENCY_PRESETS[presetIndex];
        start = preset.startFreq;
        end = preset.endFreq;
        frequencyRange.startFreqEditor.setText(preset.startText, juce::dontSendNotification);
        frequencyRange.endFreqEditor.setText(preset.endText, juce::dontSendNotification);
    }
    }
    else
    {
        start = frequencyRange.startFreqEditor.getText().getFloatValue();
        end = frequencyRange.endFreqEditor.getText().getFloatValue();
    }
    
    // Enable/disable custom editor
    frequencyRange.startFreqEditor.setEnabled(isCustom);
    frequencyRange.endFreqEditor.setEnabled(isCustom);
    
    // Set frequency range
    core->setFrequencyRange(start, end);
    
    // Set sliders to geometric mean
    if (!isCustom)
    {
        float geometricMean = core->getGeometricMean(start, end);
        manualFrequency.leftFrequencySlider.setValue(geometricMean, juce::dontSendNotification);
        manualFrequency.rightFrequencySlider.setValue(geometricMean, juce::dontSendNotification);
        core->setManualFrequency(geometricMean, geometricMean);
    }
}

void SignalGeneratorComponent::updateWaveform()
{
    if (waveform.sineWaveButton.getToggleState())
        core->setWaveform(SignalGenAudioEngine::Waveform::Sine);
    else if (waveform.squareWaveButton.getToggleState())
        core->setWaveform(SignalGenAudioEngine::Waveform::Square);
    else if (waveform.triangleWaveButton.getToggleState())
        core->setWaveform(SignalGenAudioEngine::Waveform::Triangle);
    else if (waveform.sawtoothWaveButton.getToggleState())
        core->setWaveform(SignalGenAudioEngine::Waveform::Sawtooth);
    else if (waveform.pulseWaveButton.getToggleState())
        core->setWaveform(SignalGenAudioEngine::Waveform::Pulse);
}

void SignalGeneratorComponent::updateSweepMode()
{
    if (sweep.linearSweepButton.getToggleState())
        core->setSweepMode(SignalGenAudioEngine::SweepMode::Linear);
    else if (sweep.logSweepButton.getToggleState())
        core->setSweepMode(SignalGenAudioEngine::SweepMode::Logarithmic);
}

void SignalGeneratorComponent::updateSweepSpeed()
{
    bool enableSlider = false;
    
    if (sweep.fastSmoothButton.getToggleState())
    {
        core->setSweepSpeed(SignalGenAudioEngine::SweepSpeed::FastSmooth);
        enableSlider = true;
    }
    else if (sweep.fastSteppedButton.getToggleState())
    {
        core->setSweepSpeed(SignalGenAudioEngine::SweepSpeed::FastStepped);
        enableSlider = true;
    }
    else if (sweep.slowButton.getToggleState())
    {
        core->setSweepSpeed(SignalGenAudioEngine::SweepSpeed::Slow);
        enableSlider = true;
    }
    else if (sweep.manualButton.getToggleState())
    {
        core->setSweepSpeed(SignalGenAudioEngine::SweepSpeed::Manual);
    }
    else if (sweep.whiteNoiseButton.getToggleState())
    {
        core->setSweepSpeed(SignalGenAudioEngine::SweepSpeed::WhiteNoise);
    }
    else if (sweep.pinkNoiseButton.getToggleState())
    {
        core->setSweepSpeed(SignalGenAudioEngine::SweepSpeed::PinkNoise);
    }
    else if (sweep.noSweepButton.getToggleState())
    {
        core->setSweepSpeed(SignalGenAudioEngine::SweepSpeed::NoSweep);
    }
    
    sweep.sweepSpeedSlider.setEnabled(enableSlider);
}

void SignalGeneratorComponent::updateChannels()
{
    if (channels.inPhaseButton.getToggleState())
        core->setChannelMode(SignalGenAudioEngine::ChannelMode::InPhase);
    else if (channels.phase180Button.getToggleState())
        core->setChannelMode(SignalGenAudioEngine::ChannelMode::Phase180);
    else if (channels.independentButton.getToggleState())
        core->setChannelMode(SignalGenAudioEngine::ChannelMode::Independent);
}

void SignalGeneratorComponent::updateLevelValueLabels()
{
    auto formatDb = [](double v) {
        return juce::String(v, 1);
    };
    
    outputLevel.leftLevelValueLabel.setText(formatDb(outputLevel.leftChannelSlider.getValue()), 
                                           juce::dontSendNotification);
    outputLevel.rightLevelValueLabel.setText(formatDb(outputLevel.rightChannelSlider.getValue()), 
                                            juce::dontSendNotification);
}

//==============================================================================
// Helper Methods
//==============================================================================

void SignalGeneratorComponent::syncFrequencySliders(juce::Slider* source)
{
    float leftFreq = static_cast<float>(manualFrequency.leftFrequencySlider.getValue());
    float rightFreq = static_cast<float>(manualFrequency.rightFrequencySlider.getValue());
    
    if (source == &manualFrequency.leftFrequencySlider)
    {
        if (core->areFrequenciesLocked())
        {
            manualFrequency.rightFrequencySlider.setValue(leftFreq);
            rightFreq = leftFreq;
        }
    }
    else if (source == &manualFrequency.rightFrequencySlider)
    {
        if (core->areFrequenciesLocked())
        {
            manualFrequency.leftFrequencySlider.setValue(rightFreq);
            leftFreq = rightFreq;
        }
    }
    
    core->setManualFrequency(leftFreq, rightFreq);
}

void SignalGeneratorComponent::syncLevelSliders(juce::Slider* source)
{
    float leftLevel = static_cast<float>(outputLevel.leftChannelSlider.getValue());
    float rightLevel = static_cast<float>(outputLevel.rightChannelSlider.getValue());
    
    if (source == &outputLevel.leftChannelSlider)
    {
        if (core->areLevelsLocked())
        {
            outputLevel.rightChannelSlider.setValue(leftLevel, juce::dontSendNotification);
            rightLevel = leftLevel;
        }
    }
    else if (source == &outputLevel.rightChannelSlider)
    {
        if (core->areLevelsLocked())
        {
            outputLevel.leftChannelSlider.setValue(rightLevel, juce::dontSendNotification);
            leftLevel = rightLevel;
        }
    }
    
    core->setOutputLevel(leftLevel, rightLevel);
}