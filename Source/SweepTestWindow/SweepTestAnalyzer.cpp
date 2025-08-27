// SweepTestAnalyzer.cpp
#include "SweepTestAnalyzer.h"
#include <cmath>
#include <algorithm>
#include <vector>

// ==============================
// FrequencySpectrumDisplay
// ==============================
FrequencySpectrumDisplay::FrequencySpectrumDisplay(const juce::String& title)
    : displayTitle(title)
{
}

FrequencySpectrumDisplay::~FrequencySpectrumDisplay() {}

void FrequencySpectrumDisplay::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));

    auto bounds = getLocalBounds().reduced(5);

    // Title
    g.setColour(juce::Colours::white);
    g.setFont(14.0f);
    auto titleArea = bounds.removeFromTop(20);
    g.drawText(displayTitle, titleArea, juce::Justification::centred);

    auto plotArea = bounds.toFloat().reduced(40, 20);

    // Background
    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillRect(plotArea);

    // Grid
    drawGrid(g, plotArea);

    // Spectrum curve
    if (!magnitudeData.empty() && !frequencyData.empty())
    {
        juce::Path spectrumPath;
        bool firstPoint = true;

        for (size_t i = 0; i < magnitudeData.size() && i < frequencyData.size(); ++i)
        {
            float logFreq = std::log10(juce::jmax(20.0f, frequencyData[i]));
            float logMin = std::log10(20.0f);
            float logMax = std::log10(20000.0f);
            float xNorm = (logFreq - logMin) / (logMax - logMin);

            // dB mapping (-60dB to 0dB)
            float yNorm = 1.0f - (magnitudeData[i] + 60.0f) / 60.0f;
            yNorm = juce::jlimit(0.0f, 1.0f, yNorm);

            float x = plotArea.getX() + xNorm * plotArea.getWidth();
            float y = plotArea.getY() + yNorm * plotArea.getHeight();

            if (firstPoint)
            {
                spectrumPath.startNewSubPath(x, y);
                firstPoint = false;
            }
            else
            {
                spectrumPath.lineTo(x, y);
            }
        }

        g.setColour(juce::Colours::cyan);
        g.strokePath(spectrumPath, juce::PathStrokeType(2.0f));
    }

    // Border
    g.setColour(juce::Colours::white.withAlpha(0.5f));
    g.drawRect(plotArea, 1.0f);

    drawFrequencyLabels(g, plotArea);
    drawMagnitudeLabels(g, plotArea);
}

void FrequencySpectrumDisplay::resized() {}

void FrequencySpectrumDisplay::setSpectrumData(const std::vector<float>& magnitudes,
                                               const std::vector<float>& frequencies)
{
    magnitudeData = magnitudes;
    frequencyData = frequencies;
    repaint();
}

void FrequencySpectrumDisplay::clear()
{
    magnitudeData.clear();
    frequencyData.clear();
    repaint();
}

void FrequencySpectrumDisplay::drawGrid(juce::Graphics& g, const juce::Rectangle<float>& plotArea)
{
    g.setColour(juce::Colours::darkgrey.withAlpha(0.3f));

    // Vertical grid lines
    std::vector<float> freqLines = {50, 100, 200, 500, 1000, 2000, 5000, 10000};
    for (float freq : freqLines)
    {
        float logFreq = std::log10(freq);
        float logMin = std::log10(20.0f);
        float logMax = std::log10(20000.0f);
        float xNorm = (logFreq - logMin) / (logMax - logMin);
        float x = plotArea.getX() + xNorm * plotArea.getWidth();
        g.drawVerticalLine((int)x, (int)plotArea.getY(), (int)plotArea.getBottom());
    }

    // Horizontal grid lines
    for (int db = -60; db <= 0; db += 10)
    {
        float yNorm = 1.0f - (db + 60.0f) / 60.0f;
        float y = plotArea.getY() + yNorm * plotArea.getHeight();
        g.drawHorizontalLine((int)y, plotArea.getX(), plotArea.getRight());
    }
}

void FrequencySpectrumDisplay::drawFrequencyLabels(juce::Graphics& g, const juce::Rectangle<float>& plotArea)
{
    g.setColour(juce::Colours::lightgrey);
    g.setFont(10.0f);

    std::vector<std::pair<float, juce::String>> freqLabels = {
        {20, "20"}, {100, "100"}, {1000, "1k"}, {10000, "10k"}, {20000, "20k"}};

    for (const auto& kv : freqLabels)
    {
        float freq = kv.first;
        juce::String label = kv.second;
        float logFreq = std::log10(freq);
        float logMin = std::log10(20.0f);
        float logMax = std::log10(20000.0f);
        float xNorm = (logFreq - logMin) / (logMax - logMin);
        float x = plotArea.getX() + xNorm * plotArea.getWidth();

        g.drawText(label, x - 20, plotArea.getBottom() + 5, 40, 15, juce::Justification::centred);
    }

    g.drawText("Frequency (Hz)", plotArea.getCentreX() - 50,
               plotArea.getBottom() + 20, 100, 15, juce::Justification::centred);
}

