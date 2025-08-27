#include "AudioEngine.h"
#include <cmath>

SignalGenAudioEngine::SignalGenAudioEngine()
{
}

SignalGenAudioEngine::~SignalGenAudioEngine() = default;

void SignalGenAudioEngine::prepareToPlay(double newSampleRate, int newSamplesPerBlock)
{
    sampleRate = newSampleRate;
    samplesPerBlock = newSamplesPerBlock;
    leftPhase = 0.0;
    rightPhase = 0.0;
    sweepPhase = 0.0;
}

void SignalGenAudioEngine::processBlock(juce::AudioBuffer<float>& buffer)
{
    if (!running)
    {
        buffer.clear();
        return;
    }

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    if (numChannels < 2) { buffer.clear(); return; }

    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);

    const float leftGain = dbToGain(leftChannelLevel);
    const float rightGain = dbToGain(rightChannelLevel);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float leftSample = 0.0f;
        float rightSample = 0.0f;

        if (sweepSpeed == SweepSpeed::WhiteNoise)
        {
            // Generate white noise with channel-mode and lock semantics similar to tone paths
            float noise = random.nextFloat() * 2.0f - 1.0f;
            leftSample = noise;

            if (lockChannels)
            {
                switch (channelMode.load())
                {
                    case ChannelMode::InPhase:
                        rightSample = noise;
                        break;
                    case ChannelMode::Phase180:
                        rightSample = -noise;
                        break;
                    case ChannelMode::Independent:
                        rightSample = noise; // locked but independent makes little sense; mirror left
                        break;
                }
            }
            else
            {
                // Unlocked: independent right channel, use separate RNG
                rightSample = randomRight.nextFloat() * 2.0f - 1.0f;
            }
        }
        else if (sweepSpeed == SweepSpeed::PinkNoise)
        {
            // Generate pink noise with channel-mode and lock semantics similar to tone paths
            float noise = generatePinkNoise();
            leftSample = noise;

            if (lockChannels)
            {
                switch (channelMode.load())
                {
                    case ChannelMode::InPhase:
                        rightSample = noise;
                        break;
                    case ChannelMode::Phase180:
                        rightSample = -noise;
                        break;
                    case ChannelMode::Independent:
                        rightSample = noise;
                        break;
                }
            }
            else
            {
                rightSample = generatePinkNoise();
            }
        }
        else
        {
            float leftFreq, rightFreq;

            if (sweepSpeed == SweepSpeed::NoSweep || sweepSpeed == SweepSpeed::Manual)
            {
                leftFreq = leftManualFrequency;
                rightFreq = lockChannels ? leftManualFrequency : rightManualFrequency;
            }
            else
            {
                float sweepFreq = calculateSweepFrequency(sweepPhase);
                leftFreq = sweepFreq;
                rightFreq = sweepFreq;

                float sweepRate = 0.0f;
                switch (sweepSpeed.load())
                {
                    case SweepSpeed::FastSmooth:
                    case SweepSpeed::FastStepped:
                        sweepRate = static_cast<float>(sweepSpeedValue / 1000.0);
                        break;
                    case SweepSpeed::Slow:
                        sweepRate = static_cast<float>(sweepSpeedValue / 10000.0);
                        break;
                    default:
                        break;
                }

                sweepPhase += sweepRate / sampleRate;
                if (sweepPhase >= 1.0)
                    sweepPhase -= 1.0;

                if (halfOctaveMarkerEnabled)
                    checkHalfOctaveMarker(sweepFreq);
            }

            currentFrequency = leftFreq;

            leftSample = generateSample(waveform, leftPhase);

            switch (channelMode.load())
            {
                case ChannelMode::InPhase:
                    rightSample = generateSample(waveform, lockChannels ? leftPhase : rightPhase);
                    break;
                case ChannelMode::Phase180:
                    rightSample = -generateSample(waveform, lockChannels ? leftPhase : rightPhase);
                    break;
                case ChannelMode::Independent:
                    rightSample = generateSample(waveform, rightPhase);
                    break;
            }

            updatePhase(leftPhase, leftFreq);
            if (!lockChannels)
                updatePhase(rightPhase, rightFreq);
            else
                rightPhase = leftPhase;

            if (sweepSpeed == SweepSpeed::FastStepped && phaseSteppedEnabled)
            {
                const int stepsPerCycle = 20;
                const double stepSize = 1.0 / stepsPerCycle;
                if (std::fmod(sweepPhase, stepSize) < (1.0 / sampleRate))
                {
                    leftPhase = 0.0;
                    rightPhase = 0.0;
                }
            }
        }

        leftChannel[sample] = leftSample * leftGain * (leftEnabled ? 1.0f : 0.0f);
        rightChannel[sample] = rightSample * rightGain * (rightEnabled ? 1.0f : 0.0f);
    }
}

void SignalGenAudioEngine::releaseResources()
{
    running = false;
    leftPhase = 0.0;
    rightPhase = 0.0;
    sweepPhase = 0.0;
}

