#include "WaveformAnalyzer.h"
#include <cmath>

WaveformAnalyzer::WaveformAnalyzer()
{
    // Initialize the sample buffer with default size
    sampleBuffer.resize(currentBufferSize, 0.0f);
    
    // Start the timer to trigger repaints (30 fps)
    startTimerHz(30);
}

WaveformAnalyzer::~WaveformAnalyzer()
{
    // Stop the timer when the component is destroyed
    stopTimer();
}

void WaveformAnalyzer::paint(juce::Graphics& g)
{
    // Fill the background
    g.fillAll(backgroundColour);
    
    // Define the waveform area (excluding margins)
    auto waveformArea = getLocalBounds()
        .withTrimmedLeft(leftMargin)
        .withTrimmedBottom(bottomMargin)
        .withTrimmedRight(rightMargin);
    
    // Draw the grid
    drawGrid(g, waveformArea);
    
    // Draw the waveform
    drawWaveform(g, waveformArea);
    
    // Draw RMS value
    drawRMS(g, waveformArea);
    
    // Draw Y-axis labels
    drawYAxisLabels(g, getLocalBounds());
    
    // Draw X-axis labels
    drawXAxisLabels(g, getLocalBounds());
}

void WaveformAnalyzer::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    // Check if mouse is over the waveform area (not the labels)
    if (event.x > leftMargin)
    {
        if (event.mods.isCtrlDown())
        {
            // Ctrl + wheel: adjust X-axis (time window)
            if (wheel.deltaY > 0)
            {
                // Zoom in (reduce time window)
                xAxisTimeWindow /= zoomFactor;
            }
            else if (wheel.deltaY < 0)
            {
                // Zoom out (increase time window)
                xAxisTimeWindow *= zoomFactor;
            }
            
            // Clamp to valid range
            xAxisTimeWindow = juce::jlimit(minTimeWindow, maxTimeWindow, xAxisTimeWindow);
            
            // Update buffer size based on new time window
            updateBufferSize();
        }
        else if (event.mods.isShiftDown())
        {
            // Shift + wheel: pan X-axis (time offset)
            float panSpeed = xAxisTimeWindow * 0.1f;
            // Pan 10% of window per tick
            xAxisTimeOffset -= wheel.deltaY * panSpeed;  // Negative for natural scrolling
            
            // Limit panning to reasonable range based on buffer size
            float maxTimeInBuffer = static_cast<float>(currentBufferSize) / static_cast<float>(sampleRate);
            float maxOffset = (maxTimeInBuffer - xAxisTimeWindow) * 0.5f;
            maxOffset = juce::jmax(0.0f, maxOffset);
            xAxisTimeOffset = juce::jlimit(-maxOffset, maxOffset, xAxisTimeOffset);
        }
        else
        {
            // No modifier: adjust Y-axis (existing behavior)
            if (wheel.deltaY > 0)
            {
                // Zoom in (reduce yAxisMax)
                yAxisMax /= zoomFactor;
            }
            else if (wheel.deltaY < 0)
            {
                // Zoom out (increase yAxisMax)
                yAxisMax *= zoomFactor;
            }
            
            // Clamp to valid range
            yAxisMax = juce::jlimit(minYAxisMax, maxYAxisMax, yAxisMax);
        }
        
        // Trigger repaint to update the display
        repaint();
    }
}

void WaveformAnalyzer::resized()
{
    // Update samples per pixel based on component width
    auto waveformWidth = getWidth() - leftMargin;
    if (waveformWidth > 0)
    {
        updateBufferSize();
    }
}

void WaveformAnalyzer::setSampleRate(double newSampleRate)
{
    sampleRate = newSampleRate;
    updateBufferSize();
}

void WaveformAnalyzer::pushSample(float sample)
{
    // Add the sample to the buffer and advance the write position
    sampleBuffer[writePosition] = sample;
    writePosition = (writePosition + 1) % currentBufferSize;
}

void WaveformAnalyzer::timerCallback()
{
    // Update RMS and Peak calculations
    rmsValue = calculateRMS();
    peakValue = calculatePeak();
    
    // Trigger a repaint to update the waveform display
    repaint();
}

