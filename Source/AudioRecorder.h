#pragma once

#include <JuceHeader.h>

class AudioRecorder
{
public:
    AudioRecorder();
    ~AudioRecorder();
    
    void startRecording(double sampleRate);
    void stopRecording();
    bool isRecording() const { return recording; }
    
    void addSamples(float leftSample, float rightSample);
    
private:
    void writeWavFile();
    juce::String generateFileName();
    
    bool recording = false;
    double currentSampleRate = 44100.0;
    juce::AudioBuffer<float> recordBuffer;
    int bufferWritePosition = 0;
    juce::String currentFileName;
    
    static const int maxRecordingLength = 44100 * 60 * 10; // 10 minutes at 44.1kHz
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioRecorder)
};