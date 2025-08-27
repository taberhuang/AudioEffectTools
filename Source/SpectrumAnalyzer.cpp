#include "SpectrumAnalyzer.h"

SpectrumAnalyzer::SpectrumAnalyzer()
    : forwardFFT(fftOrder),
      window(fftSize, juce::dsp::WindowingFunction<float>::hann)
{
    // Initialize data buffers
    juce::zeromem(fifo, sizeof(fifo));
    juce::zeromem(fftData, sizeof(fftData));
    juce::zeromem(scopeData, sizeof(scopeData));
    
    // Start timer, 30fps update rate
    startTimerHz(30);
}

SpectrumAnalyzer::~SpectrumAnalyzer()
{
    stopTimer();
}

void SpectrumAnalyzer::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    auto bounds = getLocalBounds();
    auto spectrumBounds = bounds.reduced(45, 5)
                                   .withTrimmedBottom(30)
                                   .withTrimmedTop(5);

    // Background gradient
    juce::ColourGradient gradient(juce::Colour(20, 20, 30),
                                 spectrumBounds.getX(), spectrumBounds.getY(),
                                 juce::Colour(10, 10, 20),
                                 spectrumBounds.getX(), spectrumBounds.getBottom(),
                                 false);
    g.setGradientFill(gradient);
    g.fillRect(spectrumBounds);

    // Grid
    g.setColour(juce::Colours::darkgrey.withAlpha(0.6f));
    for (int i = 1; i < 10; ++i)
    {
        auto x = spectrumBounds.getX() + (spectrumBounds.getWidth() * i / 10);
        g.drawVerticalLine(x, spectrumBounds.getY(), spectrumBounds.getBottom());
    }
    for (int i = 0; i < 9; ++i)
    {
        auto y = spectrumBounds.getY() + (spectrumBounds.getHeight() * i / 4);
        g.drawHorizontalLine(y, spectrumBounds.getX(), spectrumBounds.getRight());
    }

    // Spectrum fill path
    juce::Path spectrumPath;
    bool pathStarted = false;
    const int pointsToRender = scopeSize - 1;

    for (int i = 0; i < pointsToRender; ++i)
    {
        float scopeValue = juce::jlimit(0.0f, 1.0f, scopeData[i]);
        auto level = juce::jmap(scopeValue, 0.0f, 1.0f,
                               (float)spectrumBounds.getBottom(),
                               (float)spectrumBounds.getY());
        auto x = spectrumBounds.getX() + (spectrumBounds.getWidth() * i / (float)pointsToRender);

        if (!pathStarted)
        {
            spectrumPath.startNewSubPath(x, spectrumBounds.getBottom());
            spectrumPath.lineTo(x, level);
            pathStarted = true;
        }
        else
        {
            spectrumPath.lineTo(x, level);
        }
    }

    if (pathStarted)
    {
        spectrumPath.lineTo(spectrumBounds.getRight(), spectrumBounds.getBottom());
        spectrumPath.closeSubPath();
    }

    juce::ColourGradient spectrumGradient(juce::Colour(0, 180, 255).withAlpha(0.8f),
                                         spectrumBounds.getX(), spectrumBounds.getY(),
                                         juce::Colour(0, 100, 200).withAlpha(0.3f),
                                         spectrumBounds.getX(), spectrumBounds.getBottom(),
                                         false);
    g.setGradientFill(spectrumGradient);
    g.fillPath(spectrumPath);

    // Spectrum line
    juce::Path linePath;
    pathStarted = false;
    for (int i = 0; i < pointsToRender; ++i)
    {
        float scopeValue = juce::jlimit(0.0f, 1.0f, scopeData[i]);
        auto level = juce::jmap(scopeValue, 0.0f, 1.0f,
                               (float)spectrumBounds.getBottom(),
                               (float)spectrumBounds.getY());
        auto x = spectrumBounds.getX() + (spectrumBounds.getWidth() * i / (float)pointsToRender);

        if (!pathStarted)
        {
            linePath.startNewSubPath(x, level);
            pathStarted = true;
        }
        else
        {
            linePath.lineTo(x, level);
        }
    }

    g.setColour(juce::Colours::cyan);
    g.strokePath(linePath, juce::PathStrokeType(2.0f));

    g.setColour(juce::Colours::white.withAlpha(0.5f));
    g.drawRect(spectrumBounds, 1.0f);

    drawFrequencyLabels(g, bounds);
    drawAmplitudeLabels(g, bounds);
    drawPeakFrequency(g, spectrumBounds);

    // ==== Display THD / THD+N (using file-static values) ====
    g.setColour(juce::Colours::yellow);
    g.setFont(14.0f);

    juce::String thdText = "THD: " + juce::String(currentTHD * 100.0f, 2) + " %";
    juce::String thdnText = "THD+N: " + juce::String(currentTHDplusN * 100.0f, 2) + " %";

    auto textArea = getLocalBounds().removeFromTop(25).reduced(45, 5);
    g.drawText(thdText, textArea, juce::Justification::topLeft);

    auto textArea2 = textArea.translated(0, 18);
    g.drawText(thdnText, textArea2, juce::Justification::topLeft);
}

