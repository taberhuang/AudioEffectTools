// EQWindow.cpp
#include "EQWindow.h"
#include "../EffectJUCE/IIRFilterProcessor.h"
#include "../EffectJUCE/LimiterProcessor.h"
#include "../Utils.h"

// EQWindow Implementation
EQWindow::EQWindow(IIRFilterProcessor* processor)
    : DocumentWindow("7-Band Parametric EQ", juce::Colours::black, DocumentWindow::closeButton),
      eqProcessor(processor)
{
    setUsingNativeTitleBar(true);
    
    // Create content component
    content = std::make_unique<EQContent>(this);
    content->setSize(MINIMUM_CONTENT_WIDTH, MINIMUM_CONTENT_HEIGHT);
    
    // Always create and use a viewport (most stable)
    viewport = std::make_unique<juce::Viewport>();
    viewport->setViewedComponent(content.get(), false);
    viewport->setScrollBarThickness(15);
    
    // Set scrollbar colors
    viewport->getVerticalScrollBar().setColour(juce::ScrollBar::backgroundColourId, juce::Colour(0xff2a2a2a));
    viewport->getVerticalScrollBar().setColour(juce::ScrollBar::thumbColourId, juce::Colour(0xff6bb6ff));
    viewport->getHorizontalScrollBar().setColour(juce::ScrollBar::backgroundColourId, juce::Colour(0xff2a2a2a));
    viewport->getHorizontalScrollBar().setColour(juce::ScrollBar::thumbColourId, juce::Colour(0xff6bb6ff));
    
    // Set viewport as window content
    setContentNonOwned(viewport.get(), true);
    
    // Get screen size
    int windowWidth = MINIMUM_CONTENT_WIDTH;
    int windowHeight = MINIMUM_CONTENT_HEIGHT;
    int maxWidth = 3840;
    int maxHeight = 2160;
    
    auto displays = juce::Desktop::getInstance().getDisplays();
    if (auto* mainDisplay = displays.getPrimaryDisplay())
    {
        auto screenArea = mainDisplay->userArea;
        maxWidth = screenArea.getWidth();
        maxHeight = screenArea.getHeight();
        
        // Initial window size
        windowWidth = juce::jmin(MINIMUM_CONTENT_WIDTH + 20, maxWidth - 50);
        windowHeight = juce::jmin(MINIMUM_CONTENT_HEIGHT + 20, maxHeight - 50);
    }
    
    // Set window size
    setSize(windowWidth, windowHeight);
    
    // Allow resizing
    setResizable(true, false);
    
    // Set size limits
    setResizeLimits(400, 300, maxWidth, maxHeight);
    
    // Initially decide whether to show scrollbars based on window size
    viewport->setScrollBarsShown(
        windowHeight < MINIMUM_CONTENT_HEIGHT,
        windowWidth < MINIMUM_CONTENT_WIDTH
    );
    
    // Center display
    centreWithSize(getWidth(), getHeight());
    
    setVisible(true);
    startTimerHz(30);
}

EQWindow::~EQWindow()
{
    stopTimer();
    
    if (viewport)
    {
        viewport->setViewedComponent(nullptr, false);
    }
}

void EQWindow::closeButtonPressed()
{
    if (onCloseButtonPressed)
    {
        onCloseButtonPressed();
    }
    else
    {
        // If no callback is set, just hide the window
        setVisible(false);
    }
}

void EQWindow::resized()
{
    DocumentWindow::resized();
    
    if (viewport)
    {
        auto contentArea = getLocalBounds();
        viewport->setBounds(contentArea);
        
        if (content)
        {
            int viewportWidth = viewport->getMaximumVisibleWidth();
            int viewportHeight = viewport->getMaximumVisibleHeight();
            
            bool needsHScroll = viewportWidth < MINIMUM_CONTENT_WIDTH;
            bool needsVScroll = viewportHeight < MINIMUM_CONTENT_HEIGHT;
            
            viewport->setScrollBarsShown(needsVScroll, needsHScroll);
            
            if (!needsHScroll && !needsVScroll)
            {
                content->setSize(viewportWidth, viewportHeight);
            }
            else
            {
                content->setSize(MINIMUM_CONTENT_WIDTH, MINIMUM_CONTENT_HEIGHT);
            }
        }
    }
}