void SignalGenAudioEngine::setWaveform(Waveform newWaveform) { waveform = newWaveform; }
void SignalGenAudioEngine::setFrequencyRange(float startFreq, float endFreq) { startFrequency = startFreq; endFrequency = endFreq; }
void SignalGenAudioEngine::setManualFrequency(float leftFreq, float rightFreq) { leftManualFrequency = leftFreq; rightManualFrequency = rightFreq; }
void SignalGenAudioEngine::setSweepMode(SweepMode mode) { sweepMode = mode; }
void SignalGenAudioEngine::setSweepSpeed(SweepSpeed speed) { sweepSpeed = speed; }
void SignalGenAudioEngine::setSweepSpeedValue(float value) { sweepSpeedValue = value; }
void SignalGenAudioEngine::setLeftChannelLevel(float levelDb) { leftChannelLevel = levelDb; }
void SignalGenAudioEngine::setRightChannelLevel(float levelDb) { rightChannelLevel = levelDb; }
void SignalGenAudioEngine::setChannelMode(ChannelMode mode) { channelMode = mode; }
void SignalGenAudioEngine::setLockChannels(bool lock) { lockChannels = lock; }
void SignalGenAudioEngine::setHalfOctaveMarker(bool enabled)
{
    halfOctaveMarkerEnabled = enabled;
    if (enabled)
    {
        lastMarkerFrequency = 0.0f;
        nextMarkerFrequency = startFrequency * std::pow(2.0f, 0.5f);
    }
}
void SignalGenAudioEngine::setPhaseSteppedEnabled(bool enabled) { phaseSteppedEnabled = enabled; }
void SignalGenAudioEngine::setLeftEnabled(bool enabled) { leftEnabled = enabled; }
void SignalGenAudioEngine::setRightEnabled(bool enabled) { rightEnabled = enabled; }
void SignalGenAudioEngine::setDutyCycle(float newDutyCycle) { dutyCycle = juce::jlimit(0.0f, 1.0f, newDutyCycle); }
void SignalGenAudioEngine::start()
{
    running = true;
    sweepPhase = 0.0;
    if (halfOctaveMarkerEnabled)
    {
        lastMarkerFrequency = 0.0f;
        nextMarkerFrequency = startFrequency * std::pow(2.0f, 0.5f);
    }
}
void SignalGenAudioEngine::stop() { running = false; }

float SignalGenAudioEngine::calculateSweepFrequency(double phase)
{
    float start = startFrequency;
    float end = endFrequency;
    if (sweepMode == SweepMode::Linear)
    {
        return start + (end - start) * static_cast<float>(phase);
    }
    else
    {
        float logStart = std::log10(start);
        float logEnd = std::log10(end);
        float logFreq = logStart + (logEnd - logStart) * static_cast<float>(phase);
        return std::pow(10.0f, logFreq);
    }
}

float SignalGenAudioEngine::generateSample(Waveform waveformType, double phase)
{
    switch (waveformType)
    {
        case Waveform::Sine:
            return static_cast<float>(std::sin(phase));
        case Waveform::Square:
            return phase < juce::MathConstants<double>::pi ? 1.0f : -1.0f;
        case Waveform::Triangle:
        {
            double normalizedPhase = phase / juce::MathConstants<double>::twoPi;
            if (normalizedPhase < 0.5)
                return static_cast<float>(4.0 * normalizedPhase - 1.0);
            else
                return static_cast<float>(3.0 - 4.0 * normalizedPhase);
        }
        case Waveform::Sawtooth:
        {
            double normalizedPhase = phase / juce::MathConstants<double>::twoPi;
            return static_cast<float>(2.0 * normalizedPhase - 1.0);
        }
        case Waveform::Pulse:
        {
            double normalizedPhase = phase / juce::MathConstants<double>::twoPi;
            return normalizedPhase < dutyCycle ? 1.0f : -1.0f;
        }
        default:
            return 0.0f;
    }
}

void SignalGenAudioEngine::updatePhase(double& phase, float frequency)
{
    const double phaseIncrement = (juce::MathConstants<double>::twoPi * frequency) / sampleRate;
    phase += phaseIncrement;
    while (phase >= juce::MathConstants<double>::twoPi)
        phase -= juce::MathConstants<double>::twoPi;
}

float SignalGenAudioEngine::dbToGain(float db)
{
    return std::pow(10.0f, db / 20.0f);
}

void SignalGenAudioEngine::checkHalfOctaveMarker(float freq)
{
    if (freq >= nextMarkerFrequency)
    {
        lastMarkerFrequency = nextMarkerFrequency;
        nextMarkerFrequency = nextMarkerFrequency * std::pow(2.0f, 0.5f);
        if (nextMarkerFrequency > endFrequency)
        {
            nextMarkerFrequency = startFrequency * std::pow(2.0f, 0.5f);
        }
    }
}

float SignalGenAudioEngine::generatePinkNoise()
{
    float output = 0.0f;
    pinkNoiseCounter++;
    for (int i = 0; i < 7; ++i)
    {
        if ((pinkNoiseCounter & (1 << i)) == 0)
        {
            pinkNoiseState[i] = random.nextFloat() * 2.0f - 1.0f;
        }
        output += pinkNoiseState[i];
    }
    output *= 0.11f;
    return juce::jlimit(-1.0f, 1.0f, output);
}