void SpectrumAnalyzer::resized()
{
    // Handle component size changes
}

void SpectrumAnalyzer::timerCallback()
{
    if (nextFFTBlockReady)
    {
        drawNextFrameOfSpectrum();
        nextFFTBlockReady = false;
        repaint();
    }
}

void SpectrumAnalyzer::pushNextSampleIntoFifo(float sample) noexcept
{
    // If FIFO contains enough data, set flag to render next frame of spectrum in the next timer callback
   if (fifoIndex == fftSize)
    {
        if (!nextFFTBlockReady)
        {
            juce::zeromem(fftData, sizeof(fftData));
            memcpy(fftData, fifo, sizeof(fifo));
            nextFFTBlockReady = true;
        }
        fifoIndex = 0;
    }
    fifo[fifoIndex++] = sample;
}

static inline float sumBinsAround(const std::vector<float>& mags, int centerBin, int halfWidth)
{
    int start = std::max(0, centerBin - halfWidth);
    int end = std::min((int)mags.size() - 1, centerBin + halfWidth);
    float sum = 0.0f;
    for (int i = start; i <= end; ++i)
        sum += mags[i] * mags[i];
    return sum;
}

void SpectrumAnalyzer::drawNextFrameOfSpectrum()
{
     if (currentSampleRate <= 0.0)
    {
        juce::zeromem(scopeData, sizeof(scopeData));
        return;
    }

    // Raw time domain samples are already stored in fftData (copied via pushNextSampleIntoFifo)
    // Apply window function to fftData
    window.multiplyWithWindowingTable(fftData, fftSize);

    // Perform FFT (returns single-sided magnitude information to fftData[0..fftSize/2])
    forwardFFT.performFrequencyOnlyForwardTransform(fftData);

    const int maxValidBin = fftSize / 2;
    const float nyquist = (float)(currentSampleRate * 0.5f);

    // ==== Normalize magnitude (linear amplitude) ====
    // scale: divide by fftSize; for single-sided spectrum multiply non-DC/non-Nyquist by 2
    float scale = 1.0f / (float)fftSize;

    // Get window's coherent gain:
    // For Hann window, coherent gain ≈ 0.5; Hamming ≈ 0.54.
    // Here we use Hann correction (if using Hamming, change this value to 0.54f)
    const float windowCoherentGain = 0.5f;

    std::vector<float> mags(maxValidBin + 1, 0.0f);
    for (int i = 0; i <= maxValidBin; ++i)
    {
        float mag = fftData[i] * scale;
        if (i > 0 && i < maxValidBin)
            mag *= 2.0f; // Single-sided spectrum
        // Correct window's amplitude loss (coherent gain)
        mag /= windowCoherentGain;
        mags[i] = mag;
    }

    // ==== Build scopeData (using your original logarithmic mapping, but with mags[]) ====
    auto mindB = -100.0f;
    auto maxdB = 0.0f;
    juce::zeromem(scopeData, sizeof(scopeData));

    for (int i = 0; i < scopeSize; ++i)
    {
        float proportion = (float)i / (float)(scopeSize - 1);
        float minFreq = 20.0f;
        float maxFreq = std::min(20000.0f, (float)nyquist);
        float logMinFreq = std::log10(minFreq);
        float logMaxFreq = std::log10(maxFreq);
        float logFreq = logMinFreq + proportion * (logMaxFreq - logMinFreq);
        float frequency = std::pow(10.0f, logFreq);

        int fftDataIndex = (int)std::floor(frequency * fftSize / (float)currentSampleRate);
        if (fftDataIndex < 0 || fftDataIndex > maxValidBin)
        {
            scopeData[i] = 0.0f;
            continue;
        }

        float mag = mags[fftDataIndex];
        if (mag <= 0.0f || !std::isfinite(mag)) mag = 1e-12f;

        float dB = juce::Decibels::gainToDecibels(mag);
        dB = juce::jlimit(mindB, maxdB, dB);

        float level = juce::jmap(dB, mindB, maxdB, 0.0f, 1.0f);
        if (!std::isfinite(level)) level = 0.0f;
        scopeData[i] = level;
    }

    // Smoothing
    for (int i = 1; i < scopeSize - 1; ++i)
        scopeData[i] = (scopeData[i - 1] * 0.2f + scopeData[i] * 0.6f + scopeData[i + 1] * 0.2f);

    // Update peak frequency (keeping your original implementation logic)
    findPeakFrequency();

    // ================= THD / THD+N Calculation =================
    // Calculate search range (20Hz - 20kHz)
    int minBin = std::max(1, (int)std::floor(20.0f * fftSize / currentSampleRate));
    int maxSearchBin = std::min(maxValidBin, (int)std::floor(20000.0f * fftSize / currentSampleRate));

    // Find maximum peak (fundamental) - using mags[]
    float maxVal = 0.0f;
    int maxBin = minBin;
    for (int i = minBin; i <= maxSearchBin; ++i)
    {
        if (mags[i] > maxVal)
        {
            maxVal = mags[i];
            maxBin = i;
        }
    }

    // Parabolic interpolation to get sub-bin position (more stable with log values)
    float peakBinFloat = (float)maxBin;
    if (maxBin > 1 && maxBin < maxValidBin - 1)
    {
        float y1 = std::log(mags[maxBin - 1] + 1e-20f);
        float y2 = std::log(mags[maxBin] + 1e-20f);
        float y3 = std::log(mags[maxBin + 1] + 1e-20f);
        float denom = (2.0f * y2 - y1 - y3);
        if (std::abs(denom) > 1e-20f)
        {
            float delta = 0.5f * (y3 - y1) / denom;
            delta = juce::jlimit(-0.5f, 0.5f, delta);
            peakBinFloat = maxBin + delta;
        }
    }

    // Total power (within valid search range)
    double totalPower = 0.0;
    for (int i = minBin; i <= maxSearchBin; ++i)
        totalPower += double(mags[i]) * double(mags[i]);

    // Fundamental power: sum around fundamental (±halfWidth bins)
    const int halfWidth = 3; // +/-3 bins, suitable for most cases; can be adjusted as needed
    int peakCenterBin = (int)std::round(peakBinFloat);
    double fundamentalPower = (double)sumBinsAround(mags, peakCenterBin, halfWidth);

    // Harmonic power
    double harmonicPower = 0.0;
    int maxHarmonics = 10; // Don't search endlessly, find up to 10 harmonics or up to Nyquist
    double binResolution = currentSampleRate / (double)fftSize;
    double fundamentalFreq = peakBinFloat * binResolution;

    for (int h = 2; h <= maxHarmonics; ++h)
    {
        double harmonicFreq = fundamentalFreq * (double)h;
        if (harmonicFreq >= nyquist - 1.0) break; // Stop if beyond Nyquist

        double harmonicBinF = harmonicFreq / binResolution;
        int harmonicCenter = (int)std::round(harmonicBinF);
        if (harmonicCenter < 1 || harmonicCenter > maxValidBin) continue;

        harmonicPower += (double)sumBinsAround(mags, harmonicCenter, halfWidth);
    }

    // Noise power = total power - fundamental - harmonics
    double noisePower = totalPower - fundamentalPower - harmonicPower;
    if (noisePower < 0.0)
        noisePower = 0.0;

    // Calculate THD / THD+N (based on power -> converted to RMS amplitude ratio)
    double thd = 0.0, thdn = 0.0;
    if (fundamentalPower > 0.0)
    {
        thd = std::sqrt(harmonicPower) / std::sqrt(fundamentalPower);
        thdn = std::sqrt(harmonicPower + noisePower) / std::sqrt(fundamentalPower);
    }

    // Smoothing (avoid jitter)
    const float smoothing = 0.85f; // Closer to 1 = smoother
    currentTHD = currentTHD * smoothing + (float)thd * (1.0f - smoothing);
    currentTHDplusN = currentTHDplusN * smoothing + (float)thdn * (1.0f - smoothing);
}

