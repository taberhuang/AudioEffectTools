#pragma once

#include <JuceHeader.h>
#include <atomic>

class SignalGenAudioEngine
{
public:
    enum class Waveform
    {
        Sine,
        Square,
        Triangle,
        Sawtooth,
        Pulse
    };

    enum class SweepMode
    {
        Linear,
        Logarithmic
    };

    enum class SweepSpeed
    {
        FastSmooth,
        FastStepped,
        Slow,
        Manual,
        WhiteNoise,
        PinkNoise,
        NoSweep
    };

    enum class ChannelMode
    {
        InPhase,
        Phase180,
        Independent
    };

    SignalGenAudioEngine();
    ~SignalGenAudioEngine();

    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void processBlock(juce::AudioBuffer<float>& buffer);
    void releaseResources();

    // Setters
    void setWaveform(Waveform waveform);
    void setFrequencyRange(float startFreq, float endFreq);
    void setManualFrequency(float leftFreq, float rightFreq);
    void setSweepMode(SweepMode mode);
    void setSweepSpeed(SweepSpeed speed);
    void setSweepSpeedValue(float value); // 0-100

    void setLeftChannelLevel(float levelDb);
    void setRightChannelLevel(float levelDb);
    void setChannelMode(ChannelMode mode);
    void setLockChannels(bool lock);
    void setHalfOctaveMarker(bool enabled);
    void setPhaseSteppedEnabled(bool enabled);
    void setLeftEnabled(bool enabled);
    void setRightEnabled(bool enabled);
    void setDutyCycle(float dutyCycle); // 0.0-1.0 for pulse wave

    // Control
    void start();
    void stop();
    bool isRunning() const { return running; }

    // Getters
    float getCurrentFrequency() const { return currentFrequency; }
    float getLastMarkerFrequency() const { return lastMarkerFrequency; }

private:
    // Audio parameters
    double sampleRate = 44100.0;
    int samplesPerBlock = 512;

    // State
    std::atomic<bool> running {false};
    std::atomic<Waveform> waveform {Waveform::Sine};
    std::atomic<SweepMode> sweepMode {SweepMode::Logarithmic};
    std::atomic<SweepSpeed> sweepSpeed {SweepSpeed::NoSweep};
    std::atomic<ChannelMode> channelMode {ChannelMode::InPhase};

    // Frequency parameters
    std::atomic<float> startFrequency {1000.0f};
    std::atomic<float> endFrequency {1000.0f};
    std::atomic<float> leftManualFrequency {1000.0f};
    std::atomic<float> rightManualFrequency {1000.0f};
    std::atomic<float> currentFrequency {1000.0f};
    std::atomic<float> sweepSpeedValue {50.0f};

    // Level parameters
    std::atomic<float> leftChannelLevel {0.0f}; // in dB
    std::atomic<float> rightChannelLevel {0.0f}; // in dB

    // Waveform parameters
    std::atomic<float> dutyCycle {0.5f}; // 0.0-1.0 for pulse wave

    // Options
    std::atomic<bool> lockChannels {true};
    std::atomic<bool> halfOctaveMarkerEnabled {false};
    std::atomic<bool> phaseSteppedEnabled {false};
    std::atomic<bool> leftEnabled {true};
    std::atomic<bool> rightEnabled {true};

    // Phase accumulators
    double leftPhase = 0.0;
    double rightPhase = 0.0;
    double sweepPhase = 0.0;

    // Marker tracking
    float lastMarkerFrequency = 0.0f;
    float nextMarkerFrequency = 0.0f;

    // Random number generators (separate per channel to avoid correlation when unlocked)
    juce::Random random;
    juce::Random randomRight;

    // Pink noise filter state (Voss-McCartney algorithm)
    float pinkNoiseState[7] = {0.0f};
    int pinkNoiseCounter = 0;

    // Helper methods
    float calculateSweepFrequency(double phase);
    float generateSample(Waveform waveform, double phase);
    void updatePhase(double& phase, float frequency);
    float dbToGain(float db);
    void checkHalfOctaveMarker(float freq);
    float generatePinkNoise();
};


