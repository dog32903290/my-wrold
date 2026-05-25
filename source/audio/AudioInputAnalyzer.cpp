#include "AudioInputAnalyzer.h"

#include <cstring>

namespace myworld
{
void AudioInputAnalyzer::audioDeviceAboutToStart (juce::AudioIODevice* device)
{
    if (device != nullptr)
    {
        sampleRate.store (device->getCurrentSampleRate(), std::memory_order_relaxed);
        bufferSize.store (device->getCurrentBufferSizeSamples(), std::memory_order_relaxed);
    }
}

void AudioInputAnalyzer::audioDeviceIOCallbackWithContext (const float* const* inputChannelData,
                                                           int numInputChannels,
                                                           float* const* outputChannelData,
                                                           int numOutputChannels,
                                                           int numSamples,
                                                           const juce::AudioIODeviceCallbackContext&)
{
    analyzerState.processBlock (inputChannelData,
                                numInputChannels,
                                numSamples,
                                analysisGain.load (std::memory_order_relaxed));
    realtimeDelivery.publishFromRealtime (analyzerState.getSnapshot());

    for (int channel = 0; channel < numOutputChannels; ++channel)
        if (outputChannelData[channel] != nullptr)
            std::memset (outputChannelData[channel], 0, sizeof (float) * static_cast<size_t> (numSamples));
}

void AudioInputAnalyzer::audioDeviceStopped()
{
    sampleRate.store (0.0, std::memory_order_relaxed);
    bufferSize.store (0, std::memory_order_relaxed);
}

AudioAnalyzerSnapshot AudioInputAnalyzer::getSnapshot() const noexcept
{
    return analyzerState.getSnapshot();
}

AudioRealtimeDeliveryResult AudioInputAnalyzer::consumeRealtimeDelivery (std::uint64_t& lastSeenSequence) const noexcept
{
    return realtimeDelivery.consumeLatest (lastSeenSequence);
}

double AudioInputAnalyzer::getSampleRate() const noexcept
{
    return sampleRate.load (std::memory_order_relaxed);
}

int AudioInputAnalyzer::getBufferSize() const noexcept
{
    return bufferSize.load (std::memory_order_relaxed);
}

void AudioInputAnalyzer::setAnalysisGain (float newGain) noexcept
{
    analysisGain.store (newGain, std::memory_order_relaxed);
}
}
