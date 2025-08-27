// EQControls.cpp
#include "EQControls.h"

// CustomSliderLookAndFeel Implementation
CustomSliderLookAndFeel::CustomSliderLookAndFeel()
{
    // Set colors
    setColour(juce::Slider::backgroundColourId, juce::Colour(0xff3a3a3a));
    setColour(juce::Slider::trackColourId, juce::Colour(0xff6bb6ff));
    setColour(juce::Slider::thumbColourId, juce::Colour(0xff00ff00));
    setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff2a2a2a));
    setColour(juce::TextEditor::textColourId, juce::Colour(0xffff8c00)); // Orange text
    setColour(juce::TextEditor::highlightColourId, juce::Colour(0xff6bb6ff));
    setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff4a4a4a));
}

void CustomSliderLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                               float sliderPos, float minSliderPos, float maxSliderPos,
                                               const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (slider.isBar())
    {
        g.setColour(slider.findColour(juce::Slider::trackColourId));
        g.fillRect(slider.isHorizontal() ? juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y) + static_cast<float>(height) * 0.5f - 1.0f,
                                                                 sliderPos - static_cast<float>(x), 2.0f)
                                        : juce::Rectangle<float>(static_cast<float>(x) + static_cast<float>(width) * 0.5f - 1.0f, sliderPos,
                                                                 2.0f, static_cast<float>(y) + static_cast<float>(height) - sliderPos));
    }
    else
    {
        auto isTwoVal = (style == juce::Slider::SliderStyle::TwoValueVertical || style == juce::Slider::SliderStyle::TwoValueHorizontal);
        auto isThreeVal = (style == juce::Slider::SliderStyle::ThreeValueVertical || style == juce::Slider::SliderStyle::ThreeValueHorizontal);

        auto trackWidth = juce::jmin(6.0f, slider.isHorizontal() ? static_cast<float>(height) * 0.25f : static_cast<float>(width) * 0.25f);

        juce::Point<float> startPoint(slider.isHorizontal() ? static_cast<float>(x) : static_cast<float>(x) + static_cast<float>(width) * 0.5f,
                                     slider.isHorizontal() ? static_cast<float>(y) + static_cast<float>(height) * 0.5f : static_cast<float>(height) + static_cast<float>(y));

        juce::Point<float> endPoint(slider.isHorizontal() ? static_cast<float>(width) + static_cast<float>(x) : startPoint.x,
                                   slider.isHorizontal() ? startPoint.y : static_cast<float>(y));

        juce::Path backgroundTrack;
        backgroundTrack.startNewSubPath(startPoint);
        backgroundTrack.lineTo(endPoint);
        g.setColour(juce::Colour(0xff3a3a3a));
        g.strokePath(backgroundTrack, juce::PathStrokeType(trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::Path valueTrack;
        juce::Point<float> minPoint, maxPoint, thumbPoint;

        if (isTwoVal || isThreeVal)
        {
            minPoint = { slider.isHorizontal() ? minSliderPos : static_cast<float>(width) * 0.5f + static_cast<float>(x),
                        slider.isHorizontal() ? static_cast<float>(height) * 0.5f + static_cast<float>(y) : minSliderPos };

            maxPoint = { slider.isHorizontal() ? maxSliderPos : static_cast<float>(width) * 0.5f + static_cast<float>(x),
                        slider.isHorizontal() ? static_cast<float>(height) * 0.5f + static_cast<float>(y) : maxSliderPos };
        }
        else
        {
            auto kx = slider.isHorizontal() ? sliderPos : (static_cast<float>(x) + static_cast<float>(width) * 0.5f);
            auto ky = slider.isHorizontal() ? (static_cast<float>(y) + static_cast<float>(height) * 0.5f) : sliderPos;

            minPoint = startPoint;
            maxPoint = { kx, ky };
        }

        auto thumbWidth = getSliderThumbRadius(slider);

        valueTrack.startNewSubPath(minPoint);
        valueTrack.lineTo(isThreeVal ? thumbPoint : maxPoint);
        g.setColour(juce::Colour(0xff6bb6ff));
        g.strokePath(valueTrack, juce::PathStrokeType(trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        if (!isTwoVal)
        {
            g.setColour(juce::Colour(0xff6bb6ff));
            g.fillEllipse(juce::Rectangle<float>(static_cast<float>(thumbWidth), static_cast<float>(thumbWidth)).withCentre(isThreeVal ? thumbPoint : maxPoint));
        }

        if (isTwoVal || isThreeVal)
        {
            auto sr = juce::jmin(trackWidth, (slider.isHorizontal() ? static_cast<float>(height) : static_cast<float>(width)) * 0.4f);

            g.setColour(juce::Colour(0xff6bb6ff));
            g.fillEllipse(juce::Rectangle<float>(static_cast<float>(sr), static_cast<float>(sr)).withCentre(minPoint));
            g.fillEllipse(juce::Rectangle<float>(static_cast<float>(sr), static_cast<float>(sr)).withCentre(maxPoint));
        }
    }
}

void CustomSliderLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                               const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider)
{
    auto radius = (float) juce::jmin(width / 2, height / 2) - 4.0f;
    auto centreX = (float) x + (float) width * 0.5f;
    auto centreY = (float) y + (float) height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    g.setColour(juce::Colour(0xff3a3a3a));
    g.fillEllipse(rx, ry, rw, rw);

    g.setColour(juce::Colour(0xff6bb6ff));
    g.drawEllipse(rx, ry, rw, rw, 2.0f);

    juce::Path p;
    auto pointerLength = radius * 0.33f;
    auto pointerThickness = 3.0f;
    p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
    p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

    g.setColour(juce::Colour(0xff6bb6ff));
    g.fillPath(p);

    g.setColour(juce::Colour(0xff6bb6ff));
    g.fillEllipse(centreX - 3.0f, centreY - 3.0f, 6.0f, 6.0f);
}

void CustomSliderLookAndFeel::drawTextEditorOutline(juce::Graphics& g, int width, int height, juce::TextEditor& textEditor)
{
    if (textEditor.isEnabled())
    {
        if (textEditor.hasKeyboardFocus(true) && !textEditor.isReadOnly())
        {
            g.setColour(juce::Colour(0xffff8c00));
            g.drawRect(0, 0, width, height, 2);
        }
        else
        {
            g.setColour(juce::Colour(0xffff8c00));
            g.drawRect(0, 0, width, height, 1);
        }
    }
    else
    {
        g.setColour(juce::Colour(0xff666666));
        g.drawRect(0, 0, width, height, 1);
    }
}

int CustomSliderLookAndFeel::getSliderThumbRadius(juce::Slider& slider)
{
    return juce::jmin(12, slider.isHorizontal() ? static_cast<int>(slider.getHeight() * 0.5f)
                                                : static_cast<int>(slider.getWidth() * 0.5f));
}

// EQBand Implementation
EQBand::EQBand()
    : typeLabel("", "Type:"),
      freqLabel("", "Freq(Hz):"),
      gainLabel("", "Gain(dB):"),
      qLabel("", "Q:"),
      freqSlider(juce::Slider::LinearHorizontal, juce::Slider::NoTextBox),
      gainSlider(juce::Slider::LinearHorizontal, juce::Slider::NoTextBox),
      qSlider(juce::Slider::LinearHorizontal, juce::Slider::NoTextBox)
{
    // Disable accessibility for all components
    enableButton.setAccessible(false);
    typeLabel.setAccessible(false);
    freqLabel.setAccessible(false);
    gainLabel.setAccessible(false);
    qLabel.setAccessible(false);
    typeCombo.setAccessible(false);
    freqSlider.setAccessible(false);
    gainSlider.setAccessible(false);
    qSlider.setAccessible(false);
    freqText.setAccessible(false);
    gainText.setAccessible(false);
    qText.setAccessible(false);
    
    enableButton.setToggleState(true, juce::dontSendNotification);
    
    typeLabel.setText("Type", juce::dontSendNotification);
    freqLabel.setText("Freq (Hz)", juce::dontSendNotification);
    gainLabel.setText("Gain (dB)", juce::dontSendNotification);
    qLabel.setText("Q", juce::dontSendNotification);
    
    typeLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff6b35));
    freqLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff6b35));
    gainLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff6b35));
    qLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff6b35));
    
    freqSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    gainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    qSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    
    freqSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    gainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    qSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    
    freqText.setJustification(juce::Justification::centred);
    gainText.setJustification(juce::Justification::centred);
    qText.setJustification(juce::Justification::centred);
    
    freqText.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff2a2a2a));
    gainText.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff2a2a2a));
    qText.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff2a2a2a));
    
    freqText.setColour(juce::TextEditor::textColourId, juce::Colour(0xffff6b35));
    gainText.setColour(juce::TextEditor::textColourId, juce::Colour(0xffff6b35));
    qText.setColour(juce::TextEditor::textColourId, juce::Colour(0xffff6b35));
    
    freqText.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff4a4a4a));
    gainText.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff4a4a4a));
    qText.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff4a4a4a));
}

