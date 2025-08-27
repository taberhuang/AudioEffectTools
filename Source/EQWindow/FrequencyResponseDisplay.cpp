// FrequencyResponseDisplay.cpp
#include "FrequencyResponseDisplay.h"
#include "EQWindow.h"
#include "../EffectJUCE/IIRFilterProcessor.h"
#include <cmath>
#include <complex>

FrequencyResponseDisplay::FrequencyResponseDisplay(IIRFilterProcessor* proc)
    : processor(proc)
{
    // Completely disable accessibility
    setAccessible(false);
    setWantsKeyboardFocus(false);
    
    // Disable accessibility for child components
    setInterceptsMouseClicks(true, false);  // Do not intercept mouse events of child components
    
    bandEnabled.resize(7, true);
    selectedBand = -1;
}

FrequencyResponseDisplay::~FrequencyResponseDisplay()
{
    // Clean up all resources
    processor = nullptr;
    responseCurve.clear();
    bandPoints.clear();
    bandEnabled.clear();
}

void FrequencyResponseDisplay::parentHierarchyChanged()
{
    // Update response after component is added to parent
    if (getParentComponent() != nullptr)
    {
        updateResponse();
    }
}

void FrequencyResponseDisplay::paint(juce::Graphics& g)
{
    // Check if component is valid
    if (!isVisible() || getWidth() <= 0 || getHeight() <= 0)
        return;
        
    auto bounds = getLocalBounds();
    
    // Reserve margins for labels
    auto drawArea = bounds.reduced(10, 10).withTrimmedRight(40).withTrimmedBottom(20);
    
    // Check if drawing area is valid
    if (drawArea.getWidth() <= 0 || drawArea.getHeight() <= 0)
        return;
    
    // Background
    g.fillAll(juce::Colour(0xff0a0a0a));
    
    // Grid
    g.setColour(juce::Colour(0xff2a2a2a));
    
    // Frequency grid lines (logarithmic)
    const float frequencies[] = {20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000};
    for (float freq : frequencies)
    {
        float x = drawArea.getX() + frequencyToX(freq, static_cast<float>(drawArea.getWidth()));
        if (x >= drawArea.getX() && x <= drawArea.getRight())
        {
            g.drawVerticalLine(static_cast<int>(x), drawArea.getY(), drawArea.getBottom());
        }
    }
    
    // Additional fine grid lines
    const float fineFreqs[] = {30, 40, 60, 70, 80, 90, 150, 300, 400, 600, 700, 800, 900, 
                               1500, 3000, 4000, 6000, 7000, 8000, 9000, 15000};
    g.setColour(juce::Colour(0xff1a1a1a));
    for (float freq : fineFreqs)
    {
        float x = drawArea.getX() + frequencyToX(freq, static_cast<float>(drawArea.getWidth()));
        if (x >= drawArea.getX() && x <= drawArea.getRight())
        {
            g.drawVerticalLine(static_cast<int>(x), drawArea.getY(), drawArea.getBottom());
        }
    }
    
    // Frequency labels at bottom
    g.setColour(juce::Colour(0xff6a6a6a));
    g.setFont(10.0f);
    for (float freq : frequencies)
    {
        float x = drawArea.getX() + frequencyToX(freq, static_cast<float>(drawArea.getWidth()));
        if (x >= drawArea.getX() - 20 && x <= drawArea.getRight() + 20)
        {
            juce::String label = freq < 1000 ? juce::String(static_cast<int>(freq)) : 
                                juce::String(static_cast<int>(freq/1000)) + "k";
            g.drawText(label, static_cast<int>(x - 20), drawArea.getBottom() + 5, 40, 20, 
                       juce::Justification::centred);
        }
    }
    
    // Gain grid lines
    g.setColour(juce::Colour(0xff2a2a2a));
    for (int db = -12; db <= 12; db += 3)
    {
        float y = drawArea.getY() + gainToY(static_cast<float>(db), static_cast<float>(drawArea.getHeight()));
        if (y >= drawArea.getY() && y <= drawArea.getBottom())
        {
            g.drawHorizontalLine(static_cast<int>(y), drawArea.getX(), drawArea.getRight());
        }
    }
    
    // Gain labels on right side
    g.setColour(juce::Colour(0xff6a6a6a));
    g.setFont(10.0f);
    for (int db = -12; db <= 12; db += 3)
    {
        float y = drawArea.getY() + gainToY(static_cast<float>(db), static_cast<float>(drawArea.getHeight()));
        if (y >= drawArea.getY() - 10 && y <= drawArea.getBottom() + 10)
        {
            g.drawText(juce::String(db) + "dB", drawArea.getRight() + 5, 
                       static_cast<int>(y - 10), 40, 20, juce::Justification::left);
        }
    }
    
    // Draw 0dB reference line
    g.setColour(juce::Colour(0xff666666));
    float zeroY = drawArea.getY() + gainToY(0, static_cast<float>(drawArea.getHeight()));
    if (zeroY >= drawArea.getY() && zeroY <= drawArea.getBottom())
    {
        g.drawHorizontalLine(static_cast<int>(zeroY), drawArea.getX(), drawArea.getRight());
    }
    
    // Draw response curve
    if (!responseCurve.isEmpty())
    {
        g.setColour(juce::Colour(0xff00ff00));
        
        // Use scoped save state to handle clip region
        {
            juce::Graphics::ScopedSaveState saveState(g);
            g.reduceClipRegion(drawArea);
            
            juce::Path translatedPath = responseCurve;
            translatedPath.applyTransform(juce::AffineTransform::translation(static_cast<float>(drawArea.getX()), 
                                                                             static_cast<float>(drawArea.getY())));
            g.strokePath(translatedPath, juce::PathStrokeType(3.0f));
            
            // saveState destructor automatically restores clip region and other graphics states
        }
    }
    
    // Draw band points
    for (size_t i = 0; i < bandPoints.size(); ++i)
    {
        if (i < bandEnabled.size() && bandEnabled[i] && i < 7)  // Ensure no more than 7 bands
        {
            float x = drawArea.getX() + frequencyToX(bandPoints[i].x, static_cast<float>(drawArea.getWidth()));
            float y = drawArea.getY() + gainToY(bandPoints[i].y, static_cast<float>(drawArea.getHeight()));
            
            // Ensure point is within drawing area
            if (x >= drawArea.getX() - 15 && x <= drawArea.getRight() + 15 &&
                y >= drawArea.getY() - 15 && y <= drawArea.getBottom() + 15)
            {
                // Draw circle with band number
                g.setColour(juce::Colour(0xff6bb6ff));
                g.fillEllipse(x - 15, y - 15, 30, 30);
                
                g.setColour(juce::Colours::white);
                g.setFont(14.0f);
                g.drawText(juce::String(static_cast<int>(i + 1)), 
                           static_cast<int>(x - 15), static_cast<int>(y - 15), 
                           30, 30, juce::Justification::centred);
            }
        }
    }
}

