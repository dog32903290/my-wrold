#include "AudioAnalyzerState.h"

#include <algorithm>
#include <cmath>

namespace myworld
{
void AudioAnalyzerState::processBlock (const float* const* inputChannels,
                                       int numInputChannels,
                                       int numSamples,
                                       float analysisGain) noexcept
{
    if (inputChannels == nullptr || numInputChannels <= 0 || numSamples <= 0)
    {
        rms.store (0.0f, std::memory_order_relaxed);
        peak.store (0.0f, std::memory_order_relaxed);
        loudness.store (0.0f, std::memory_order_relaxed);
        gate.store (0.0f, std::memory_order_relaxed);
        confidence.store (0.0f, std::memory_order_relaxed);
        active.store (false, std::memory_order_relaxed);
        return;
    }

    double sumSquares = 0.0;
    float blockPeak = 0.0f;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float mono = 0.0f;
        int channelsUsed = 0;

        for (int channel = 0; channel < numInputChannels; ++channel)
        {
            if (inputChannels[channel] != nullptr)
            {
                mono += inputChannels[channel][sample];
                ++channelsUsed;
            }
        }

        if (channelsUsed > 0)
            mono = (mono / static_cast<float> (channelsUsed)) * analysisGain;

        sumSquares += static_cast<double> (mono) * static_cast<double> (mono);
        blockPeak = std::max (blockPeak, std::abs (mono));
    }

    const auto blockRms = static_cast<float> (std::sqrt (sumSquares / static_cast<double> (numSamples)));
    const auto blockGate = blockPeak > 0.0001f ? 1.0f : 0.0f;
    rms.store (blockRms, std::memory_order_relaxed);
    peak.store (blockPeak, std::memory_order_relaxed);
    loudness.store (blockRms * blockGate, std::memory_order_relaxed);
    gate.store (blockGate, std::memory_order_relaxed);
    confidence.store (blockGate, std::memory_order_relaxed);
    active.store (blockGate > 0.0f, std::memory_order_relaxed);
    sampleCounter.fetch_add (static_cast<std::uint64_t> (numSamples), std::memory_order_relaxed);
}

AudioAnalyzerSnapshot AudioAnalyzerState::getSnapshot() const noexcept
{
    return {
        rms.load (std::memory_order_relaxed),
        peak.load (std::memory_order_relaxed),
        loudness.load (std::memory_order_relaxed),
        gate.load (std::memory_order_relaxed),
        confidence.load (std::memory_order_relaxed),
        active.load (std::memory_order_relaxed),
        sampleCounter.load (std::memory_order_relaxed)
    };
}
}
