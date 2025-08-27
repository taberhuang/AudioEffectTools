#include "AudioRecorder.h"

AudioRecorder::AudioRecorder()
{
    recordBuffer.setSize(2, maxRecordingLength);
    recordBuffer.clear();
}

AudioRecorder::~AudioRecorder()
{
    if (recording)
        stopRecording();
}

void AudioRecorder::startRecording(double sampleRate)
{
    if (recording)
        return;
        
    currentSampleRate = sampleRate;
    bufferWritePosition = 0;
    recordBuffer.clear();
    currentFileName = generateFileName();
    recording = true;
    
    juce::Logger::writeToLog("Recording started: " + currentFileName);
}

void AudioRecorder::stopRecording()
{
    if (!recording)
        return;
        
    recording = false;
    writeWavFile();
    
    juce::Logger::writeToLog("Recording stopped: " + currentFileName);
}

void AudioRecorder::addSamples(float leftSample, float rightSample)
{
    if (!recording || bufferWritePosition >= maxRecordingLength)
        return;
        
    recordBuffer.setSample(0, bufferWritePosition, leftSample);   // Left channel (input)
    recordBuffer.setSample(1, bufferWritePosition, rightSample); // Right channel (effect output)
    bufferWritePosition++;
}

void AudioRecorder::writeWavFile()
{
    if (bufferWritePosition == 0)
        return;
        
    // Get the executable directory
    juce::File exeFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile);
    juce::File outputDir = exeFile.getParentDirectory();
    juce::File outputFile = outputDir.getChildFile(currentFileName);
    
    // Create the audio format writer
    juce::WavAudioFormat wavFormat;
    std::unique_ptr<juce::AudioFormatWriter> writer;
    
    {
        juce::FileOutputStream* outputStream = new juce::FileOutputStream(outputFile);
        if (outputStream->openedOk())
        {
            writer.reset(wavFormat.createWriterFor(outputStream, 
                                                  currentSampleRate,
                                                  2, // stereo
                                                  16, // 16-bit
                                                  {},
                                                  0));
        }
        else
        {
            delete outputStream;
        }
    }
    
    if (writer != nullptr)
    {
        // Write the recorded audio data
        writer->writeFromAudioSampleBuffer(recordBuffer, 0, bufferWritePosition);
        writer.reset(); // This will close the file
        
        juce::Logger::writeToLog("WAV file saved: " + outputFile.getFullPathName());
        
        // Show dialog to user with file save location
        juce::String fileName = outputFile.getFileName();
        juce::String fullPath = outputFile.getFullPathName();
        
        // Format path with line breaks every 30 characters
        juce::String formattedPath;
        for (int i = 0; i < fullPath.length(); i += 30)
        {
            if (i > 0) formattedPath += "\n";
            formattedPath += fullPath.substring(i, i + 30);
        }
        
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::InfoIcon,
            "Recording Saved",
            "\n\nSaved at:\n" + formattedPath,
            "OK"
        );
    }
    else
    {
        juce::Logger::writeToLog("Failed to create WAV file: " + outputFile.getFullPathName());
    }
}

juce::String AudioRecorder::generateFileName()
{
    juce::Time currentTime = juce::Time::getCurrentTime();
    
    return juce::String::formatted("Recording_%04d%02d%02d_%02d%02d%02d.wav",
                                   currentTime.getYear(),
                                   currentTime.getMonth() + 1,
                                   currentTime.getDayOfMonth(),
                                   currentTime.getHours(),
                                   currentTime.getMinutes(),
                                   currentTime.getSeconds());
}