void FrequencySpectrumDisplay::drawMagnitudeLabels(juce::Graphics& g, const juce::Rectangle<float>& plotArea)
{
    g.setColour(juce::Colours::lightgrey);
    g.setFont(10.0f);

    for (int db = -60; db <= 0; db += 20)
    {
        float yNorm = 1.0f - (db + 60.0f) / 60.0f;
        float y = plotArea.getY() + yNorm * plotArea.getHeight();
        g.drawText(juce::String(db) + "dB",
                   plotArea.getX() - 35, y - 7, 30, 15, juce::Justification::centredRight);
    }
}

// ==============================
// SweepTestAnalyzer
// ==============================
juce::Array<SweepTestAnalyzer*> SweepTestAnalyzer::openAnalyzers;
juce::CriticalSection SweepTestAnalyzer::analyzerLock;

SweepTestAnalyzer::SweepTestAnalyzer(const juce::File& audioFile)
    : DocumentWindow("Sweep Test Analysis - " + audioFile.getFileName(),
                     juce::Colours::darkgrey,
                     DocumentWindow::allButtons)
{
    content = std::make_unique<AnalyzerContent>(audioFile);
    setContentOwned(content.get(), true);

    setResizable(true, true);
    setResizeLimits(800, 600, 1600, 1200);
    setSize(1200, 800);
    centreWithSize(getWidth(), getHeight());
    setVisible(true);
    {
        const juce::ScopedLock sl(analyzerLock);
        openAnalyzers.add(this);
    }
}

SweepTestAnalyzer::~SweepTestAnalyzer()
{
    const juce::ScopedLock sl(analyzerLock);
    openAnalyzers.removeFirstMatchingValue(this);
}

void SweepTestAnalyzer::closeButtonPressed()
{
    juce::MessageManager::callAsync([this]() { delete this; });
}

void SweepTestAnalyzer::showAnalyzer(const juce::File& audioFile)
{
    (new SweepTestAnalyzer(audioFile))->toFront(true);
}

void SweepTestAnalyzer::deleteAllAnalyzers()
{
    juce::Array<SweepTestAnalyzer*> list;
    {
        const juce::ScopedLock sl(analyzerLock);
        list = openAnalyzers;
        openAnalyzers.clear();
    }
    for (auto* a : list) delete a;
}

// ==============================
// AnalyzerContent
// ==============================
SweepTestAnalyzer::AnalyzerContent::AnalyzerContent(const juce::File& audioFile)
{
    inputSpectrumDisplay = std::make_unique<FrequencySpectrumDisplay>("Input Signal Spectrum");
    outputSpectrumDisplay = std::make_unique<FrequencySpectrumDisplay>("Output Signal Spectrum");
    leftChannelDisplay = std::make_unique<FrequencySpectrumDisplay>("Left Channel (Input)");
    rightChannelDisplay = std::make_unique<FrequencySpectrumDisplay>("Right Channel (Output)");

    addAndMakeVisible(inputSpectrumDisplay.get());
    addAndMakeVisible(outputSpectrumDisplay.get());
    addAndMakeVisible(leftChannelDisplay.get());
    addAndMakeVisible(rightChannelDisplay.get());

    fileInfoLabel.setText("File: " + audioFile.getFileName(), juce::dontSendNotification);
    fileInfoLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(fileInfoLabel);

    analysisInfoLabel.setText("Analyzing...", juce::dontSendNotification);
    analysisInfoLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(analysisInfoLabel);

    analyzeAudioFile(audioFile);
}

SweepTestAnalyzer::AnalyzerContent::~AnalyzerContent() {}

void SweepTestAnalyzer::AnalyzerContent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2a2a2a));
}

void SweepTestAnalyzer::AnalyzerContent::resized()
{
    auto bounds = getLocalBounds().reduced(10);

    // Top information area
    fileInfoLabel.setBounds(bounds.removeFromTop(25));
    analysisInfoLabel.setBounds(bounds.removeFromTop(25));
    bounds.removeFromTop(10);

    // Divide into top and bottom sections
    auto topHalf = bounds.removeFromTop(bounds.getHeight() / 2).reduced(5);
    auto bottomHalf = bounds.reduced(5);

    // Top half: input spectrum
    inputSpectrumDisplay->setBounds(topHalf);

    // Bottom half: output spectrum
    outputSpectrumDisplay->setBounds(bottomHalf);
}