void SpectrumAnalyzer::drawFrequencyLabels(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    g.setColour(juce::Colours::lightgrey);
    g.setFont(10.0f);
    
    auto spectrumBounds = bounds.reduced(40, 20);
    auto labelArea = bounds.removeFromBottom(20);
    
    // Frequency labels - using the same logarithmic mapping as the spectrum
    float labelFreqs[] = { 20.0f, 50.0f, 100.0f, 500.0f, 1000.0f, 5000.0f, 10000.0f, 15000.0f, 20000.0f };
    juce::StringArray freqLabels = { "20", "50", "100", "500", "1k", "5k", "10k", "15k", "20k" };
    
    float minFreq = 20.0f;
    float maxFreq = juce::jmin(20000.0f, (float)(currentSampleRate * 0.5f));
    float logMinFreq = std::log10(minFreq);
    float logMaxFreq = std::log10(maxFreq);
    
    for (int i = 0; i < 9; ++i)
    {
        // Use the same logarithmic mapping as the spectrum drawing to calculate position
        float logFreq = std::log10(juce::jlimit(minFreq, maxFreq, labelFreqs[i]));
        float proportion = (logFreq - logMinFreq) / (logMaxFreq - logMinFreq);
        
        // Convert to scopeData index, then calculate x coordinate
        int scopeIndex = (int)(proportion * (scopeSize - 1));
        scopeIndex = juce::jlimit(0, scopeSize - 1, scopeIndex);
        
        const int pointsToRender = scopeSize - 1;
        auto x = spectrumBounds.getX() + (spectrumBounds.getWidth() * scopeIndex / (float)pointsToRender);
        
        g.drawText(freqLabels[i], x - 15, labelArea.getY(), 30, labelArea.getHeight(), 
                  juce::Justification::centred);
    }
}