void FrequencyResponseDisplay::mouseDown(const juce::MouseEvent& event)
{
    if (!isEnabled())
        return;
        
    float minDist = 30.0f;
    selectedBand = -1;
    
    auto bounds = getLocalBounds();
    auto drawArea = bounds.reduced(10, 10).withTrimmedRight(40).withTrimmedBottom(20);
    
    if (drawArea.getWidth() <= 0 || drawArea.getHeight() <= 0)
        return;
    
    for (size_t i = 0; i < bandPoints.size() && i < 7; ++i)
    {
        if (i < bandEnabled.size() && bandEnabled[i])
        {
            float x = drawArea.getX() + frequencyToX(bandPoints[i].x, static_cast<float>(drawArea.getWidth()));
            float y = drawArea.getY() + gainToY(bandPoints[i].y, static_cast<float>(drawArea.getHeight()));
            float dist = event.position.getDistanceFrom(juce::Point<float>(x, y));
            
            if (dist < minDist)
            {
                minDist = dist;
                selectedBand = static_cast<int>(i);
            }
        }
    }
}

void FrequencyResponseDisplay::mouseDrag(const juce::MouseEvent& event)
{
    if (!isEnabled() || selectedBand < 0 || selectedBand >= 7)
        return;
        
    if (selectedBand >= static_cast<int>(bandEnabled.size()) || !bandEnabled[selectedBand])
        return;
        
    auto bounds = getLocalBounds();
    auto drawArea = bounds.reduced(10, 10).withTrimmedRight(40).withTrimmedBottom(20);
    
    if (drawArea.getWidth() <= 0 || drawArea.getHeight() <= 0)
        return;
        
    // Calculate coordinates relative to the drawing area
    float relativeX = static_cast<float>(event.x - drawArea.getX());
    float relativeY = static_cast<float>(event.y - drawArea.getY());
    
    float freq = xToFrequency(relativeX, static_cast<float>(drawArea.getWidth()));
    float gain = yToGain(relativeY, static_cast<float>(drawArea.getHeight()));
    
    freq = juce::jlimit(20.0f, 20000.0f, freq);
    gain = juce::jlimit(-12.0f, 12.0f, gain);
    
    if (selectedBand < static_cast<int>(bandPoints.size()))
    {
        bandPoints[selectedBand] = {freq, gain};
    }
    
    // Update corresponding controls in EQWindow - using weak reference to avoid circular reference
    auto* parentComp = findParentComponentOfClass<EQWindow>();
    if (parentComp && parentComp->content)
    {
        if (selectedBand < parentComp->content->eqBands.size())
        {
            auto* band = parentComp->content->eqBands[selectedBand];
            if (band)
            {
                band->freqSlider.setValue(freq, juce::sendNotification);
                band->gainSlider.setValue(gain, juce::sendNotification);
            }
        }
    }
    
    updateResponse();
    repaint();
}