float WaveformAnalyzer::calculateRMS() const
{
    // Calculate samples to process based on time window
    float samplesPerSecond = static_cast<float>(sampleRate);
    int samplesToProcess = static_cast<int>(xAxisTimeWindow * samplesPerSecond);
    samplesToProcess = juce::jmin(samplesToProcess, currentBufferSize);
    
    // Calculate sample offset based on time offset
    int sampleOffset = static_cast<int>(xAxisTimeOffset * samplesPerSecond);
    
    // Calculate starting position in the circular buffer
    int centerPos = (writePosition - currentBufferSize / 2 + currentBufferSize) % currentBufferSize;
    int startPos = (centerPos - samplesToProcess / 2 - sampleOffset + currentBufferSize * 2) % currentBufferSize;
    
    // Calculate RMS
    double sumOfSquares = 0.0;
    int count = 0;
    
    for (int i = 0; i < samplesToProcess; ++i)
    {
        int index = (startPos + i) % currentBufferSize;
        float sample = sampleBuffer[index];
        sumOfSquares += sample * sample;
        count++;
    }
    
    if (count == 0) return -100.0f;
    
    float linearRMS = std::sqrt(sumOfSquares / count);
    if (linearRMS < 0.00001f) return -100.0f;
    return 20.0f * std::log10(linearRMS);
}

float WaveformAnalyzer::calculatePeak() const
{
    // Calculate samples to process based on time window
    float samplesPerSecond = static_cast<float>(sampleRate);
    int samplesToProcess = static_cast<int>(xAxisTimeWindow * samplesPerSecond);
    samplesToProcess = juce::jmin(samplesToProcess, currentBufferSize);
    
    // Calculate sample offset based on time offset
    int sampleOffset = static_cast<int>(xAxisTimeOffset * samplesPerSecond);
    
    // Calculate starting position in the circular buffer
    int centerPos = (writePosition - currentBufferSize / 2 + currentBufferSize) % currentBufferSize;
    int startPos = (centerPos - samplesToProcess / 2 - sampleOffset + currentBufferSize * 2) % currentBufferSize;
    
    // Find the peak value
    float maxAmplitude = 0.0f;
    for (int i = 0; i < samplesToProcess; ++i)
    {
        int index = (startPos + i) % currentBufferSize;
        float sample = std::abs(sampleBuffer[index]); // Use absolute value
        maxAmplitude = juce::jmax(maxAmplitude, sample);
    }
    
    if (maxAmplitude < 0.00001f) return -100.0f;
    return 20.0f * std::log10(maxAmplitude);
}