void EQWindow::sliderValueChanged(juce::Slider* slider)
{
    if (!content)
        return;
        
    // Update text fields when sliders change
    for (int i = 0; i < content->eqBands.size(); ++i)
    {
        auto* band = content->eqBands[i];
        if (slider == &band->freqSlider)
        {
            band->freqText.setText(juce::String(static_cast<int>(static_cast<float>(band->freqSlider.getValue()))), false);
        }
        else if (slider == &band->gainSlider)
        {
            band->gainText.setText(juce::String(static_cast<float>(band->gainSlider.getValue()), 1), false);
        }
        else if (slider == &band->qSlider)
        {
            band->qText.setText(juce::String(static_cast<float>(band->qSlider.getValue()), 2), false);
        }
    }
    
    // Handle RMS Limiter sliders
    if (content->rmsLimiter)
    {
        if (slider == &content->rmsLimiter->thSlider)
        {
            content->rmsLimiter->thText.setText(juce::String(static_cast<float>(content->rmsLimiter->thSlider.getValue()), 1), false);
        }
        else if (slider == &content->rmsLimiter->ratioSlider)
        {
            content->rmsLimiter->ratioText.setText(juce::String(static_cast<float>(content->rmsLimiter->ratioSlider.getValue()), 1), false);
        }
        else if (slider == &content->rmsLimiter->atSlider)
        {
            content->rmsLimiter->atText.setText(juce::String(static_cast<int>(static_cast<float>(content->rmsLimiter->atSlider.getValue()))), false);
        }
        else if (slider == &content->rmsLimiter->rtSlider)
        {
            content->rmsLimiter->rtText.setText(juce::String(static_cast<int>(static_cast<float>(content->rmsLimiter->rtSlider.getValue()))), false);
        }
    }
    
    // Handle Peak Limiter sliders
    if (content->peakLimiter)
    {
        if (slider == &content->peakLimiter->thSlider)
        {
            content->peakLimiter->thText.setText(juce::String(static_cast<float>(content->peakLimiter->thSlider.getValue()), 1), false);
        }
        else if (slider == &content->peakLimiter->ratioSlider)
        {
            content->peakLimiter->ratioText.setText(juce::String(static_cast<float>(content->peakLimiter->ratioSlider.getValue()), 1), false);
        }
        else if (slider == &content->peakLimiter->atSlider)
        {
            content->peakLimiter->atText.setText(juce::String(static_cast<int>(static_cast<float>(content->peakLimiter->atSlider.getValue()))), false);
        }
        else if (slider == &content->peakLimiter->rtSlider)
        {
            content->peakLimiter->rtText.setText(juce::String(static_cast<int>(static_cast<float>(content->peakLimiter->rtSlider.getValue()))), false);
        }
    }
    
    updateEQProcessor();
    updateLimiterProcessor();
    
    // Update frequency response display
    if (content && content->frequencyDisplay)
    {
        content->frequencyDisplay->updateResponse();
        content->frequencyDisplay->repaint();
    }
}

void EQWindow::comboBoxChanged(juce::ComboBox* /*comboBox*/)
{
    updateEQProcessor();
    
    if (content && content->frequencyDisplay)
    {
        content->frequencyDisplay->updateResponse();
        content->frequencyDisplay->repaint();
    }
}

void EQWindow::buttonClicked(juce::Button* button)
{
    if (!content)
        return;
        
    if (button == &content->loadButton)
    {
        loadLimiterParams();
        return;
    }
    
    if (button == &content->saveButton)
    {
        saveLimiterParams();
        return;
    }

    if (button == &content->resetButton)
    {
        resetParamsToDefaults();
        return;
    }
    
    // For all other buttons (Enable/Disable)
    updateEQProcessor();
    updateLimiterProcessor();
    
    if (content->frequencyDisplay)
    {
        content->frequencyDisplay->updateResponse();
        content->frequencyDisplay->repaint();
    }
}

void EQWindow::timerCallback()
{
    updateDisplay();
}

void EQWindow::updateEQProcessor()
{
    if (!eqProcessor || !content)
        return;
    
    for (int i = 0; i < content->eqBands.size() && i < 7; ++i)
    {
        auto* band = content->eqBands[i];
        bool enabled = band->enableButton.getToggleState();
        juce::String type = band->typeCombo.getText();
        float freq = static_cast<float>(band->freqSlider.getValue());
        float gain = static_cast<float>(band->gainSlider.getValue());
        float q = static_cast<float>(band->qSlider.getValue());
        
        // Enable/disable controls based on enable button
        band->typeCombo.setEnabled(enabled);
        band->freqSlider.setEnabled(enabled);
        band->gainSlider.setEnabled(enabled);
        band->qSlider.setEnabled(enabled);
        band->freqText.setEnabled(enabled);
        band->gainText.setEnabled(enabled);
        band->qText.setEnabled(enabled);
        
        if (enabled)
        {
            eqProcessor->updateBand(i, type, freq, gain, q);
            if (secondEqProcessor) secondEqProcessor->updateBand(i, type, freq, gain, q);
        }
        else
        {
            eqProcessor->updateBand(i, type, freq, 0.0f, q);
            if (secondEqProcessor) secondEqProcessor->updateBand(i, type, freq, 0.0f, q);
        }
    }
}

void EQWindow::updateLimiterProcessor()
{
    if (!limiterProcessor || !content)
        return;
        
    // Get RMS limiter parameters from UI
    LimiterProcessor::LimiterParams rmsParams;
    rmsParams.threshold = static_cast<float>(content->rmsLimiter->thSlider.getValue());
    rmsParams.ratio = static_cast<float>(content->rmsLimiter->ratioSlider.getValue());
    rmsParams.attack = static_cast<float>(content->rmsLimiter->atSlider.getValue());
    rmsParams.release = static_cast<float>(content->rmsLimiter->rtSlider.getValue());
    rmsParams.enabled = content->rmsLimiter->enableButton.getToggleState();
    
    // Get Peak limiter parameters from UI
    LimiterProcessor::LimiterParams peakParams;
    peakParams.threshold = static_cast<float>(content->peakLimiter->thSlider.getValue());
    peakParams.ratio = static_cast<float>(content->peakLimiter->ratioSlider.getValue());
    peakParams.attack = static_cast<float>(content->peakLimiter->atSlider.getValue());
    peakParams.release = static_cast<float>(content->peakLimiter->rtSlider.getValue());
    peakParams.enabled = content->peakLimiter->enableButton.getToggleState();
    
    // Update the limiter processor with new parameters
    limiterProcessor->setRMSParams(rmsParams);
    limiterProcessor->setPeakParams(peakParams);
}

