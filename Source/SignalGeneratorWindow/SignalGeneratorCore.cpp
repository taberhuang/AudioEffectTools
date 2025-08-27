#include "SignalGeneratorCore.h"

//==============================================================================
// SignalGeneratorCore Implementation
//==============================================================================

SignalGeneratorCore::SignalGeneratorCore(SignalGenAudioEngine* engine)
    : audioEngine(engine)
{
    jassert(audioEngine != nullptr);
}

void SignalGeneratorCore::setFrequencyRange(float start, float end)
{
    if (audioEngine)
    {
        start = clampFrequency(start);
        end = clampFrequency(end);
        if (start > end) std::swap(start, end);
        audioEngine->setFrequencyRange(start, end);
    }
}

void SignalGeneratorCore::setManualFrequency(float left, float right)
{
    if (audioEngine)
    {
        lastLeftFrequency = left;
        lastRightFrequency = right;
        
        if (lockFrequencies)
        {
            audioEngine->setManualFrequency(left, left);
            lastRightFrequency = left;
        }
        else
        {
            audioEngine->setManualFrequency(left, right);
        }
    }
}

void SignalGeneratorCore::setLockFrequencies(bool lock)
{
    lockFrequencies = lock;
    if (audioEngine)
    {
        audioEngine->setLockChannels(lock);
        if (lock)
        {
            setManualFrequency(lastLeftFrequency, lastLeftFrequency);
        }
    }
}

float SignalGeneratorCore::getGeometricMean(float start, float end) const
{
    return std::sqrt(start * end);
}

float SignalGeneratorCore::clampFrequency(float freq) const
{
    return juce::jlimit(SignalGenConstants::MIN_FREQUENCY, 
                       SignalGenConstants::MAX_FREQUENCY, freq);
}

void SignalGeneratorCore::setOutputLevel(float left, float right)
{
    if (audioEngine)
    {
        lastLeftLevel = left;
        lastRightLevel = right;
        
        if (lockLevels)
        {
            audioEngine->setLeftChannelLevel(left);
            audioEngine->setRightChannelLevel(left);
            lastRightLevel = left;
        }
        else
        {
            audioEngine->setLeftChannelLevel(left);
            audioEngine->setRightChannelLevel(right);
        }
    }
}

void SignalGeneratorCore::setLockLevels(bool lock)
{
    lockLevels = lock;
    if (lock)
    {
        setOutputLevel(lastLeftLevel, lastLeftLevel);
    }
}

void SignalGeneratorCore::setChannelEnabled(bool leftEnabled, bool rightEnabled)
{
    if (audioEngine)
    {
        audioEngine->setLeftEnabled(leftEnabled);
        audioEngine->setRightEnabled(rightEnabled);
    }
}

void SignalGeneratorCore::setWaveform(SignalGenAudioEngine::Waveform waveform)
{
    if (audioEngine)
        audioEngine->setWaveform(waveform);
}

void SignalGeneratorCore::setDutyCycle(float dutyCycle)
{
    if (audioEngine)
        audioEngine->setDutyCycle(dutyCycle);
}

void SignalGeneratorCore::setSweepMode(SignalGenAudioEngine::SweepMode mode)
{
    if (audioEngine)
        audioEngine->setSweepMode(mode);
}

void SignalGeneratorCore::setSweepSpeed(SignalGenAudioEngine::SweepSpeed speed)
{
    if (audioEngine)
        audioEngine->setSweepSpeed(speed);
}

void SignalGeneratorCore::setSweepSpeedValue(float value)
{
    if (audioEngine)
        audioEngine->setSweepSpeedValue(value);
}

void SignalGeneratorCore::setChannelMode(SignalGenAudioEngine::ChannelMode mode)
{
    if (audioEngine)
        audioEngine->setChannelMode(mode);
}

void SignalGeneratorCore::start()
{
    if (audioEngine)
        audioEngine->start();
}

void SignalGeneratorCore::stop()
{
    if (audioEngine)
        audioEngine->stop();
}

bool SignalGeneratorCore::isRunning() const
{
    return audioEngine ? audioEngine->isRunning() : false;
}

//==============================================================================
// LayoutHelper Implementation
//==============================================================================

void LayoutHelper::layoutVerticalButtons(juce::GroupComponent& group, 
                                        const std::vector<juce::Component*>& buttons,
                                        int margin)
{
    auto flexBox = createFlexBox(juce::FlexBox::Direction::column);
    addToFlexBox(flexBox, buttons);
    performLayout(flexBox, group.getBounds().reduced(margin, SignalGenConstants::GROUP_HEADER_HEIGHT));
}

void LayoutHelper::layoutHorizontalButtons(juce::GroupComponent& group,
                                          const std::vector<juce::Component*>& buttons,
                                          int margin)
{
    auto flexBox = createFlexBox(juce::FlexBox::Direction::row);
    addToFlexBox(flexBox, buttons);
    performLayout(flexBox, group.getBounds().reduced(margin, SignalGenConstants::GROUP_HEADER_HEIGHT));
}