static std::vector<float> makeHannWindow(int N)
{
    std::vector<float> w(N);
    if (N <= 1) { w.assign(N, 1.0f); return w; }
    for (int n = 0; n < N; ++n)
        w[n] = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<double>::pi * n / (N - 1)));
    return w;
}

// Use Goertzel to estimate specific frequency magnitude on a windowed signal (normalized with window compensation)
static float goertzelMagnitude(const float* data, int N, float targetFreq, double sampleRate, 
                               const std::vector<float>& window)
{
    if (N <= 0) return 0.0f;
    double omega = 2.0 * juce::MathConstants<double>::pi * (double)targetFreq / sampleRate;
    double coeff = 2.0 * std::cos(omega);

    double s_prev = 0.0;
    double s_prev2 = 0.0;
    for (int n = 0; n < N; ++n)
    {
        double x = data[n] * (window.empty() ? 1.0 : window[n]);
        double s = x + coeff * s_prev - s_prev2;
        s_prev2 = s_prev;
        s_prev = s;
    }

    double real = s_prev - s_prev2 * std::cos(omega);
    double imag = s_prev2 * std::sin(omega);
    double magnitude = std::sqrt(real * real + imag * imag);

    // DFT single-sided magnitude normalization: magnitude ≈ (N/2) * A  => A ≈ (2 * magnitude) / N
    double magNormalized = (2.0 * magnitude) / (double)N;

    // Window compensation (coherent gain = mean(window))
    double cg = 1.0;
    if (!window.empty())
    {
        double sum = 0.0;
        for (auto v : window) sum += v;
        cg = sum / (double)window.size();
        if (cg <= 1e-12) cg = 1.0;
    }
    magNormalized /= cg;

    return (float)magNormalized;
}

void SweepTestAnalyzer::AnalyzerContent::analyzeAudioFile(const juce::File& file)
{
    juce::AudioFormatManager fm;
    fm.registerBasicFormats();

    auto reader = std::unique_ptr<juce::AudioFormatReader>(fm.createReaderFor(file));
    if (!reader)
    {
        analysisInfoLabel.setText("Error: Could not read audio file", juce::dontSendNotification);
        return;
    }

    auto numChannels = reader->numChannels;
    auto numSamples = (int)reader->lengthInSamples;
    auto sampleRate = reader->sampleRate;

    juce::AudioBuffer<float> buffer(numChannels, numSamples);
    reader->read(&buffer, 0, numSamples, 0, true, true);

    // Determine whether to use sweep-specific analysis (based on filename containing keywords)
    juce::String fn = file.getFileName().toLowerCase();
    bool isSweepFile = fn.contains("sweep") || fn.contains("chirp");

    std::vector<float> inMag, inFreq, outMag, outFreq;

    if (isSweepFile)
    {
        // Use sweep-specific analysis (logarithmic sweep -> time mapping -> narrowband detection)
        performSweepAnalysis(buffer.getReadPointer(0), numSamples, inMag, inFreq, sampleRate);

        if (numChannels > 1)
            performSweepAnalysis(buffer.getReadPointer(1), numSamples, outMag, outFreq, sampleRate);
        else
            outMag = inMag, outFreq = inFreq;
    }
    else
    {
        // Compatible with previous FFT Welch analysis (for steady-state signals)
        performFFT(buffer.getReadPointer(0), numSamples, inMag, inFreq, sampleRate);
        if (numChannels > 1)
            performFFT(buffer.getReadPointer(1), numSamples, outMag, outFreq, sampleRate);
        else
            outMag = inMag, outFreq = inFreq;
    }

    inputSpectrumDisplay->setSpectrumData(inMag, inFreq);
    outputSpectrumDisplay->setSpectrumData(outMag, outFreq);

    // **Fix**: Bottom left/right displays show Input / Output (previously both were filled with response)
    leftChannelDisplay->setSpectrumData(inMag, inFreq);
    rightChannelDisplay->setSpectrumData(outMag, outFreq);

    analysisInfoLabel.setText("Analysis complete. Sample rate: " + juce::String(sampleRate) +
                                  " Hz, Duration: " + juce::String(numSamples / sampleRate, 2) + " sec",
                              juce::dontSendNotification);
}

