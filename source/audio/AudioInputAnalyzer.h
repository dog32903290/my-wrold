#pragma once

#include "AudioAnalyzerState.h"

#include <juce_audio_devices/juce_audio_devices.h>

#include <atomic>

namespace myworld
{
class AudioInputAnalyzer final : public juce::AudioIODeviceCallback
{
public:
    void audioDeviceAboutToStart (juce::AudioIODevice* device) override;
    void audioDeviceIOCallbackWithContext (const float* const* inputChannelData,
                                           int numInputChannels,
                                           float* const* outputChannelData,
                                           int numOutputChannels,
                                           int numSamples,
                                           const juce::AudioIODeviceCallbackContext& context) override;
    void audioDeviceStopped() override;

    AudioAnalyzerSnapshot getSnapshot() const noexcept;
    double getSampleRate() const noexcept;
    int getBufferSize() const noexcept;
    void setAnalysisGain (float newGain) noexcept;

private:
    AudioAnalyzerState analyzerState;
    std::atomic<float> analysisGain { 1.0f };
    std::atomic<double> sampleRate { 0.0 };
    std::atomic<int> bufferSize { 0 };
};
}