void SpectrumAnalyzer::drawAmplitudeLabels(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    g.setColour(juce::Colours::lightgrey);
    g.setFont(10.0f);
    
    auto spectrumBounds = bounds.reduced(40, 20);
    auto labelArea = bounds.removeFromLeft(40);
    
    // Amplitude labels (0dB, -20dB, -40dB, -60dB, -80dB)
    juce::StringArray ampLabels = { "0", "-20", "-40", "-60", "-80" };
    
    for (int i = 0; i < 5; ++i)
    {
        auto y = spectrumBounds.getY() + (spectrumBounds.getHeight() * i / 4);
        g.drawText(ampLabels[i], labelArea.getX(), y - 8, labelArea.getWidth() - 5, 16, 
                  juce::Justification::centredRight);
    }
}

float SpectrumAnalyzer::getFrequencyForBin(int binIndex, double sampleRate)
{
    return (float)(binIndex * sampleRate / fftSize);
}

void SpectrumAnalyzer::findPeakFrequency()
{
   if (currentSampleRate <= 0.0)
        return;
        
    // Find peak frequency
    float maxLevel = 0.0f;
    int maxBin = 0;
    
    // Only search valid frequency range (20Hz - 20kHz)
    int minBin = juce::jmax(1, (int)(20.0f * fftSize / currentSampleRate));
    int maxSearchBin = juce::jmin(fftSize / 2, (int)(20000.0f * fftSize / currentSampleRate));
    
    // Set noise threshold
    const float noiseThreshold = 1e-6f;  // -120dB
    
    // Find maximum value
    for (int i = minBin; i < maxSearchBin; ++i)
    {
        if (fftData[i] > maxLevel && fftData[i] > noiseThreshold)
        {
            maxLevel = fftData[i];
            maxBin = i;
        }
    }
    
    // Only update peak when signal exceeds noise threshold
    if (maxLevel > noiseThreshold && maxBin > 0)
    {
        // Use parabolic interpolation for more precise frequency
        float interpolatedBin = maxBin;
        
        // Ensure we have enough points for interpolation
        if (maxBin > 1 && maxBin < fftSize / 2 - 1)
        {
            float y1 = std::log(fftData[maxBin - 1] + 1e-10f);
            float y2 = std::log(fftData[maxBin] + 1e-10f);
            float y3 = std::log(fftData[maxBin + 1] + 1e-10f);
            
            // Parabolic interpolation formula
            float delta = 0.5f * (y3 - y1) / (2.0f * y2 - y1 - y3);
            
            // Limit delta to reasonable range
            delta = juce::jlimit(-0.5f, 0.5f, delta);
            interpolatedBin = maxBin + delta;
        }
        
        // Calculate precise peak frequency
        float newPeakFreq = interpolatedBin * currentSampleRate / fftSize;
        
        // Calculate peak level (dB)
        float newPeakLevel = juce::Decibels::gainToDecibels(maxLevel) - 
                            juce::Decibels::gainToDecibels((float)fftSize);
        
        // Apply smoothing filter to reduce jumps
        const float smoothingFactor = 0.8f;  // 0.8 = smoother, 0.2 = faster response
        
        if (peakFrequency == 0.0f)  // First detection
        {
            peakFrequency = newPeakFreq;
            peakLevel = newPeakLevel;
        }
        else
        {
            // If frequency changes significantly (more than 10%), update quickly
            float freqChange = std::abs(newPeakFreq - peakFrequency) / peakFrequency;
            if (freqChange > 0.1f)
            {
                peakFrequency = newPeakFreq;
                peakLevel = newPeakLevel;
            }
            else
            {
                // Otherwise smooth the values
                peakFrequency = peakFrequency * smoothingFactor + newPeakFreq * (1.0f - smoothingFactor);
                peakLevel = peakLevel * smoothingFactor + newPeakLevel * (1.0f - smoothingFactor);
            }
        }
    }
    else
    {
        // When no valid signal, gradually reduce peak display
        peakLevel = peakLevel * 0.9f;
        if (peakLevel < -80.0f)
        {
            peakLevel = -100.0f;
            peakFrequency = 0.0f;
        }
    }
}

