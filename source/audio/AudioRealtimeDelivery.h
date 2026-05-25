#pragma once

#include "AudioAnalyzerState.h"

#include <atomic>
#include <cstdint>

namespace myworld
{
struct AudioRealtimeDeliveryResult
{
    bool available = false;
    std::uint64_t sequence = 0;
    AudioAnalyzerSnapshot snapshot;
};

class AudioRealtimeDelivery
{
public:
    void publishFromRealtime (const AudioAnalyzerSnapshot& snapshot) noexcept;
    AudioRealtimeDeliveryResult consumeLatest (std::uint64_t& lastSeenSequence) const noexcept;

private:
    std::atomic<std::uint64_t> sequence { 0 };
    std::atomic<float> rms { 0.0f };
    std::atomic<float> peak { 0.0f };
    std::atomic<float> loudness { 0.0f };
    std::atomic<float> gate { 0.0f };
    std::atomic<float> confidence { 0.0f };
    std::atomic<bool> active { false };
    std::atomic<std::uint64_t> sampleCounter { 0 };
};
}
