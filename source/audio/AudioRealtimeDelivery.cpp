#include "AudioRealtimeDelivery.h"

#include <sstream>

namespace myworld
{
const char* audioRealtimeDeliveryStatusToString (AudioRealtimeDeliveryStatus status) noexcept
{
    switch (status)
    {
        case AudioRealtimeDeliveryStatus::empty:     return "empty";
        case AudioRealtimeDeliveryStatus::writing:   return "writing";
        case AudioRealtimeDeliveryStatus::repeated:  return "repeated";
        case AudioRealtimeDeliveryStatus::delivered: return "delivered";
    }

    return "unknown";
}

std::string makeAudioRealtimeDeliveryStatusText (const AudioRealtimeDeliveryResult& result)
{
    std::ostringstream text;
    text << "rt " << audioRealtimeDeliveryStatusToString (result.status)
         << " seq " << result.sequence
         << " skip " << result.skippedSnapshots
         << " over " << result.overwrittenSnapshots;
    return text.str();
}

std::uint64_t AudioRealtimeDelivery::slotIndexForSequence (std::uint64_t completedSequence) noexcept
{
    const auto publishIndex = (completedSequence / 2) - 1;
    return publishIndex % slotCount;
}

void AudioRealtimeDelivery::publishFromRealtime (const AudioAnalyzerSnapshot& snapshot) noexcept
{
    const auto completedSequence = sequence.load (std::memory_order_relaxed) + 2;
    auto& slot = slots[slotIndexForSequence (completedSequence)];

    slot.sequence.store (completedSequence - 1, std::memory_order_release);
    slot.rms.store (snapshot.rms, std::memory_order_relaxed);
    slot.peak.store (snapshot.peak, std::memory_order_relaxed);
    slot.loudness.store (snapshot.loudness, std::memory_order_relaxed);
    slot.gate.store (snapshot.gate, std::memory_order_relaxed);
    slot.confidence.store (snapshot.confidence, std::memory_order_relaxed);
    slot.active.store (snapshot.active, std::memory_order_relaxed);
    slot.sampleCounter.store (snapshot.sampleCounter, std::memory_order_relaxed);
    slot.sequence.store (completedSequence, std::memory_order_release);
    sequence.store (completedSequence, std::memory_order_release);
}

AudioRealtimeDeliveryResult AudioRealtimeDelivery::consumeLatest (std::uint64_t& lastSeenSequence) const noexcept
{
    AudioRealtimeDeliveryResult result;

    const auto latestSequence = sequence.load (std::memory_order_acquire);
    result.sequence = latestSequence;

    if (latestSequence == 0)
    {
        result.status = AudioRealtimeDeliveryStatus::empty;
        return result;
    }

    if ((latestSequence % 2) != 0)
    {
        result.status = AudioRealtimeDeliveryStatus::writing;
        return result;
    }

    if (latestSequence == lastSeenSequence)
    {
        result.status = AudioRealtimeDeliveryStatus::repeated;
        return result;
    }

    const auto& slot = slots[slotIndexForSequence (latestSequence)];
    const auto startSequence = slot.sequence.load (std::memory_order_acquire);
    if (startSequence != latestSequence || (startSequence % 2) != 0)
    {
        result.status = AudioRealtimeDeliveryStatus::writing;
        result.sequence = startSequence;
        return result;
    }

    AudioAnalyzerSnapshot snapshot;
    snapshot.rms = slot.rms.load (std::memory_order_relaxed);
    snapshot.peak = slot.peak.load (std::memory_order_relaxed);
    snapshot.loudness = slot.loudness.load (std::memory_order_relaxed);
    snapshot.gate = slot.gate.load (std::memory_order_relaxed);
    snapshot.confidence = slot.confidence.load (std::memory_order_relaxed);
    snapshot.active = slot.active.load (std::memory_order_relaxed);
    snapshot.sampleCounter = slot.sampleCounter.load (std::memory_order_relaxed);

    const auto endSequence = slot.sequence.load (std::memory_order_acquire);
    if (startSequence != endSequence || endSequence != latestSequence || (endSequence % 2) != 0)
    {
        result.status = AudioRealtimeDeliveryStatus::writing;
        result.sequence = endSequence;
        return result;
    }

    if (lastSeenSequence > 0 && endSequence > lastSeenSequence + 2)
    {
        const auto missedSnapshots = ((endSequence - lastSeenSequence) / 2) - 1;
        result.skippedSnapshots = missedSnapshots < slotCount ? missedSnapshots : slotCount - 1;
        result.overwrittenSnapshots = missedSnapshots - result.skippedSnapshots;
        result.droppedSnapshots = result.skippedSnapshots + result.overwrittenSnapshots;
    }

    lastSeenSequence = endSequence;
    result.available = true;
    result.status = AudioRealtimeDeliveryStatus::delivered;
    result.sequence = endSequence;
    result.snapshot = snapshot;
    return result;
}
}