void FrequencyResponseDisplay::mouseUp(const juce::MouseEvent& /*event*/)
{
    selectedBand = -1;
}

void FrequencyResponseDisplay::updateResponse()
{
    responseCurve.clear();
    bandPoints.clear();

    auto bounds = getLocalBounds();
    auto drawArea = bounds.reduced(10, 10).withTrimmedRight(40).withTrimmedBottom(20);
    if (drawArea.getWidth() <= 0 || drawArea.getHeight() <= 0)
        return;

    const int numPoints = drawArea.getWidth();
    if (numPoints <= 0)
        return;

    auto* parentWindow = findParentComponentOfClass<EQWindow>();
    if (!parentWindow || !parentWindow->content)
        return;

    const int maxBands = juce::jmin(7, parentWindow->content->eqBands.size());

    // Record band information
    struct BandData {
        bool enabled;
        float b0,b1,b2,a0,a1,a2;
    };
    std::vector<BandData> bands(maxBands);

    for (int i = 0; i < maxBands; ++i)
    {
        auto* band = parentWindow->content->eqBands[i];
        if (!band) continue;

        float freq = static_cast<float>(band->freqSlider.getValue());
        float gain = static_cast<float>(band->gainSlider.getValue());
        bool enabled = band->enableButton.getToggleState();
        float Q = static_cast<float>(band->qSlider.getValue());
        juce::String filterType = band->typeCombo.getText();

        if (i < (int)bandEnabled.size())
            bandEnabled[i] = enabled;

        bandPoints.push_back({freq, enabled ? gain : 0.0f});

        // Limit range
        freq = juce::jlimit(20.0f, 20000.0f, freq);
        gain = juce::jlimit(-12.0f, 12.0f, gain);
        Q = juce::jlimit(0.25f, 10.0f, Q);

        bands[i].enabled = enabled;
        if (enabled)
            calcEqCoef(filterType, freq, gain, Q, &bands[i].b0);
        else
            bands[i] = { false, 0,0,0,1,0,0 };
    }

    const float sampleRate = 48000.0f;

    for (int i = 0; i < numPoints; ++i)
    {
        float x = static_cast<float>(i);
        float freq = xToFrequency(x, (float)drawArea.getWidth());
        freq = juce::jlimit(20.0f, 20000.0f, freq);

        float totalMag = 1.0f;

        for (int b = 0; b < maxBands; ++b)
        {
            if (!bands[b].enabled) continue;

            float w = 2.0f * juce::MathConstants<float>::pi * freq / sampleRate;
            std::complex<float> ejw = std::exp(std::complex<float>(0, -w));
            std::complex<float> ej2w = ejw * ejw;

            auto num = bands[b].b0 + bands[b].b1 * ejw + bands[b].b2 * ej2w;
            auto den = bands[b].a0 + bands[b].a1 * ejw + bands[b].a2 * ej2w;

            if (std::abs(den) > 1e-6f)
                totalMag *= std::abs(num / den);
        }

        float totalGain = 20.0f * std::log10(std::max(0.0001f, totalMag));
        totalGain = juce::jlimit(-24.0f, 24.0f, totalGain);
        float y = gainToY(totalGain, (float)drawArea.getHeight());

        if (i == 0) responseCurve.startNewSubPath(x, y);
        else responseCurve.lineTo(x, y);
    }
}
#if 0
float FrequencyResponseDisplay::calculateBandResponse(float freq, float bandFreq, float bandGain, 
                                                      float bandQ, const juce::String& filterType,
                                                      float sampleRate)
{
    // Use the exact same calculation logic as IIRFilterProcessor
    if (bandFreq <= 20.0f || bandFreq >= sampleRate / 2.0f || bandQ <= 0.0f)
        return 0.0f;
    
    float omega = 2.0f * juce::MathConstants<float>::pi * bandFreq / sampleRate;
    float w = 2.0f * juce::MathConstants<float>::pi * freq / sampleRate;
    float sn = std::sin(omega);
    float cs = std::cos(omega);
    float alpha = sn / (2.0f * bandQ);
    
    float a0, b0, b1, b2, a1, a2;
    
    if (filterType == "PEQ")
    {
        if (std::abs(bandGain) <= 0.01f)
        {
            // Bypass filter when gain is near zero
            return 0.0f;
        }
        
        // Use the same A calculation as IIRFilterProcessor: gain/40 instead of gain/20
        float A = std::pow(10.0f, bandGain / 40.0f);
        
        // Standard Peaking EQ (Bell) formula - exactly consistent with IIRFilterProcessor
        b0 = 1.0f + alpha * A;
        b1 = -2.0f * cs;
        b2 = 1.0f - alpha * A;
        a0 = 1.0f + alpha / A;
        a1 = -2.0f * cs;
        a2 = 1.0f - alpha / A;
    }
    else if (filterType == "LSH") // Low Shelf
    {
        // Low shelf uses /20 correctly - consistent with IIRFilterProcessor
        float A_shelf = std::pow(10.0f, bandGain / 20.0f);
        
        a0 = (A_shelf + 1.0f) + (A_shelf - 1.0f) * cs + 2.0f * std::sqrt(A_shelf) * alpha;
        b0 = A_shelf * ((A_shelf + 1.0f) - (A_shelf - 1.0f) * cs + 2.0f * std::sqrt(A_shelf) * alpha);
        b1 = 2.0f * A_shelf * ((A_shelf - 1.0f) - (A_shelf + 1.0f) * cs);
        b2 = A_shelf * ((A_shelf + 1.0f) - (A_shelf - 1.0f) * cs - 2.0f * std::sqrt(A_shelf) * alpha);
        a1 = -2.0f * ((A_shelf - 1.0f) + (A_shelf + 1.0f) * cs);
        a2 = (A_shelf + 1.0f) + (A_shelf - 1.0f) * cs - 2.0f * std::sqrt(A_shelf) * alpha;
    }
    else if (filterType == "HSH") // High Shelf
    {
        // High shelf uses /20 correctly - consistent with IIRFilterProcessor
        float A_shelf = std::pow(10.0f, bandGain / 20.0f);
        
        a0 = (A_shelf + 1.0f) - (A_shelf - 1.0f) * cs + 2.0f * std::sqrt(A_shelf) * alpha;
        b0 = A_shelf * ((A_shelf + 1.0f) + (A_shelf - 1.0f) * cs + 2.0f * std::sqrt(A_shelf) * alpha);
        b1 = -2.0f * A_shelf * ((A_shelf - 1.0f) + (A_shelf + 1.0f) * cs);
        b2 = A_shelf * ((A_shelf + 1.0f) + (A_shelf - 1.0f) * cs - 2.0f * std::sqrt(A_shelf) * alpha);
        a1 = 2.0f * ((A_shelf - 1.0f) - (A_shelf + 1.0f) * cs);
        a2 = (A_shelf + 1.0f) - (A_shelf - 1.0f) * cs - 2.0f * std::sqrt(A_shelf) * alpha;
    }
    else if (filterType == "LPF") // Low Pass
    {
        a0 = 1.0f + alpha;
        b0 = (1.0f - cs) / 2.0f;
        b1 = 1.0f - cs;
        b2 = (1.0f - cs) / 2.0f;
        a1 = -2.0f * cs;
        a2 = 1.0f - alpha;
    }
    else if (filterType == "HPF") // High Pass
    {
        a0 = 1.0f + alpha;
        b0 = (1.0f + cs) / 2.0f;
        b1 = -(1.0f + cs);
        b2 = (1.0f + cs) / 2.0f;
        a1 = -2.0f * cs;
        a2 = 1.0f - alpha;
    }
    else
    {
        // Default to bypass
        return 0.0f;
    }

    if (std::abs(a0) < 1e-6f)
    {
        // Prevent division by zero
        return 0.0f;
    }

    // Normalize coefficients - consistent with IIRFilterProcessor
    b0 /= a0; b1 /= a0; b2 /= a0; a1 /= a0; a2 /= a0;

    // Calculate frequency response H(e^jw) = (b0 + b1*e^(-jw) + b2*e^(-j2w)) / (1 + a1*e^(-jw) + a2*e^(-j2w))
    std::complex<float> ejw = std::exp(std::complex<float>(0, w));
    std::complex<float> ej2w = ejw * ejw;
    
    std::complex<float> numerator = b0 + b1 / ejw + b2 / ej2w;
    std::complex<float> denominator = 1.0f + a1 / ejw + a2 / ej2w;
    
    // Prevent division by zero
    if (std::abs(denominator) > 0.0001f)
    {
        std::complex<float> H = numerator / denominator;
        float magnitude = std::abs(H);
        if (magnitude > 0.0001f)
        {
            float bandResponse = 20.0f * std::log10(magnitude);
            return juce::jlimit(-24.0f, 24.0f, bandResponse);
        }
    }
    
    return 0.0f;
}
#endif
void FrequencyResponseDisplay::setBandEnabled(int band, bool enabled)
{
    if (band >= 0 && band < static_cast<int>(bandEnabled.size()))
    {
        bandEnabled[band] = enabled;
        updateResponse();
        repaint();
    }
}