EQBand::~EQBand()
{
    // Clean up LookAndFeel
    freqSlider.setLookAndFeel(nullptr);
    gainSlider.setLookAndFeel(nullptr);
    qSlider.setLookAndFeel(nullptr);
    freqText.setLookAndFeel(nullptr);
    gainText.setLookAndFeel(nullptr);
    qText.setLookAndFeel(nullptr);
}

void EQBand::setupCallbacks()
{
    freqText.onTextChange = [this]()
    {
        float value = freqText.getText().getFloatValue();
        if (value >= 20.0f && value <= 20000.0f)
        {
            freqSlider.setValue(static_cast<double>(value));
        }
    };
    
    gainText.onTextChange = [this]()
    {
        float value = gainText.getText().getFloatValue();
        if (value >= -12.0f && value <= 12.0f)
        {
            gainSlider.setValue(static_cast<double>(value));
        }
    };
    
    qText.onTextChange = [this]()
    {
        float value = qText.getText().getFloatValue();
        if (value >= 0.25f && value <= 10.0f)
        {
            qSlider.setValue(static_cast<double>(value));
        }
    };
}

// LimiterSection Implementation
LimiterSection::LimiterSection(const juce::String& title)
    : titleLabel("", title),
      enableButton("Enable"),
      thLabel("", "Threshold:"),
      ratioLabel("", "Ratio:"),
      atLabel("", "Attack:"),
      rtLabel("", "Release:"),
      thSlider(juce::Slider::LinearHorizontal, juce::Slider::NoTextBox),
      ratioSlider(juce::Slider::LinearHorizontal, juce::Slider::NoTextBox),
      atSlider(juce::Slider::LinearHorizontal, juce::Slider::NoTextBox),
      rtSlider(juce::Slider::LinearHorizontal, juce::Slider::NoTextBox)
{
    // Disable accessibility for all components
    titleLabel.setAccessible(false);
    enableButton.setAccessible(false);
    thLabel.setAccessible(false);
    ratioLabel.setAccessible(false);
    atLabel.setAccessible(false);
    rtLabel.setAccessible(false);
    thSlider.setAccessible(false);
    ratioSlider.setAccessible(false);
    atSlider.setAccessible(false);
    rtSlider.setAccessible(false);
    thText.setAccessible(false);
    ratioText.setAccessible(false);
    atText.setAccessible(false);
    rtText.setAccessible(false);
    
    titleLabel.setText(title, juce::dontSendNotification);
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff6b35));
    titleLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    
    enableButton.setButtonText("Enable");
    enableButton.setToggleState(false, juce::dontSendNotification);
    
    thLabel.setText("Th(dBFS)", juce::dontSendNotification);
    ratioLabel.setText("Ratio", juce::dontSendNotification);
    atLabel.setText("AT(ms)", juce::dontSendNotification);
    rtLabel.setText("RT(ms)", juce::dontSendNotification);
    
    thLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff6b35));
    ratioLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff6b35));
    atLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff6b35));
    rtLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff6b35));
    
    thSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    ratioSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    atSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    rtSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    
    thSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    ratioSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    atSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    rtSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    
    thSlider.setRange(-60.0, 0.0, 0.1);
    thSlider.setValue(0.0, juce::dontSendNotification);
    
    ratioSlider.setRange(1.0, 20.0, 0.1);
    ratioSlider.setValue(10.0, juce::dontSendNotification);
    
    atSlider.setRange(1.0, 100.0, 1.0);
    atSlider.setValue(50.0, juce::dontSendNotification);
    
    rtSlider.setRange(10.0, 1000.0, 1.0);
    rtSlider.setValue(200.0, juce::dontSendNotification);
    
    thText.setJustification(juce::Justification::centred);
    ratioText.setJustification(juce::Justification::centred);
    atText.setJustification(juce::Justification::centred);
    rtText.setJustification(juce::Justification::centred);
    
    thText.setText("0.0", juce::dontSendNotification);
    ratioText.setText("10.0", juce::dontSendNotification);
    atText.setText("50", juce::dontSendNotification);
    rtText.setText("200", juce::dontSendNotification);
    
    thText.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff2a2a2a));
    ratioText.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff2a2a2a));
    atText.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff2a2a2a));
    rtText.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff2a2a2a));
    
    thText.setColour(juce::TextEditor::textColourId, juce::Colour(0xffff6b35));
    ratioText.setColour(juce::TextEditor::textColourId, juce::Colour(0xffff6b35));
    atText.setColour(juce::TextEditor::textColourId, juce::Colour(0xffff6b35));
    rtText.setColour(juce::TextEditor::textColourId, juce::Colour(0xffff6b35));
}