void SpectrumAnalyzer::drawPeakFrequency(juce::Graphics& g, juce::Rectangle<int> bounds)
{
     // Only display when peak level is above threshold and frequency is valid
    if (peakLevel > -80.0f && peakFrequency > 0.0f && currentSampleRate > 0.0)
    {
        // Display peak information text in top right corner
        g.setColour(juce::Colours::yellow);
        g.setFont(14.0f);
        
        // Format frequency display
        juce::String freqText;
        if (peakFrequency < 1000.0f)
            freqText = juce::String(peakFrequency, 1) + " Hz";
        else
            freqText = juce::String(peakFrequency / 1000.0f, 2) + " kHz";
        
        // Format level display
        juce::String levelText = juce::String(peakLevel, 1) + " dB";
        
        // Display peak frequency and level (in top right corner of component)
        juce::String peakText = "Peak: " + freqText + " @ " + levelText;
        auto textArea = getLocalBounds().removeFromTop(25).reduced(45, 5);
        g.drawText(peakText, textArea, juce::Justification::topRight);
        
        // bounds parameter is actually spectrumBounds (spectrum drawing area)
        // Use exactly the same mapping as in drawNextFrameOfSpectrum
        float minFreq = 20.0f;
        float maxFreq = juce::jmin(20000.0f, (float)(currentSampleRate * 0.5f));
        
        // Ensure peak frequency is within range
        float clampedPeakFreq = juce::jmax(minFreq, juce::jmin(maxFreq, peakFrequency));
        
        // Logarithmic mapping (consistent with drawNextFrameOfSpectrum)
        float logMinFreq = std::log10(minFreq);
        float logMaxFreq = std::log10(maxFreq);
        float logPeakFreq = std::log10(clampedPeakFreq);
        
        // Calculate index position in scopeData array
        float proportion = (logPeakFreq - logMinFreq) / (logMaxFreq - logMinFreq);
        int scopeIndex = (int)(proportion * (scopeSize - 1));
        scopeIndex = juce::jlimit(0, scopeSize - 1, scopeIndex);
        
        // Use same calculation as in paint function
        const int pointsToRender = scopeSize - 1;
        
        // Find actual maximum value in scopeData and its position
        float maxScopeValue = 0.0f;
        int maxScopeIndex = scopeIndex;  // Default to calculated index
        
        // Search for actual maximum near peak frequency (accounting for interpolation errors)
        int searchRange = 5;  // Search range
        int searchStart = juce::jmax(0, scopeIndex - searchRange);
        int searchEnd = juce::jmin((int)scopeSize, scopeIndex + searchRange + 1);
        
        for (int i = searchStart; i < searchEnd; i++)
        {
            if (scopeData[i] > maxScopeValue)
            {
                maxScopeValue = scopeData[i];
                maxScopeIndex = i;
            }
        }
        
        // Calculate x coordinate at position of actual maximum
        float x = bounds.getX() + (bounds.getWidth() * maxScopeIndex / (float)pointsToRender);
        
        // Use found maximum to calculate Y coordinate
        float y = juce::jmap(maxScopeValue, 0.0f, 1.0f,
                            (float)bounds.getBottom(),
                            (float)bounds.getY());
        
        // Draw vertical marker line (only from bottom to peak point)
        g.setColour(juce::Colours::yellow.withAlpha(0.7f));
        g.drawLine(x, bounds.getBottom(), x, y, 1.5f);
        
        // Draw peak point marker (red dot)
        g.setColour(juce::Colours::red);
        g.fillEllipse(x - 4.0f, y - 4.0f, 8.0f, 8.0f);
        
        // Draw white border
        g.setColour(juce::Colours::white);
        g.drawEllipse(x - 4.0f, y - 4.0f, 8.0f, 8.0f, 1.0f);
    }
}