float FrequencyResponseDisplay::frequencyToX(float freq, float width)
{
    if (width <= 0 || freq <= 0)
        return 0;
        
    float minFreq = 20.0f;
    float maxFreq = 20000.0f;
    freq = juce::jlimit(minFreq, maxFreq, freq);
    
    float proportion = (std::log10(freq) - std::log10(minFreq)) / 
                      (std::log10(maxFreq) - std::log10(minFreq));
    return proportion * width;
}

float FrequencyResponseDisplay::gainToY(float gain, float height)
{
    if (height <= 0)
        return 0;
        
    gain = juce::jlimit(-12.0f, 12.0f, gain);
    float proportion = 1.0f - ((gain + 12.0f) / 24.0f);
    return proportion * height;
}

float FrequencyResponseDisplay::xToFrequency(float x, float width)
{
    if (width <= 0)
        return 20.0f;
        
    float minFreq = 20.0f;
    float maxFreq = 20000.0f;
    float proportion = juce::jlimit(0.0f, 1.0f, x / width);
    
    return std::pow(10.0f, proportion * (std::log10(maxFreq) - std::log10(minFreq)) + 
                    std::log10(minFreq));
}

float FrequencyResponseDisplay::yToGain(float y, float height)
{
    if (height <= 0)
        return 0;
        
    float proportion = juce::jlimit(0.0f, 1.0f, 1.0f - (y / height));
    return (proportion * 24.0f) - 12.0f;
}