LimiterSection::~LimiterSection()
{
    // Clean up LookAndFeel
    thSlider.setLookAndFeel(nullptr);
    ratioSlider.setLookAndFeel(nullptr);
    atSlider.setLookAndFeel(nullptr);
    rtSlider.setLookAndFeel(nullptr);
    thText.setLookAndFeel(nullptr);
    ratioText.setLookAndFeel(nullptr);
    atText.setLookAndFeel(nullptr);
    rtText.setLookAndFeel(nullptr);
}

void LimiterSection::setupCallbacks()
{
    thText.onTextChange = [this]()
    {
        float value = thText.getText().getFloatValue();
        if (value >= -60.0f && value <= 0.0f)
            thSlider.setValue(static_cast<double>(value));
    };
    
    ratioText.onTextChange = [this]()
    {
        float value = ratioText.getText().getFloatValue();
        if (value >= 1.0f && value <= 20.0f)
            ratioSlider.setValue(static_cast<double>(value));
    };
    
    atText.onTextChange = [this]()
    {
        float value = atText.getText().getFloatValue();
        if (value >= 1.0f && value <= 100.0f)
            atSlider.setValue(static_cast<double>(value));
    };
    
    rtText.onTextChange = [this]()
    {
        float value = rtText.getText().getFloatValue();
        if (value >= 10.0f && value <= 1000.0f)
            rtSlider.setValue(static_cast<double>(value));
    };
}

void LimiterSection::setLinearSliders()
{
    thSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    ratioSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    atSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    rtSlider.setSliderStyle(juce::Slider::LinearHorizontal);
}