// Traditional FFT + Welch averaging (retained for compatibility with non-sweep signals)
// This implementation is the same as the improved version provided earlier: Blackman-Harris window, power averaging, window compensation
void SweepTestAnalyzer::AnalyzerContent::performFFT(const float* audioData, int numSamples,
                                                    std::vector<float>& magnitudes,
                                                    std::vector<float>& frequencies,
                                                    double sampleRate)
{
    const int fftOrder = 14; // 16384
    const int fftSize = 1 << fftOrder;
    juce::dsp::FFT fft(fftOrder);
    juce::dsp::WindowingFunction<float> window(fftSize,
                                               juce::dsp::WindowingFunction<float>::blackmanHarris);
    const float windowCoherentGain = 0.42f; // Blackman-Harris coherent gain (approx)

    const int hopSize = fftSize / 2;
    const int numWindows = juce::jmax(1, (numSamples - fftSize) / hopSize + 1);

    std::vector<double> avgPower(fftSize / 2, 0.0);
    std::vector<float> fftData(fftSize * 2, 0.0f);

    for (int w = 0; w < numWindows; ++w)
    {
        int startSample = w * hopSize;
        if (startSample + fftSize > numSamples) break;

        for (int i = 0; i < fftSize; ++i)
            fftData[i] = audioData[startSample + i];

        window.multiplyWithWindowingTable(fftData.data(), fftSize);
        fft.performFrequencyOnlyForwardTransform(fftData.data());

        for (int i = 0; i < fftSize / 2; ++i)
        {
            double mag = (double)fftData[i] / (double)fftSize;
            if (i > 0 && i < fftSize / 2) mag *= 2.0;
            mag /= windowCoherentGain;
            avgPower[i] += mag * mag;
        }
    }

    magnitudes.clear();
    frequencies.clear();

    for (int i = 1; i < fftSize / 2; ++i)
    {
        double freq = (double)i * sampleRate / (double)fftSize;
        if (freq >= 20.0 && freq <= 20000.0)
        {
            double rmsMag = std::sqrt(avgPower[i] / (double)juce::jmax(1, (numSamples - fftSize) / (fftSize/2) + 1));
            float magnitudeDB = 20.0f * std::log10((float)juce::jmax(rmsMag, 1e-12));
            frequencies.push_back((float)freq);
            magnitudes.push_back(magnitudeDB);
        }
    }
}

// ================ Sweep-specific analysis ================
// Assume signal is logarithmic sweep (from 20Hz to 20kHz), logarithmic frequency to time mapping:
// t(f) = T * log(f / fmin) / log(fmax / fmin)
// We take a short window (analysisLen) at position t(f) and use Goertzel to estimate the amplitude of that frequency.
void SweepTestAnalyzer::AnalyzerContent::performSweepAnalysis(const float* audioData, int numSamples,
                                                              std::vector<float>& magnitudes,
                                                              std::vector<float>& frequencies,
                                                              double sampleRate)
{
    // Parameters: number of output points, analysis window length (samples)
    const int numPoints = 2000;          // Display points (logarithmic spacing), you can adjust smaller/larger
    const int analysisLen = juce::jmin(4096, numSamples); // Use 4096 or smaller window
    const int halfLen = analysisLen / 2;

    const float fmin = 20.0f;
    const float fmax = juce::jmin(20000.0f, (float)(sampleRate * 0.5));

    if (numSamples < 16 || fmin >= fmax)
    {
        magnitudes.clear();
        frequencies.clear();
        return;
    }

    // Pre-generate Hann window, calculate coherent gain
    std::vector<float> win = makeHannWindow(analysisLen);
    double cg = 0.0;
    for (auto v : win) cg += v;
    cg /= (double)analysisLen;
    if (cg <= 1e-12) cg = 1.0;

    magnitudes.clear();
    frequencies.clear();
    magnitudes.reserve(numPoints);
    frequencies.reserve(numPoints);

    double logMin = std::log10(fmin);
    double logMax = std::log10(fmax);

    // Perform detection for each frequency point
    for (int i = 0; i < numPoints; ++i)
    {
        double prop = (double)i / (double)(numPoints - 1);
        double logF = logMin + prop * (logMax - logMin);
        double freq = std::pow(10.0, logF);

        // Map to time index (logarithmic sweep mapping)
        double timeProp = (logF - logMin) / (logMax - logMin);
        int centerSample = (int)std::round(timeProp * (double)(numSamples - 1));

        // Extract window data (center-aligned)
        int start = centerSample - halfLen;
        if (start < 0) start = 0;
        if (start + analysisLen > numSamples) start = juce::jmax(0, numSamples - analysisLen);

        // Here we directly give Goertzel a pointer (no copying entire segment to reduce memory), but ensure range is correct
        std::vector<float> segment(analysisLen, 0.0f);
        for (int n = 0; n < analysisLen; ++n)
            segment[n] = audioData[start + n];

        // Use Goertzel to estimate specific frequency amplitude on windowed segment
        float mag = goertzelMagnitude(segment.data(), analysisLen, (float)freq, sampleRate, win);

        // mag is linear amplitude (approximates real amplitude), convert to dB (expected range -60..0)
        float magDB = 20.0f * std::log10(juce::jmax(1e-12f, mag));

        frequencies.push_back((float)freq);
        magnitudes.push_back(magDB);
    }
}