void EQWindow::updateDisplay()
{
    if (content && content->frequencyDisplay)
    {
        content->frequencyDisplay->updateResponse();
    }
}

void EQWindow::saveLimiterParams()
{
    if (!limiterProcessor || !content)
        return;
        
    // Create JSON object
    juce::var jsonData = juce::var(new juce::DynamicObject());
    
    // Save EQ parameters
    juce::var eqData = juce::var(new juce::DynamicObject());
    for (int i = 0; i < content->eqBands.size() && i < 7; ++i)
    {
        auto* band = content->eqBands[i];
        juce::var bandData = juce::var(new juce::DynamicObject());
        
        bandData.getDynamicObject()->setProperty("enabled", band->enableButton.getToggleState());
        bandData.getDynamicObject()->setProperty("type", band->typeCombo.getText());
        bandData.getDynamicObject()->setProperty("frequency", band->freqSlider.getValue());
        bandData.getDynamicObject()->setProperty("gain", band->gainSlider.getValue());
        bandData.getDynamicObject()->setProperty("q", band->qSlider.getValue());
        
        eqData.getDynamicObject()->setProperty("band" + juce::String(i + 1), bandData);
    }
    jsonData.getDynamicObject()->setProperty("eq", eqData);
    
    // Get current parameters
    auto rmsParams = limiterProcessor->getRMSParams();
    auto peakParams = limiterProcessor->getPeakParams();
    
    // Save RMS limiter parameters
    juce::var rmsData = juce::var(new juce::DynamicObject());
    rmsData.getDynamicObject()->setProperty("threshold", rmsParams.threshold);
    rmsData.getDynamicObject()->setProperty("ratio", rmsParams.ratio);
    rmsData.getDynamicObject()->setProperty("attack", rmsParams.attack);
    rmsData.getDynamicObject()->setProperty("release", rmsParams.release);
    rmsData.getDynamicObject()->setProperty("enabled", rmsParams.enabled);
    
    // Save Peak limiter parameters
    juce::var peakData = juce::var(new juce::DynamicObject());
    peakData.getDynamicObject()->setProperty("threshold", peakParams.threshold);
    peakData.getDynamicObject()->setProperty("ratio", peakParams.ratio);
    peakData.getDynamicObject()->setProperty("attack", peakParams.attack);
    peakData.getDynamicObject()->setProperty("release", peakParams.release);
    peakData.getDynamicObject()->setProperty("enabled", peakParams.enabled);
    
    jsonData.getDynamicObject()->setProperty("rmsLimiter", rmsData);
    jsonData.getDynamicObject()->setProperty("peakLimiter", peakData);
    
    // Save to file
    juce::File saveFile = juce::File::getCurrentWorkingDirectory().getChildFile("eq_limiter_params.json");
    juce::String jsonString = juce::JSON::toString(jsonData);
    
    if (saveFile.replaceWithText(jsonString))
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                                             "Save Successful",
                                             "Parameters saved to: " + Utils::wrapPath(saveFile.getFullPathName(), 40));
    }
    else
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                             "Save Failed",
                                             "Failed to save parameters");
    }
}