void FrequencyResponseDisplay::calcEqCoef(const juce::String& type, float freq, float gain, float Q, float* pCoef)
{
    if (type == "PEQ") EqPeakFilter(freq, gain, Q, pCoef);
    else if (type == "LSH") EqLowShelf(freq, gain, Q, pCoef);
    else if (type == "HSH") EqHighShelf(freq, gain, Q, pCoef);
    else if (type == "LPF") EqLowPass(freq, Q, pCoef);
    else if (type == "HPF") EqHighPass(freq, Q, pCoef);
    else EqIIRCoef(1,0,0,1,0,0,pCoef); // bypass
}

void FrequencyResponseDisplay::EqPeakFilter(float freq, float gain, float Q, float* pCoef)
{
    float A = pow(10.0f, gain / 40.0f);
    float omega = juce::MathConstants<float>::twoPi * freq / 48000.0f;
    float alpha = 0.5f * sin(omega) / Q;
    float c2 = -2.0f * cos(omega);
    float alphaTimesA = alpha * A;
    float alphaOverA = alpha / A;
    EqIIRCoef((1.0f + alphaTimesA), c2, (1.0f - alphaTimesA),
              (1.0f + alphaOverA), c2, (1.0f - alphaOverA), pCoef);
}

void FrequencyResponseDisplay::EqLowShelf(float freq, float gain, float Q, float* pCoef)
{
    float A = pow(10.0f, gain / 40.0f);
    float aminus1 = A - 1.0f;
    float aplus1 = A + 1.0f;
    float omega = juce::MathConstants<float>::twoPi * freq / 48000.0f;
    float coso = cos(omega);
    float beta = sin(omega) * sqrt(A) / Q;
    float aminus1TimesCoso = aminus1 * coso;
    EqIIRCoef((A * (aplus1 - aminus1TimesCoso + beta)),
              (A * 2.0f * (aminus1 - aplus1 * coso)),
              (A * (aplus1 - aminus1TimesCoso - beta)),
              (aplus1 + aminus1TimesCoso + beta),
              (-2.0f * (aminus1 + aplus1 * coso)),
              (aplus1 + aminus1TimesCoso - beta), pCoef);
}