void WaveformAnalyzer::drawWaveform(juce::Graphics& g, const juce::Rectangle<int>& area)
{
    g.setColour(waveformColour);
    
    const float width = static_cast<float>(area.getWidth());
    const float height = static_cast<float>(area.getHeight());
    const float centerY = height / 2.0f;
    const float scaleY = (height / 2.0f) / yAxisMax;
    
    // Calculate samples to display based on time window
    float samplesPerSecond = static_cast<float>(sampleRate);
    int samplesToDisplay = static_cast<int>(xAxisTimeWindow * samplesPerSecond);
    samplesToDisplay = juce::jmin(samplesToDisplay, currentBufferSize);
    
    // Calculate sample offset based on time offset
    int sampleOffset = static_cast<int>(xAxisTimeOffset * samplesPerSecond);
    
    // Create a path for the waveform
    juce::Path waveformPath;
    
    // Calculate starting position in the circular buffer
    int centerPos = (writePosition - currentBufferSize / 2 + currentBufferSize) % currentBufferSize;
    int startPos = (centerPos - samplesToDisplay / 2 - sampleOffset + currentBufferSize * 2) % currentBufferSize;
    
    // Calculate step size for downsampling if needed
    int pixelsAvailable = area.getWidth();
    float stepSize = static_cast<float>(samplesToDisplay) / static_cast<float>(pixelsAvailable);
    
    if (stepSize < 1.0f)
    {
        // We have more pixels than samples, draw all samples
        stepSize = 1.0f;
    }
    
    // Start the path at the first point
    float x = static_cast<float>(area.getX());
    int readPos = startPos;
    float sampleValue = juce::jlimit(-yAxisMax, yAxisMax, sampleBuffer[readPos]);
    float y = static_cast<float>(area.getY()) + centerY - (sampleValue * scaleY);
    waveformPath.startNewSubPath(x, y);
    
    // Add points for display
    float accumulator = 0.0f;
    for (int pixelIndex = 1; pixelIndex < pixelsAvailable; ++pixelIndex)
    {
        accumulator += stepSize;
        int samplesToAdvance = static_cast<int>(accumulator);
        accumulator -= samplesToAdvance;
        
        // Find min/max in this pixel's range for accurate visualization
        float minVal = 1.0f;
        float maxVal = -1.0f;
        
        for (int i = 0; i < samplesToAdvance; ++i)
        {
            readPos = (readPos + 1) % currentBufferSize;
            float val = sampleBuffer[readPos];
            minVal = juce::jmin(minVal, val);
            maxVal = juce::jmax(maxVal, val);
        }
        
        x = static_cast<float>(area.getX()) + (static_cast<float>(pixelIndex) / static_cast<float>(pixelsAvailable - 1)) * width;
        
        if (stepSize > 1.0f && (maxVal - minVal) > 0.01f)
        {
            // Draw vertical line from min to max for this pixel
            float yMin = static_cast<float>(area.getY()) + centerY - (juce::jlimit(-yAxisMax, yAxisMax, maxVal) * scaleY);
            float yMax = static_cast<float>(area.getY()) + centerY - (juce::jlimit(-yAxisMax, yAxisMax, minVal) * scaleY);
            
            yMin = juce::jlimit(static_cast<float>(area.getY()), static_cast<float>(area.getBottom()), yMin);
            yMax = juce::jlimit(static_cast<float>(area.getY()), static_cast<float>(area.getBottom()), yMax);
            
            waveformPath.lineTo(x, yMin);
            waveformPath.lineTo(x, yMax);
        }
        else
        {
            // Use average or last sample
            sampleValue = juce::jlimit(-yAxisMax, yAxisMax, (minVal + maxVal) * 0.5f);
            y = static_cast<float>(area.getY()) + centerY - (sampleValue * scaleY);
            y = juce::jlimit(static_cast<float>(area.getY()), static_cast<float>(area.getBottom()), y);
            waveformPath.lineTo(x, y);
        }
    }
    
    // Draw the path with a 1.5 pixel thick stroke
    g.strokePath(waveformPath, juce::PathStrokeType(1.5f));
}

void WaveformAnalyzer::drawRMS(juce::Graphics& g, const juce::Rectangle<int>& area)
{
    // Draw RMS value text in right top corner
    g.setColour(juce::Colours::yellow);
    g.setFont(14.0f);
    juce::String rmsText = "RMS: " + juce::String(rmsValue, 1) + " dB";
    g.drawText(rmsText,
               area.getX() + area.getWidth() - 100,
               area.getY() + 5,
               95,
               20,
               juce::Justification::centredRight);
    
    // Draw Peak value text below RMS
    g.setColour(juce::Colours::yellow);
    g.setFont(14.0f);
    juce::String peakText = "Peak: " + juce::String(peakValue, 1) + " dB";
    g.drawText(peakText,
               area.getX() + area.getWidth() - 100,
               area.getY() + 25, // 20 pixels below RMS
               95,
               20,
               juce::Justification::centredRight);
}