void EQWindow::loadLimiterParams()
{
    if (!limiterProcessor || !content)
        return;
        
    juce::File loadFile = juce::File::getCurrentWorkingDirectory().getChildFile("eq_limiter_params.json");
    
    if (!loadFile.existsAsFile())
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                             "Load Failed",
                                             "Parameters file not found");
        return;
    }
    
    juce::String jsonString = loadFile.loadFileAsString();
    juce::var jsonData = juce::JSON::parse(jsonString);
    
    if (!jsonData.isObject())
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                             "Load Failed",
                                             "Invalid JSON format");
        return;
    }
    
    // Load EQ parameters
    if (jsonData.hasProperty("eq"))
    {
        juce::var eqData = jsonData["eq"];
        if (eqData.isObject())
        {
            for (int i = 0; i < content->eqBands.size() && i < 7; ++i)
            {
                juce::String bandKey = "band" + juce::String(i + 1);
                if (eqData.hasProperty(bandKey.toRawUTF8()))
                {
                    juce::var bandData = eqData[bandKey.toRawUTF8()];
                    if (bandData.isObject())
                    {
                        auto* band = content->eqBands[i];
                        
                        bool enabled = bandData.getProperty("enabled", true);
                        juce::String type = bandData.getProperty("type", "PEQ").toString();
                        double freq = bandData.getProperty("frequency", 1000.0);
                        double gain = bandData.getProperty("gain", 0.0);
                        double q = bandData.getProperty("q", 0.7);
                        
                        band->enableButton.setToggleState(enabled, juce::dontSendNotification);
                        
                        for (int j = 1; j <= band->typeCombo.getNumItems(); ++j)
                        {
                            if (band->typeCombo.getItemText(j - 1) == type)
                            {
                                band->typeCombo.setSelectedId(j, juce::dontSendNotification);
                                break;
                            }
                        }
                        
                        band->freqSlider.setValue(freq, juce::dontSendNotification);
                        band->gainSlider.setValue(gain, juce::dontSendNotification);
                        band->qSlider.setValue(q, juce::dontSendNotification);
                        
                        band->freqText.setText(juce::String(static_cast<int>(freq)), juce::dontSendNotification);
                        band->gainText.setText(juce::String(gain, 1), juce::dontSendNotification);
                        band->qText.setText(juce::String(q, 2), juce::dontSendNotification);
                    }
                }
            }
        }
        updateEQProcessor();
    }
    
    // Load RMS limiter parameters
    if (jsonData.hasProperty("rmsLimiter"))
    {
        juce::var rmsData = jsonData["rmsLimiter"];
        if (rmsData.isObject())
        {
            LimiterProcessor::LimiterParams rmsParams;
            rmsParams.threshold = rmsData.getProperty("threshold", 0.0f);
            rmsParams.ratio = rmsData.getProperty("ratio", 10.0f);
            rmsParams.attack = rmsData.getProperty("attack", 50.0f);
            rmsParams.release = rmsData.getProperty("release", 200.0f);
            rmsParams.enabled = rmsData.getProperty("enabled", false);
            
            limiterProcessor->setRMSParams(rmsParams);
            
            content->rmsLimiter->thSlider.setValue(rmsParams.threshold, juce::dontSendNotification);
            content->rmsLimiter->ratioSlider.setValue(rmsParams.ratio, juce::dontSendNotification);
            content->rmsLimiter->atSlider.setValue(rmsParams.attack, juce::dontSendNotification);
            content->rmsLimiter->rtSlider.setValue(rmsParams.release, juce::dontSendNotification);
            content->rmsLimiter->enableButton.setToggleState(rmsParams.enabled, juce::dontSendNotification);
            
            content->rmsLimiter->thText.setText(juce::String(rmsParams.threshold, 1), juce::dontSendNotification);
            content->rmsLimiter->ratioText.setText(juce::String(rmsParams.ratio, 1), juce::dontSendNotification);
            content->rmsLimiter->atText.setText(juce::String(rmsParams.attack, 0), juce::dontSendNotification);
            content->rmsLimiter->rtText.setText(juce::String(rmsParams.release, 0), juce::dontSendNotification);
        }
    }
    
    // Load Peak limiter parameters
    if (jsonData.hasProperty("peakLimiter"))
    {
        juce::var peakData = jsonData["peakLimiter"];
        if (peakData.isObject())
        {
            LimiterProcessor::LimiterParams peakParams;
            peakParams.threshold = peakData.getProperty("threshold", 0.0f);
            peakParams.ratio = peakData.getProperty("ratio", 10.0f);
            peakParams.attack = peakData.getProperty("attack", 50.0f);
            peakParams.release = peakData.getProperty("release", 200.0f);
            peakParams.enabled = peakData.getProperty("enabled", false);
            
            limiterProcessor->setPeakParams(peakParams);
            
            content->peakLimiter->thSlider.setValue(peakParams.threshold, juce::dontSendNotification);
            content->peakLimiter->ratioSlider.setValue(peakParams.ratio, juce::dontSendNotification);
            content->peakLimiter->atSlider.setValue(peakParams.attack, juce::dontSendNotification);
            content->peakLimiter->rtSlider.setValue(peakParams.release, juce::dontSendNotification);
            content->peakLimiter->enableButton.setToggleState(peakParams.enabled, juce::dontSendNotification);
            
            content->peakLimiter->thText.setText(juce::String(peakParams.threshold, 1), juce::dontSendNotification);
            content->peakLimiter->ratioText.setText(juce::String(peakParams.ratio, 1), juce::dontSendNotification);
            content->peakLimiter->atText.setText(juce::String(peakParams.attack, 0), juce::dontSendNotification);
            content->peakLimiter->rtText.setText(juce::String(peakParams.release, 0), juce::dontSendNotification);
        }
    }

    updateEQProcessor();
    updateLimiterProcessor();
    
    // Update FrequencyDisplay
    if (content && content->frequencyDisplay)
    {
        content->frequencyDisplay->updateResponse();
        content->frequencyDisplay->repaint();
    }
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                                         "Load Successful",
                                         "Parameters loaded successfully");
}

