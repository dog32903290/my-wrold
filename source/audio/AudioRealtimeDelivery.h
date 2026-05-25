#pragma once

#include "AudioAnalyzerState.h"

#include <array>
#include <atomic>
#include <cstdint>
#include <string>

namespace myworld
{
enum class AudioRealtimeDeliveryStatus
{
    empty,
    writing,
    repeated,
    delivered
};

struct AudioRealtimeDeliveryResult
{
    bool available = false;
    AudioRealtimeDeliveryStatus status = AudioRealtimeDeliveryStatus::empty;
    std::uint64_t sequence = 0;
    std::uint64_t droppedSnapshots = 0;
    std::uint64_t skippedSnapshots = 0;
    std::uint64_t overwrittenSnapshots = 0;
    AudioAnalyzerSnapshot snapshot;
};

const char* audioRealtimeDeliveryStatusToString (AudioRealtimeDeliveryStatus status) noexcept;
std::string makeAudioRealtimeDeliveryStatusText (const AudioRealtimeDeliveryResult& result);

class AudioRealtimeDelivery
{
public:
    void publishFromRealtime (const AudioAnalyzerSnapshot& snapshot) noexcept;
    AudioRealtimeDeliveryResult consumeLatest (std::uint64_t& lastSeenSequence) const noexcept;

private:
    struct Slot
    {
        std::atomic<std::uint64_t> sequence { 0 };
        std::atomic<float> rms { 0.0f };
        std::atomic<float> peak { 0.0f };
        std::atomic<float> loudness { 0.0f };
        std::atomic<float> gate { 0.0f };
        std::atomic<float> confidence { 0.0f };
        std::atomic<bool> active { false };
        std::atomic<std::uint64_t> sampleCounter { 0 };
    };

    static constexpr std::uint64_t slotCount = 4;
    static std::uint64_t slotIndexForSequence (std::uint64_t completedSequence) noexcept;

    std::atomic<std::uint64_t> sequence { 0 };
    std::array<Slot, slotCount> slots;
};
}