void WaveformAnalyzer::drawGrid(juce::Graphics& g, const juce::Rectangle<int>& area)
{
    g.setColour(gridColour);
    
    const float width = static_cast<float>(area.getWidth());
    const float height = static_cast<float>(area.getHeight());
    const float x = static_cast<float>(area.getX());
    const float y = static_cast<float>(area.getY());
    
    // Draw horizontal center line (0 level)
    g.drawLine(x, y + height / 2.0f, x + width, y + height / 2.0f, 1.0f);
    
    // Determine Y-axis grid line spacing based on zoom level
    float gridStep;
    if (yAxisMax <= 0.25f)
        gridStep = 0.05f;
    else if (yAxisMax <= 0.5f)
        gridStep = 0.1f;
    else
        gridStep = 0.25f;
    
    // Draw horizontal grid lines
    for (float level = gridStep; level < yAxisMax; level += gridStep)
    {
        // Positive level
        float lineY = y + height * (0.5f - (level / yAxisMax) * 0.5f);
        g.drawLine(x, lineY, x + width, lineY, 0.5f);
        
        // Negative level
        lineY = y + height * (0.5f + (level / yAxisMax) * 0.5f);
        g.drawLine(x, lineY, x + width, lineY, 0.5f);
    }
    
    // Draw vertical grid lines (time divisions)
    // Draw center line (time = 0) with different style
    float centerX = x + width / 2.0f;
    g.setColour(gridColour.brighter(0.2f));
    g.drawLine(centerX, y, centerX, y + height, 1.0f);
    g.setColour(gridColour);
    
    // Draw other vertical lines
    const int numDivisions = 8;
    for (int i = 0; i <= numDivisions; ++i)
    {
        if (i == numDivisions / 2) continue; // Skip center line (already drawn)
        
        float lineX = x + width * static_cast<float>(i) / static_cast<float>(numDivisions);
        g.drawLine(lineX, y, lineX, y + height, 0.5f);
    }
}

void WaveformAnalyzer::drawYAxisLabels(juce::Graphics& g, const juce::Rectangle<int>& area)
{
    g.setColour(juce::Colours::white);
    g.setFont(10.0f);
    
    const float height = static_cast<float>(area.getHeight() - bottomMargin);
    const float textHeight = 20.0f;
    const float halfTextHeight = textHeight / 2.0f;
    
    // Determine label spacing based on zoom level
    float labelStep;
    int decimalPlaces;
    
    if (yAxisMax <= 0.25f)
    {
        labelStep = 0.05f;
        decimalPlaces = 2;
    }
    else if (yAxisMax <= 0.5f)
    {
        labelStep = 0.1f;
        decimalPlaces = 1;
    }
    else
    {
        labelStep = 0.25f;
        decimalPlaces = 2;
    }
    
    // Draw center line label (0)
    {
        float y = height * 0.5f;
        float labelY = y - halfTextHeight;
        labelY = juce::jlimit(0.0f, height - textHeight, labelY);
        
        g.drawText("0", 
                   0, 
                   static_cast<int>(labelY), 
                   leftMargin - 5, 
                   static_cast<int>(textHeight), 
                   juce::Justification::centredRight);
    }
    
    // Draw positive and negative labels
    for (float value = labelStep; value <= yAxisMax; value += labelStep)
    {
        // Format the label text
        juce::String labelText = juce::String(value, decimalPlaces);
        
        // Positive value
        {
            float y = height * (0.5f - (value / yAxisMax) * 0.5f);
            float labelY = y - halfTextHeight;
            labelY = juce::jlimit(0.0f, height - textHeight, labelY);
            
            g.drawText(labelText, 
                       0, 
                       static_cast<int>(labelY), 
                       leftMargin - 5, 
                       static_cast<int>(textHeight), 
                       juce::Justification::centredRight);
        }
        
        // Negative value
        {
            float y = height * (0.5f + (value / yAxisMax) * 0.5f);
            float labelY = y - halfTextHeight;
            labelY = juce::jlimit(0.0f, height - textHeight, labelY);
            
            g.drawText("-" + labelText, 
                       0, 
                       static_cast<int>(labelY), 
                       leftMargin - 5, 
                       static_cast<int>(textHeight), 
                       juce::Justification::centredRight);
        }
    }
    
    // Draw top and bottom edge labels if they don't coincide with regular labels
    const float epsilon = 0.001f;
    bool drawTopLabel = true;
    bool drawBottomLabel = true;
    
    // Check if top/bottom labels would overlap with existing labels
    for (float value = labelStep; value <= yAxisMax; value += labelStep)
    {
        if (std::abs(value - yAxisMax) < epsilon)
        {
            drawTopLabel = false;
            drawBottomLabel = false;
            break;
        }
    }
    
    if (drawTopLabel)
    {
        juce::String topLabel = juce::String(yAxisMax, decimalPlaces);
        g.drawText(topLabel, 
                   0, 0, 
                   leftMargin - 5, 
                   static_cast<int>(textHeight), 
                   juce::Justification::centredRight);
    }
    
    if (drawBottomLabel)
    {
        juce::String bottomLabel = "-" + juce::String(yAxisMax, decimalPlaces);
        g.drawText(bottomLabel, 
                   0, 
                   static_cast<int>(height - textHeight), 
                   leftMargin - 5, 
                   static_cast<int>(textHeight), 
                   juce::Justification::centredRight);
    }
    
    // Draw Y-axis line
    g.setColour(gridColour);
    g.drawLine(static_cast<float>(leftMargin), 0, 
               static_cast<float>(leftMargin), height, 1.0f);
}