void EQWindow::resetParamsToDefaults()
{
    if (!content)
        return;
        
    juce::Array<float> defaultFreqs = {100.0f, 200.0f, 400.0f, 800.0f, 1600.0f, 3200.0f, 6400.0f};
    
    // Reset EQ bands
    for (int i = 0; i < content->eqBands.size() && i < 7; ++i)
    {
        auto* band = content->eqBands[i];
        
        band->enableButton.setToggleState(true, juce::dontSendNotification);
        band->typeCombo.setSelectedId(1, juce::dontSendNotification);
        band->freqSlider.setValue(defaultFreqs[i], juce::dontSendNotification);
        band->gainSlider.setValue(0.0, juce::dontSendNotification);
        band->qSlider.setValue(0.7, juce::dontSendNotification);
        
        band->freqText.setText(juce::String(static_cast<int>(defaultFreqs[i])), juce::dontSendNotification);
        band->gainText.setText("0.0", juce::dontSendNotification);
        band->qText.setText("0.70", juce::dontSendNotification);
    }
    
    // Reset RMS Limiter
    if (content->rmsLimiter)
    {
        content->rmsLimiter->enableButton.setToggleState(false, juce::dontSendNotification);
        content->rmsLimiter->thSlider.setValue(0.0, juce::dontSendNotification);
        content->rmsLimiter->ratioSlider.setValue(10.0, juce::dontSendNotification);
        content->rmsLimiter->atSlider.setValue(50.0, juce::dontSendNotification);
        content->rmsLimiter->rtSlider.setValue(200.0, juce::dontSendNotification);
        
        content->rmsLimiter->thText.setText("0.0", juce::dontSendNotification);
        content->rmsLimiter->ratioText.setText("10.0", juce::dontSendNotification);
        content->rmsLimiter->atText.setText("50", juce::dontSendNotification);
        content->rmsLimiter->rtText.setText("200", juce::dontSendNotification);
    }
    
    // Reset Peak Limiter
    if (content->peakLimiter)
    {
        content->peakLimiter->enableButton.setToggleState(false, juce::dontSendNotification);
        content->peakLimiter->thSlider.setValue(0.0, juce::dontSendNotification);
        content->peakLimiter->ratioSlider.setValue(10.0, juce::dontSendNotification);
        content->peakLimiter->atSlider.setValue(50.0, juce::dontSendNotification);
        content->peakLimiter->rtSlider.setValue(200.0, juce::dontSendNotification);
        
        content->peakLimiter->thText.setText("0.0", juce::dontSendNotification);
        content->peakLimiter->ratioText.setText("10.0", juce::dontSendNotification);
        content->peakLimiter->atText.setText("50", juce::dontSendNotification);
        content->peakLimiter->rtText.setText("200", juce::dontSendNotification);
    }
    
    updateEQProcessor();
    updateLimiterProcessor();
    if (content && content->frequencyDisplay)
    {
        content->frequencyDisplay->updateResponse();
        content->frequencyDisplay->repaint();
    }
}

// EQContent Implementation
EQWindow::EQContent::EQContent(EQWindow* parent)
    : parentWindow(parent)
{
    // Disable accessibility for the entire content component
    setAccessible(false);
    setWantsKeyboardFocus(false);
    
    setupEQBands();
    setupLimiters();
    setupButtons();
    
    // Create frequency display - ensure correct initialization
    if (parentWindow && parentWindow->eqProcessor)
    {
        frequencyDisplay = std::make_unique<FrequencyResponseDisplay>(parentWindow->eqProcessor);
        if (frequencyDisplay)
        {
        frequencyDisplay->setAccessible(false);  // Ensure accessibility is disabled again
            addAndMakeVisible(frequencyDisplay.get());
        }
    }
}

EQWindow::EQContent::~EQContent()
{
    // First remove frequencyDisplay
    if (frequencyDisplay)
    {
        removeChildComponent(frequencyDisplay.get());
        frequencyDisplay.reset();
    }
    
    // Remove listeners from all child components
    if (parentWindow)
    {
        for (auto* band : eqBands)
        {
            if (band)
            {
                band->freqSlider.removeListener(parentWindow);
                band->gainSlider.removeListener(parentWindow);
                band->qSlider.removeListener(parentWindow);
                band->typeCombo.removeListener(parentWindow);
                band->enableButton.removeListener(parentWindow);
            }
        }
        
        if (rmsLimiter)
        {
            rmsLimiter->enableButton.removeListener(parentWindow);
            rmsLimiter->thSlider.removeListener(parentWindow);
            rmsLimiter->ratioSlider.removeListener(parentWindow);
            rmsLimiter->atSlider.removeListener(parentWindow);
            rmsLimiter->rtSlider.removeListener(parentWindow);
        }
        
        if (peakLimiter)
        {
            peakLimiter->enableButton.removeListener(parentWindow);
            peakLimiter->thSlider.removeListener(parentWindow);
            peakLimiter->ratioSlider.removeListener(parentWindow);
            peakLimiter->atSlider.removeListener(parentWindow);
            peakLimiter->rtSlider.removeListener(parentWindow);
        }
        
        loadButton.removeListener(parentWindow);
        saveButton.removeListener(parentWindow);
        resetButton.removeListener(parentWindow);
    }
    
    // Reset look and feel for all components
    auto& customLookAndFeel = CustomSliderLookAndFeel::getInstance();
    for (auto* band : eqBands)
    {
        if (band)
        {
            band->freqSlider.setLookAndFeel(nullptr);
            band->gainSlider.setLookAndFeel(nullptr);
            band->qSlider.setLookAndFeel(nullptr);
            band->freqText.setLookAndFeel(nullptr);
            band->gainText.setLookAndFeel(nullptr);
            band->qText.setLookAndFeel(nullptr);
        }
    }
    
    if (rmsLimiter)
    {
        rmsLimiter->thSlider.setLookAndFeel(nullptr);
        rmsLimiter->ratioSlider.setLookAndFeel(nullptr);
        rmsLimiter->atSlider.setLookAndFeel(nullptr);
        rmsLimiter->rtSlider.setLookAndFeel(nullptr);
        rmsLimiter->thText.setLookAndFeel(nullptr);
        rmsLimiter->ratioText.setLookAndFeel(nullptr);
        rmsLimiter->atText.setLookAndFeel(nullptr);
        rmsLimiter->rtText.setLookAndFeel(nullptr);
    }
    
    if (peakLimiter)
    {
        peakLimiter->thSlider.setLookAndFeel(nullptr);
        peakLimiter->ratioSlider.setLookAndFeel(nullptr);
        peakLimiter->atSlider.setLookAndFeel(nullptr);
        peakLimiter->rtSlider.setLookAndFeel(nullptr);
        peakLimiter->thText.setLookAndFeel(nullptr);
        peakLimiter->ratioText.setLookAndFeel(nullptr);
        peakLimiter->atText.setLookAndFeel(nullptr);
        peakLimiter->rtText.setLookAndFeel(nullptr);
    }
    
    // Clear all child components
    removeAllChildren();
    
    // Clear array
    eqBands.clear();
    
    // Reset parent window pointer
    parentWindow = nullptr;
}

