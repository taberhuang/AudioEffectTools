#include "MainComponent.h"

void MainComponent::resized()
{
    const int margin = 10;
    const int sliderHeight = 80;
    const int sliderWidth = 80;
    const int labelHeight = 20;
    const int toggleHeight = 25;
    const int controlHeight = 30;
    
    const int leftPanelWidth = getWidth() / 2;
    const int rightPanelWidth = getWidth() / 2;
    
    // Reduce top control height to leave more space at the bottom
    const int topControlsHeight = 110;  // Changed from 120 to 110
    
    // Calculate available bottom space
    const int availableBottomHeight = getHeight() - topControlsHeight - margin;
    // Allocate height for each analyzer (including labels)
    const int analyzerSectionHeight = (availableBottomHeight - margin) / 2;
    // Actual analyzer height (subtracting label and toggle height)
    const int analyzerHeight = analyzerSectionHeight - labelHeight - toggleHeight - 2 * margin;
    
    int controlX = margin;
    int controlY = margin;
    
    // === Top Left: Effect Selector and Parameters ===
    effectSelectorLabel.setBounds(controlX, controlY, 100, labelHeight);
    effectSelector.setBounds(controlX, effectSelectorLabel.getBottom(), 100, controlHeight);
    effectEnableToggle.setBounds(controlX, effectSelector.getBottom() + 5, 100, toggleHeight);
    
    int gainX = effectSelector.getRight() + margin;
    gainLabel.setBounds(gainX, controlY, sliderWidth, labelHeight);
    gainSlider.setBounds(gainX, gainLabel.getBottom(), sliderWidth, sliderHeight);
    
    // Parameter control positions
    int param1X = gainSlider.getRight() + margin;
    int param2X = param1X + sliderWidth + margin;
    int param3X = param2X + sliderWidth + margin;
    int param4X = param3X + sliderWidth + margin;
    int param5X = param4X + sliderWidth + margin;
    
    // Layout all parameter controls (keeping original code)
    delayLabel.setBounds(param1X, controlY, sliderWidth, labelHeight);
    delaySlider.setBounds(param1X, delayLabel.getBottom(), sliderWidth, sliderHeight);
    feedbackLabel.setBounds(param2X, controlY, sliderWidth, labelHeight);
    feedbackSlider.setBounds(param2X, feedbackLabel.getBottom(), sliderWidth, sliderHeight);
    
    flangerRateLabel.setBounds(param1X, controlY, sliderWidth, labelHeight);
    flangerRateSlider.setBounds(param1X, flangerRateLabel.getBottom(), sliderWidth, sliderHeight);
    flangerDepthLabel.setBounds(param2X, controlY, sliderWidth, labelHeight);
    flangerDepthSlider.setBounds(param2X, flangerDepthLabel.getBottom(), sliderWidth, sliderHeight);
    
    plateDecayLabel.setBounds(param1X, controlY, sliderWidth, labelHeight);
    plateDecaySlider.setBounds(param1X, plateDecayLabel.getBottom(), sliderWidth, sliderHeight);
    plateToneLabel.setBounds(param2X, controlY, sliderWidth, labelHeight);
    plateToneSlider.setBounds(param2X, plateToneLabel.getBottom(), sliderWidth, sliderHeight);
    
    pitchSemitoneLabel.setBounds(param1X, controlY, sliderWidth, labelHeight);
    pitchSemitoneSlider.setBounds(param1X, pitchSemitoneLabel.getBottom(), sliderWidth, sliderHeight);
    pitchDetuneLabel.setBounds(param2X, controlY, sliderWidth, labelHeight);
    pitchDetuneSlider.setBounds(param2X, pitchDetuneLabel.getBottom(), sliderWidth, sliderHeight);
    
    // Cathedral reverb controls
    cathedralDecayLabel.setBounds(param1X, controlY, sliderWidth, labelHeight);
    cathedralDecaySlider.setBounds(param1X, cathedralDecayLabel.getBottom(), sliderWidth, sliderHeight);
    cathedralToneLabel.setBounds(param2X, controlY, sliderWidth, labelHeight);
    cathedralToneSlider.setBounds(param2X, cathedralToneLabel.getBottom(), sliderWidth, sliderHeight);
    
    // JUCE effect control layout (keeping original code)
    juceDelayTimeLabel.setBounds(param1X, controlY, sliderWidth, labelHeight);
    juceDelayTimeSlider.setBounds(param1X, juceDelayTimeLabel.getBottom(), sliderWidth, sliderHeight);
    juceDelayFeedbackLabel.setBounds(param2X, controlY, sliderWidth, labelHeight);
    juceDelayFeedbackSlider.setBounds(param2X, juceDelayFeedbackLabel.getBottom(), sliderWidth, sliderHeight);
    juceDelayMixLabel.setBounds(param3X, controlY, sliderWidth, labelHeight);
    juceDelayMixSlider.setBounds(param3X, juceDelayMixLabel.getBottom(), sliderWidth, sliderHeight);
    
    juceChorusRateLabel.setBounds(param1X, controlY, sliderWidth, labelHeight);
    juceChorusRateSlider.setBounds(param1X, juceChorusRateLabel.getBottom(), sliderWidth, sliderHeight);
    juceChorusDepthLabel.setBounds(param2X, controlY, sliderWidth, labelHeight);
    juceChorusDepthSlider.setBounds(param2X, juceChorusDepthLabel.getBottom(), sliderWidth, sliderHeight);
    juceChorusCentreDelayLabel.setBounds(param3X, controlY, sliderWidth, labelHeight);
    juceChorusCentreDelaySlider.setBounds(param3X, juceChorusCentreDelayLabel.getBottom(), sliderWidth, sliderHeight);
    juceChorusFeedbackLabel.setBounds(param4X, controlY, sliderWidth, labelHeight);
    juceChorusFeedbackSlider.setBounds(param4X, juceChorusFeedbackLabel.getBottom(), sliderWidth, sliderHeight);
    juceChorusMixLabel.setBounds(param5X, controlY, sliderWidth, labelHeight);
    juceChorusMixSlider.setBounds(param5X, juceChorusMixLabel.getBottom(), sliderWidth, sliderHeight);
    
    juceReverbRoomSizeLabel.setBounds(param1X, controlY, sliderWidth, labelHeight);
    juceReverbRoomSizeSlider.setBounds(param1X, juceReverbRoomSizeLabel.getBottom(), sliderWidth, sliderHeight);
    juceReverbDampingLabel.setBounds(param2X, controlY, sliderWidth, labelHeight);
    juceReverbDampingSlider.setBounds(param2X, juceReverbDampingLabel.getBottom(), sliderWidth, sliderHeight);
    juceReverbWetLevelLabel.setBounds(param3X, controlY, sliderWidth, labelHeight);
    juceReverbWetLevelSlider.setBounds(param3X, juceReverbWetLevelLabel.getBottom(), sliderWidth, sliderHeight);
    juceReverbDryLevelLabel.setBounds(param4X, controlY, sliderWidth, labelHeight);
    juceReverbDryLevelSlider.setBounds(param4X, juceReverbDryLevelLabel.getBottom(), sliderWidth, sliderHeight);
    juceReverbWidthLabel.setBounds(param5X, controlY, sliderWidth, labelHeight);
    juceReverbWidthSlider.setBounds(param5X, juceReverbWidthLabel.getBottom(), sliderWidth, sliderHeight);
    
    jucePhaserRateLabel.setBounds(param1X, controlY, sliderWidth, labelHeight);
    jucePhaserRateSlider.setBounds(param1X, jucePhaserRateLabel.getBottom(), sliderWidth, sliderHeight);
    jucePhaserDepthLabel.setBounds(param2X, controlY, sliderWidth, labelHeight);
    jucePhaserDepthSlider.setBounds(param2X, jucePhaserDepthLabel.getBottom(), sliderWidth, sliderHeight);
    jucePhaserCentreFreqLabel.setBounds(param3X, controlY, sliderWidth, labelHeight);
    jucePhaserCentreFreqSlider.setBounds(param3X, jucePhaserCentreFreqLabel.getBottom(), sliderWidth, sliderHeight);
    jucePhaserFeedbackLabel.setBounds(param4X, controlY, sliderWidth, labelHeight);
    jucePhaserFeedbackSlider.setBounds(param4X, jucePhaserFeedbackLabel.getBottom(), sliderWidth, sliderHeight);
    jucePhaserMixLabel.setBounds(param5X, controlY, sliderWidth, labelHeight);
    jucePhaserMixSlider.setBounds(param5X, jucePhaserMixLabel.getBottom(), sliderWidth, sliderHeight);
    
    juceCompressorThresholdLabel.setBounds(param1X, controlY, sliderWidth, labelHeight);
    juceCompressorThresholdSlider.setBounds(param1X, juceCompressorThresholdLabel.getBottom(), sliderWidth, sliderHeight);
    juceCompressorRatioLabel.setBounds(param2X, controlY, sliderWidth, labelHeight);
    juceCompressorRatioSlider.setBounds(param2X, juceCompressorRatioLabel.getBottom(), sliderWidth, sliderHeight);
    juceCompressorAttackLabel.setBounds(param3X, controlY, sliderWidth, labelHeight);
    juceCompressorAttackSlider.setBounds(param3X, juceCompressorAttackLabel.getBottom(), sliderWidth, sliderHeight);
    juceCompressorReleaseLabel.setBounds(param4X, controlY, sliderWidth, labelHeight);
    juceCompressorReleaseSlider.setBounds(param4X, juceCompressorReleaseLabel.getBottom(), sliderWidth, sliderHeight);
    
    juceDistortionDriveLabel.setBounds(param1X, controlY, sliderWidth, labelHeight);
    juceDistortionDriveSlider.setBounds(param1X, juceDistortionDriveLabel.getBottom(), sliderWidth, sliderHeight);
    juceDistortionRangeLabel.setBounds(param2X, controlY, sliderWidth, labelHeight);
    juceDistortionRangeSlider.setBounds(param2X, juceDistortionRangeLabel.getBottom(), sliderWidth, sliderHeight);
    juceDistortionBlendLabel.setBounds(param3X, controlY, sliderWidth, labelHeight);
    juceDistortionBlendSlider.setBounds(param3X, juceDistortionBlendLabel.getBottom(), sliderWidth, sliderHeight);

    // Noise Suppression controls
    noiseSuppressionVadThresholdLabel.setBounds(param1X, controlY, sliderWidth, labelHeight);
    noiseSuppressionVadThresholdSlider.setBounds(param1X, noiseSuppressionVadThresholdLabel.getBottom(), sliderWidth, sliderHeight);
    noiseSuppressionVadGracePeriodLabel.setBounds(param2X, controlY, sliderWidth, labelHeight);
    noiseSuppressionVadGracePeriodSlider.setBounds(param2X, noiseSuppressionVadGracePeriodLabel.getBottom(), sliderWidth, sliderHeight);
    noiseSuppressionVadRetroactiveGracePeriodLabel.setBounds(param3X, controlY, sliderWidth, labelHeight);
    noiseSuppressionVadRetroactiveGracePeriodSlider.setBounds(param3X, noiseSuppressionVadRetroactiveGracePeriodLabel.getBottom(), sliderWidth, sliderHeight);

    // === Top Right: Control Buttons and Device Selectors ===
    int deviceX = leftPanelWidth + margin;
    int deviceY = margin;
    
    // Buttons
    bypassToggle.setBounds(deviceX, deviceY, 100, toggleHeight);
    recordButton.setBounds(bypassToggle.getRight() + margin, deviceY, 120, toggleHeight);
    eqButton.setBounds(recordButton.getRight() + margin, deviceY, 80, toggleHeight);
    sweepTestButton.setBounds(eqButton.getRight() + 10, eqButton.getY(), 
                             eqButton.getWidth(), eqButton.getHeight());
    // Device Selectors
    int deviceSelectorY = bypassToggle.getBottom() + margin;
    int comboBoxWidth = (rightPanelWidth - 2 * margin) / 2;
    
    inputDeviceLabel.setBounds(deviceX, deviceSelectorY, 150, labelHeight);
    inputDeviceSelector.setBounds(deviceX, inputDeviceLabel.getBottom(), comboBoxWidth - margin, controlHeight);
    
    int outputDeviceX = deviceX + comboBoxWidth;
    outputDeviceLabel.setBounds(outputDeviceX, deviceSelectorY, 150, labelHeight);
    outputDeviceSelector.setBounds(outputDeviceX, outputDeviceLabel.getBottom(), comboBoxWidth - margin, controlHeight);
    
    // === Bottom Layout: Waveform and Spectrum Analyzers ===
    int waveformY = topControlsHeight + margin;
    
    // Input Waveform Analyzer
    inputWaveformLabel.setBounds(margin, waveformY, 150, labelHeight);
    showInputWaveformToggle.setBounds(inputWaveformLabel.getRight() + margin, waveformY, 150, toggleHeight);
    int inputWaveformAnalyzerY = juce::jmax(inputWaveformLabel.getBottom(), showInputWaveformToggle.getBottom()) + margin;
    inputWaveformAnalyzer.setBounds(margin, inputWaveformAnalyzerY, 
                                   leftPanelWidth - 2 * margin, analyzerHeight);
    
    // Output Waveform Analyzer
    int outputWaveformY = inputWaveformAnalyzer.getBottom() + margin;
    outputWaveformLabel.setBounds(margin, outputWaveformY, 150, labelHeight);
    showOutputWaveformToggle.setBounds(outputWaveformLabel.getRight() + margin, outputWaveformY, 150, toggleHeight);
    int outputWaveformAnalyzerY = juce::jmax(outputWaveformLabel.getBottom(), showOutputWaveformToggle.getBottom()) + margin;
    outputWaveformAnalyzer.setBounds(margin, outputWaveformAnalyzerY, 
                                    leftPanelWidth - 2 * margin, analyzerHeight);
    
    // Spectrum Analyzer
    int spectrumX = leftPanelWidth + margin;
    int spectrumY = topControlsHeight + margin;
    
    // Input Spectrum Analyzer
    inputSpectrumLabel.setBounds(spectrumX, spectrumY, 150, labelHeight);
    showInputSpectrumToggle.setBounds(inputSpectrumLabel.getRight() + margin, spectrumY, 150, toggleHeight);
    int inputSpectrumAnalyzerY = juce::jmax(inputSpectrumLabel.getBottom(), showInputSpectrumToggle.getBottom()) + margin;
    inputSpectrumAnalyzer.setBounds(spectrumX, inputSpectrumAnalyzerY, 
                                   rightPanelWidth - 2 * margin, analyzerHeight);
    
    // Output Spectrum Analyzer
    int outputSpectrumY = inputSpectrumAnalyzer.getBottom() + margin;
    outputSpectrumLabel.setBounds(spectrumX, outputSpectrumY, 150, labelHeight);
    showOutputSpectrumToggle.setBounds(outputSpectrumLabel.getRight() + margin, outputSpectrumY, 150, toggleHeight);
    int outputSpectrumAnalyzerY = juce::jmax(outputSpectrumLabel.getBottom(), showOutputSpectrumToggle.getBottom()) + margin;
    outputSpectrumAnalyzer.setBounds(spectrumX, outputSpectrumAnalyzerY, 
                                    rightPanelWidth - 2 * margin, analyzerHeight);
}