void WaveformAnalyzer::drawXAxisLabels(juce::Graphics& g, const juce::Rectangle<int>& area)
{
    g.setColour(juce::Colours::white);
    g.setFont(10.0f);
    
    const float width = static_cast<float>(area.getWidth() - leftMargin - rightMargin);
    const float x = static_cast<float>(leftMargin);
    const float y = static_cast<float>(area.getHeight() - bottomMargin);
    
    // Draw X-axis line
    g.setColour(gridColour);
    g.drawLine(x, y, x + width, y, 1.0f);
    
    // Calculate time values
    float leftTime = xAxisTimeOffset - xAxisTimeWindow / 2.0f;
    float rightTime = xAxisTimeOffset + xAxisTimeWindow / 2.0f;
    
    // Determine appropriate time units and formatting
    int decimalPlaces = 1;
    float multiplier = 1000.0f; // Convert to milliseconds by default
    juce::String unit = "ms";
    
    if (xAxisTimeWindow < 0.01f)
    {
        decimalPlaces = 2;
    }
    else if (xAxisTimeWindow > 0.05f)
    {
        decimalPlaces = 0;
    }
    
    // Draw time labels
    g.setColour(textColour);
    const int numLabels = 9; // Same as grid divisions + 1
    for (int i = 0; i < numLabels; ++i)
    {
        float t = leftTime + (rightTime - leftTime) * static_cast<float>(i) / static_cast<float>(numLabels - 1);
        float labelX = x + width * static_cast<float>(i) / static_cast<float>(numLabels - 1);
        
        // Format time value
        juce::String timeText = juce::String(t * multiplier, decimalPlaces);
        
        // Special formatting for center (0)
        if (i == numLabels / 2)
        {
            timeText = "0";
        }
        
        // Draw the label
        g.drawText(timeText,
                   static_cast<int>(labelX - 30),
                   static_cast<int>(y + 5),
                   80,
                   20,
                   juce::Justification::centred);
    }
    
    // Draw time unit label
    g.drawText("Time (" + unit + ")",
               static_cast<int>(x + width / 2 - 50),
               static_cast<int>(y + 15),
               100,
               15,
               juce::Justification::centred);
}

void WaveformAnalyzer::updateBufferSize()
{
    // Calculate required buffer size based on time window and display width
    int requiredSamples = static_cast<int>(xAxisTimeWindow * sampleRate);
    
    // Add some overhead for panning
    requiredSamples = static_cast<int>(requiredSamples * 1.5f);
    
    // Round up to next power of 2 for efficient circular buffer
    int newBufferSize = 1;
    while (newBufferSize < requiredSamples && newBufferSize < maxBufferSize)
    {
        newBufferSize *= 2;
    }
    
    // Clamp to maximum
    newBufferSize = juce::jmin(newBufferSize, maxBufferSize);
    
    // Only resize if necessary
    if (newBufferSize != currentBufferSize)
    {
        std::vector<float> newBuffer(newBufferSize, 0.0f);
        
        // Copy existing data to new buffer
        if (!sampleBuffer.empty())
        {
            int copySize = juce::jmin(currentBufferSize, newBufferSize);
            int readPos = (writePosition - copySize + currentBufferSize) % currentBufferSize;
            
            for (int i = 0; i < copySize; ++i)
            {
                newBuffer[i] = sampleBuffer[readPos];
                readPos = (readPos + 1) % currentBufferSize;
            }
            
            writePosition = copySize % newBufferSize;
        }
        else
        {
            writePosition = 0;
        }
        
        sampleBuffer = std::move(newBuffer);
        currentBufferSize = newBufferSize;
    }
}