void EQWindow::EQContent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));
}

void EQWindow::EQContent::resized()
{
    auto bounds = getLocalBounds();
    
    // Button area at top
    auto buttonArea = bounds.removeFromTop(40);
    int buttonWidth = 80;
    int buttonHeight = 25;
    int buttonSpacing = 10;
    
    loadButton.setBounds(15, 8, buttonWidth, buttonHeight);
    saveButton.setBounds(15 + buttonWidth + buttonSpacing, 8, buttonWidth, buttonHeight);
    resetButton.setBounds(15 + (buttonWidth + buttonSpacing) * 2, 8, buttonWidth, buttonHeight);

    // Frequency display
    auto displayArea = bounds.removeFromTop(350);
    frequencyDisplay->setBounds(displayArea.reduced(10));
    
    // EQ bands area
    auto bandsArea = bounds.removeFromTop(300);
    bandsArea = bandsArea.reduced(10);
    int availableWidth = bandsArea.getWidth() - 70 - 20;
    int bandWidth = availableWidth / 7;
    
    // First band labels
    if (eqBands.size() > 0)
    {
        auto* firstBand = eqBands[0];
        int labelX = 10;
        int labelY = bandsArea.getY();
        firstBand->typeLabel.setBounds(labelX, labelY + 30, 40, 20);
        firstBand->freqLabel.setBounds(labelX, labelY + 90, 60, 20);
        firstBand->gainLabel.setBounds(labelX, labelY + 150, 60, 20);
        firstBand->qLabel.setBounds(labelX, labelY + 210, 40, 20);
    }
    
    // Position all bands
    for (int i = 0; i < eqBands.size(); ++i)
    {
        auto* band = eqBands[i];
        int x = bandsArea.getX() + 70 + i * bandWidth;
        int y = bandsArea.getY();
        
        if (i > 0)
        {
            band->typeLabel.setVisible(false);
            band->freqLabel.setVisible(false);
            band->gainLabel.setVisible(false);
            band->qLabel.setVisible(false);
        }
        
        band->enableButton.setBounds(x, y, bandWidth - 10, 25);
        band->typeCombo.setBounds(x, y + 30, bandWidth - 10, 25);
        band->freqSlider.setBounds(x, y + 65, bandWidth - 10, 50);
        band->freqText.setBounds(x + 10, y + 115, bandWidth - 30, 20);
        band->gainSlider.setBounds(x, y + 145, bandWidth - 10, 50);
        band->gainText.setBounds(x + 10, y + 195, bandWidth - 30, 20);
        band->qSlider.setBounds(x, y + 225, bandWidth - 10, 50);
        band->qText.setBounds(x + 10, y + 275, bandWidth - 30, 20);
    }
    
    // Limiter sections
    auto limitersArea = bounds.reduced(10);
    int limiterWidth = limitersArea.getWidth() / 2 - 10;
    
    // RMS Limiter (left)
    if (rmsLimiter)
    {
        auto rmsArea = limitersArea.removeFromLeft(limiterWidth);
        
        rmsLimiter->titleLabel.setBounds(rmsArea.getX(), rmsArea.getY(), rmsArea.getWidth(), 20);
        rmsLimiter->enableButton.setBounds(rmsArea.getX() + rmsArea.getWidth() - 80, rmsArea.getY(), 70, 20);
        
        int yPos = rmsArea.getY() + 25;
        int rowHeight = 25;
        int sliderWidth = rmsArea.getWidth() - 100;
        int textWidth = 60;
        
        rmsLimiter->thLabel.setBounds(rmsArea.getX(), yPos, 80, rowHeight);
        rmsLimiter->thSlider.setBounds(rmsArea.getX() + 85, yPos, sliderWidth, rowHeight);
        rmsLimiter->thText.setBounds(rmsArea.getX() + rmsArea.getWidth() - textWidth, yPos, textWidth, rowHeight);
        
        yPos += rowHeight + 5;
        rmsLimiter->ratioLabel.setBounds(rmsArea.getX(), yPos, 80, rowHeight);
        rmsLimiter->ratioSlider.setBounds(rmsArea.getX() + 85, yPos, sliderWidth, rowHeight);
        rmsLimiter->ratioText.setBounds(rmsArea.getX() + rmsArea.getWidth() - textWidth, yPos, textWidth, rowHeight);
        
        yPos += rowHeight + 5;
        rmsLimiter->atLabel.setBounds(rmsArea.getX(), yPos, 80, rowHeight);
        rmsLimiter->atSlider.setBounds(rmsArea.getX() + 85, yPos, sliderWidth, rowHeight);
        rmsLimiter->atText.setBounds(rmsArea.getX() + rmsArea.getWidth() - textWidth, yPos, textWidth, rowHeight);
        
        yPos += rowHeight + 5;
        rmsLimiter->rtLabel.setBounds(rmsArea.getX(), yPos, 80, rowHeight);
        rmsLimiter->rtSlider.setBounds(rmsArea.getX() + 85, yPos, sliderWidth, rowHeight);
        rmsLimiter->rtText.setBounds(rmsArea.getX() + rmsArea.getWidth() - textWidth, yPos, textWidth, rowHeight);
    }
    
    // Peak Limiter (right)
    if (peakLimiter)
    {
        limitersArea.removeFromLeft(20); // gap
        auto peakArea = limitersArea;
        
        peakLimiter->titleLabel.setBounds(peakArea.getX(), peakArea.getY(), peakArea.getWidth(), 20);
        peakLimiter->enableButton.setBounds(peakArea.getX() + peakArea.getWidth() - 80, peakArea.getY(), 70, 20);
        
        int yPos = peakArea.getY() + 25;
        int rowHeight = 25;
        int sliderWidth = peakArea.getWidth() - 100;
        int textWidth = 60;
        
        peakLimiter->thLabel.setBounds(peakArea.getX(), yPos, 80, rowHeight);
        peakLimiter->thSlider.setBounds(peakArea.getX() + 85, yPos, sliderWidth, rowHeight);
        peakLimiter->thText.setBounds(peakArea.getX() + peakArea.getWidth() - textWidth, yPos, textWidth, rowHeight);
        
        yPos += rowHeight + 5;
        peakLimiter->ratioLabel.setBounds(peakArea.getX(), yPos, 80, rowHeight);
        peakLimiter->ratioSlider.setBounds(peakArea.getX() + 85, yPos, sliderWidth, rowHeight);
        peakLimiter->ratioText.setBounds(peakArea.getX() + peakArea.getWidth() - textWidth, yPos, textWidth, rowHeight);
        
        yPos += rowHeight + 5;
        peakLimiter->atLabel.setBounds(peakArea.getX(), yPos, 80, rowHeight);
        peakLimiter->atSlider.setBounds(peakArea.getX() + 85, yPos, sliderWidth, rowHeight);
        peakLimiter->atText.setBounds(peakArea.getX() + peakArea.getWidth() - textWidth, yPos, textWidth, rowHeight);
        
        yPos += rowHeight + 5;
        peakLimiter->rtLabel.setBounds(peakArea.getX(), yPos, 80, rowHeight);
        peakLimiter->rtSlider.setBounds(peakArea.getX() + 85, yPos, sliderWidth, rowHeight);
        peakLimiter->rtText.setBounds(peakArea.getX() + peakArea.getWidth() - textWidth, yPos, textWidth, rowHeight);
    }
}