void FrequencyResponseDisplay::EqHighShelf(float freq, float gain, float Q, float* pCoef)
{
    float A = pow(10.0f, gain / 40.0f);
    float aminus1 = A - 1.0f;
    float aplus1 = A + 1.0f;
    float omega = juce::MathConstants<float>::twoPi * freq / 48000.0f;
    float coso = cos(omega);
    float beta = sin(omega) * sqrt(A) / Q;
    float aminus1TimesCoso = aminus1 * coso;
    EqIIRCoef((A * (aplus1 + aminus1TimesCoso + beta)),
              (A * -2.0f * (aminus1 + aplus1 * coso)),
              (A * (aplus1 + aminus1TimesCoso - beta)),
              (aplus1 - aminus1TimesCoso + beta),
              (2.0f * (aminus1 - aplus1 * coso)),
              (aplus1 - aminus1TimesCoso - beta), pCoef);
}

void FrequencyResponseDisplay::EqLowPass(float freq, float Q, float* pCoef)
{
    float n = 1.0f / tan(juce::MathConstants<float>::pi * freq / 48000.0f);
    float n2 = n * n;
    float c1 = 1.0f / (1.0f + n / Q + n2);
    EqIIRCoef(c1, (c1 * 2.0f), c1, 1.0f,
              (c1 * 2.0f * (1.0f - n2)),
              (c1 * (1.0f - 1.0f / Q * n + n2)), pCoef);
}

void FrequencyResponseDisplay::EqHighPass(float freq, float Q, float* pCoef)
{
    float n = tan(juce::MathConstants<float>::pi * freq / 48000.0f);
    float n2 = n * n;
    float c1 = 1.0f / (1.0f + n / Q + n2);
    EqIIRCoef(c1, (c1 * -2.0f), c1, 1.0f,
              (c1 * 2.0f * (n2 - 1.0f)),
              (c1 * (1.0f - 1.0f / Q * n + n2)), pCoef);
}

void FrequencyResponseDisplay::EqIIRCoef(float c1,float c2,float c3,float c4,float c5,float c6,float* pCoef)
{
    pCoef[0] = c1; pCoef[1] = c2; pCoef[2] = c3;
    pCoef[3] = c4; pCoef[4] = c5; pCoef[5] = c6;
}