juce::FlexBox LayoutHelper::createFlexBox(juce::FlexBox::Direction direction,
                                         juce::FlexBox::JustifyContent justifyContent)
{
    juce::FlexBox flexBox;
    flexBox.flexDirection = direction;
    flexBox.justifyContent = justifyContent;
    return flexBox;
}

void LayoutHelper::addToFlexBox(juce::FlexBox& flexBox, 
                               const std::vector<juce::Component*>& components,
                               float flex)
{
    for (auto* comp : components)
    {
        if (comp != nullptr)
            flexBox.items.add(juce::FlexItem(*comp).withFlex(flex));
    }
}

void LayoutHelper::performLayout(juce::FlexBox& flexBox, juce::Rectangle<int> bounds)
{
    flexBox.performLayout(bounds);
}

//==============================================================================
// ComponentFactory Implementation
//==============================================================================

std::vector<std::unique_ptr<juce::ToggleButton>> ComponentFactory::createRadioButtonGroup(
    const std::vector<juce::String>& buttonNames,
    int radioGroupId)
{
    std::vector<std::unique_ptr<juce::ToggleButton>> buttons;
    buttons.reserve(buttonNames.size());
    
    for (const auto& name : buttonNames)
    {
        auto button = std::make_unique<juce::ToggleButton>(name);
        button->setRadioGroupId(radioGroupId);
        buttons.push_back(std::move(button));
    }
    
    return buttons;
}

std::unique_ptr<juce::Slider> ComponentFactory::createSlider(
    juce::Slider::SliderStyle style,
    double minValue, double maxValue, double defaultValue,
    double step,
    juce::Slider::TextEntryBoxPosition textBoxPosition,
    bool readOnly,
    int textBoxWidth,
    int textBoxHeight)
{
    auto slider = std::make_unique<juce::Slider>();
    slider->setSliderStyle(style);
    slider->setRange(minValue, maxValue, step);
    slider->setValue(defaultValue);
    
    if (textBoxPosition != juce::Slider::NoTextBox)
    {
        slider->setTextBoxStyle(textBoxPosition, readOnly, textBoxWidth, textBoxHeight);
    }
    
    return slider;
}

std::unique_ptr<juce::GroupComponent> ComponentFactory::createGroup(const juce::String& title)
{
    auto group = std::make_unique<juce::GroupComponent>();
    group->setText(title);
    return group;
}

std::unique_ptr<juce::Label> ComponentFactory::createLabel(const juce::String& text,
                                                           juce::Justification justification)
{
    auto label = std::make_unique<juce::Label>("", text);
    label->setJustificationType(justification);
    return label;
}

std::unique_ptr<juce::TextEditor> ComponentFactory::createNumericEditor(const juce::String& defaultText,
                                                                       int maxLength)
{
    auto editor = std::make_unique<juce::TextEditor>();
    editor->setText(defaultText);
    editor->setInputRestrictions(maxLength, "0123456789.");
    return editor;
}

void ComponentFactory::styleButton(juce::Button& button, 
                                  const juce::Colour& colour,
                                  bool isMomentary)
{
    if (colour != juce::Colours::transparentBlack)
    {
        // 同时设置普通与选中背景色，避免切换 toggle 状态后颜色失效
        button.setColour(juce::TextButton::buttonColourId, colour);
        button.setColour(juce::TextButton::buttonOnColourId, colour);
    }
    
    if (auto* textButton = dynamic_cast<juce::TextButton*>(&button))
    {
        textButton->setClickingTogglesState(!isMomentary);
    }
}

//==============================================================================
// MeterLookAndFeel Implementation
//==============================================================================

void MeterLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                       float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
                                       const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style != juce::Slider::LinearBarVertical)
    {
        LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, 0.0f, 0.0f, style, slider);
        return;
    }

    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
    
    // Background
    g.setColour(juce::Colours::black.withAlpha(0.85f));
    g.fillRect(bounds);
    
    // Calculate fill proportion
    const float proportion = slider.valueToProportionOfLength(slider.getValue());
    
    // Draw meter bar
    drawMeterBar(g, bounds, proportion);
    
    // Border
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(bounds, 1.0f);
}

void MeterLookAndFeel::drawMeterBar(juce::Graphics& g, const juce::Rectangle<float>& bounds, float proportion)
{
    // Narrow bar centered
    const float barWidth = juce::jlimit(8.0f, 40.0f, bounds.getWidth() * 0.18f);
    juce::Rectangle<float> bar(bounds.getCentreX() - barWidth * 0.5f, 
                              bounds.getY(), 
                              barWidth, 
                              bounds.getHeight());
    
    // Filled height
    const float filledHeight = bar.getHeight() * proportion;
    juce::Rectangle<float> fill = bar.removeFromBottom(filledHeight);
    
    // Gradient: bottom green -> middle orange -> top red
    juce::ColourGradient gradient(juce::Colours::green, fill.getCentreX(), fill.getBottom(),
                                 juce::Colours::red, fill.getCentreX(), fill.getY(), false);
    gradient.addColour(0.5, juce::Colours::orange);
    
    g.setGradientFill(gradient);
    g.fillRect(fill);
}