void EQWindow::EQContent::setupEQBands()
{
    // Get singleton LookAndFeel
    auto& customLookAndFeel = CustomSliderLookAndFeel::getInstance();
    
    juce::StringArray filterTypes = {"PEQ", "LSH", "HSH", "LPF", "HPF"};
    juce::Array<float> defaultFreqs = {100.0f, 200.0f, 400.0f, 800.0f, 1600.0f, 3200.0f, 6400.0f};
    
    for (int i = 0; i < 7; ++i)
    {
        auto* band = new EQBand();
        
        band->enableButton.setButtonText("Enable");
        band->enableButton.addListener(parentWindow);
        
        band->typeCombo.addItemList(filterTypes, 1);
        band->typeCombo.setSelectedId(1);
        band->typeCombo.addListener(parentWindow);
        
        band->freqSlider.setRange(20.0, 20000.0, 1.0);
        band->freqSlider.setSkewFactorFromMidPoint(1000.0);
        band->freqSlider.setValue(defaultFreqs[i]);
        band->freqSlider.addListener(parentWindow);
        
        band->gainSlider.setRange(-12.0, 12.0, 0.1);
        band->gainSlider.setValue(0.0);
        band->gainSlider.addListener(parentWindow);
        
        band->qSlider.setRange(0.25, 10.0, 0.01);
        band->qSlider.setValue(0.7);
        band->qSlider.addListener(parentWindow);
        
        band->freqText.setText(juce::String(static_cast<int>(defaultFreqs[i])), false);
        band->gainText.setText("0.0", false);
        band->qText.setText("0.70", false);
        
        band->setupCallbacks();
        
        // Apply custom look and feel - Use singleton
        band->freqSlider.setLookAndFeel(&customLookAndFeel);
        band->gainSlider.setLookAndFeel(&customLookAndFeel);
        band->qSlider.setLookAndFeel(&customLookAndFeel);
        band->freqText.setLookAndFeel(&customLookAndFeel);
        band->gainText.setLookAndFeel(&customLookAndFeel);
        band->qText.setLookAndFeel(&customLookAndFeel);
        
        // Make visible
        addAndMakeVisible(band->enableButton);
        addAndMakeVisible(band->typeLabel);
        addAndMakeVisible(band->freqLabel);
        addAndMakeVisible(band->gainLabel);
        addAndMakeVisible(band->qLabel);
        addAndMakeVisible(band->typeCombo);
        addAndMakeVisible(band->freqSlider);
        addAndMakeVisible(band->gainSlider);
        addAndMakeVisible(band->qSlider);
        addAndMakeVisible(band->freqText);
        addAndMakeVisible(band->gainText);
        addAndMakeVisible(band->qText);
        
        eqBands.add(band);
    }
}

void EQWindow::EQContent::setupLimiters()
{
    // Get singleton LookAndFeel
    auto& customLookAndFeel = CustomSliderLookAndFeel::getInstance();
    
    // RMS Limiter
    rmsLimiter = std::make_unique<LimiterSection>("RMS Limiter:");
    rmsLimiter->setupCallbacks();
    
    // Apply custom look and feel - Use singleton
    rmsLimiter->thSlider.setLookAndFeel(&customLookAndFeel);
    rmsLimiter->ratioSlider.setLookAndFeel(&customLookAndFeel);
    rmsLimiter->atSlider.setLookAndFeel(&customLookAndFeel);
    rmsLimiter->rtSlider.setLookAndFeel(&customLookAndFeel);
    rmsLimiter->thText.setLookAndFeel(&customLookAndFeel);
    rmsLimiter->ratioText.setLookAndFeel(&customLookAndFeel);
    rmsLimiter->atText.setLookAndFeel(&customLookAndFeel);
    rmsLimiter->rtText.setLookAndFeel(&customLookAndFeel);
    
    // Add components
    addAndMakeVisible(rmsLimiter->titleLabel);
    addAndMakeVisible(rmsLimiter->enableButton);
    addAndMakeVisible(rmsLimiter->thLabel);
    addAndMakeVisible(rmsLimiter->ratioLabel);
    addAndMakeVisible(rmsLimiter->atLabel);
    addAndMakeVisible(rmsLimiter->rtLabel);
    addAndMakeVisible(rmsLimiter->thSlider);
    addAndMakeVisible(rmsLimiter->ratioSlider);
    addAndMakeVisible(rmsLimiter->atSlider);
    addAndMakeVisible(rmsLimiter->rtSlider);
    addAndMakeVisible(rmsLimiter->thText);
    addAndMakeVisible(rmsLimiter->ratioText);
    addAndMakeVisible(rmsLimiter->atText);
    addAndMakeVisible(rmsLimiter->rtText);
    
    // Add listeners
    rmsLimiter->enableButton.addListener(parentWindow);
    rmsLimiter->thSlider.addListener(parentWindow);
    rmsLimiter->ratioSlider.addListener(parentWindow);
    rmsLimiter->atSlider.addListener(parentWindow);
    rmsLimiter->rtSlider.addListener(parentWindow);
    
    // Peak Limiter
    peakLimiter = std::make_unique<LimiterSection>("Peak Limiter:");
    peakLimiter->setLinearSliders();
    peakLimiter->setupCallbacks();
    
    // Apply custom look and feel - Use singleton
    peakLimiter->thSlider.setLookAndFeel(&customLookAndFeel);
    peakLimiter->ratioSlider.setLookAndFeel(&customLookAndFeel);
    peakLimiter->atSlider.setLookAndFeel(&customLookAndFeel);
    peakLimiter->rtSlider.setLookAndFeel(&customLookAndFeel);
    peakLimiter->thText.setLookAndFeel(&customLookAndFeel);
    peakLimiter->ratioText.setLookAndFeel(&customLookAndFeel);
    peakLimiter->atText.setLookAndFeel(&customLookAndFeel);
    peakLimiter->rtText.setLookAndFeel(&customLookAndFeel);
    
    // Add components
    addAndMakeVisible(peakLimiter->titleLabel);
    addAndMakeVisible(peakLimiter->enableButton);
    addAndMakeVisible(peakLimiter->thLabel);
    addAndMakeVisible(peakLimiter->ratioLabel);
    addAndMakeVisible(peakLimiter->atLabel);
    addAndMakeVisible(peakLimiter->rtLabel);
    addAndMakeVisible(peakLimiter->thSlider);
    addAndMakeVisible(peakLimiter->ratioSlider);
    addAndMakeVisible(peakLimiter->atSlider);
    addAndMakeVisible(peakLimiter->rtSlider);
    addAndMakeVisible(peakLimiter->thText);
    addAndMakeVisible(peakLimiter->ratioText);
    addAndMakeVisible(peakLimiter->atText);
    addAndMakeVisible(peakLimiter->rtText);
    
    // Add listeners
    peakLimiter->enableButton.addListener(parentWindow);
    peakLimiter->thSlider.addListener(parentWindow);
    peakLimiter->ratioSlider.addListener(parentWindow);
    peakLimiter->atSlider.addListener(parentWindow);
    peakLimiter->rtSlider.addListener(parentWindow);
}

void EQWindow::EQContent::setupButtons()
{
    loadButton.setButtonText("Load");
    saveButton.setButtonText("Save");
    resetButton.setButtonText("Reset");
    
    loadButton.addListener(parentWindow);
    saveButton.addListener(parentWindow);
    resetButton.addListener(parentWindow);
    
    addAndMakeVisible(loadButton);
    addAndMakeVisible(saveButton);
    addAndMakeVisible(resetButton);
}
