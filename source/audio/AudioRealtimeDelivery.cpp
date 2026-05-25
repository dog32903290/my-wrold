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
         << " drop " << result.droppedSnapshots;
    return text.str();
}

void AudioRealtimeDelivery::publishFromRealtime (const AudioAnalyzerSnapshot& snapshot) noexcept
{
    auto writingSequence = sequence.load (std::memory_order_relaxed) + 1;
    if ((writingSequence % 2) == 0)
        ++writingSequence;

    sequence.store (writingSequence, std::memory_order_release);
    rms.store (snapshot.rms, std::memory_order_relaxed);
    peak.store (snapshot.peak, std::memory_order_relaxed);
    loudness.store (snapshot.loudness, std::memory_order_relaxed);
    gate.store (snapshot.gate, std::memory_order_relaxed);
    confidence.store (snapshot.confidence, std::memory_order_relaxed);
    active.store (snapshot.active, std::memory_order_relaxed);
    sampleCounter.store (snapshot.sampleCounter, std::memory_order_relaxed);
    sequence.store (writingSequence + 1, std::memory_order_release);
}

AudioRealtimeDeliveryResult AudioRealtimeDelivery::consumeLatest (std::uint64_t& lastSeenSequence) const noexcept
{
    AudioRealtimeDeliveryResult result;

    const auto startSequence = sequence.load (std::memory_order_acquire);
    result.sequence = startSequence;

    if (startSequence == 0)
    {
        result.status = AudioRealtimeDeliveryStatus::empty;
        return result;
    }

    if ((startSequence % 2) != 0)
    {
        result.status = AudioRealtimeDeliveryStatus::writing;
        return result;
    }

    if (startSequence == lastSeenSequence)
    {
        result.status = AudioRealtimeDeliveryStatus::repeated;
        return result;
    }

    AudioAnalyzerSnapshot snapshot;
    snapshot.rms = rms.load (std::memory_order_relaxed);
    snapshot.peak = peak.load (std::memory_order_relaxed);
    snapshot.loudness = loudness.load (std::memory_order_relaxed);
    snapshot.gate = gate.load (std::memory_order_relaxed);
    snapshot.confidence = confidence.load (std::memory_order_relaxed);
    snapshot.active = active.load (std::memory_order_relaxed);
    snapshot.sampleCounter = sampleCounter.load (std::memory_order_relaxed);

    const auto endSequence = sequence.load (std::memory_order_acquire);
    if (startSequence != endSequence || (endSequence % 2) != 0)
    {
        result.status = AudioRealtimeDeliveryStatus::writing;
        result.sequence = endSequence;
        return result;
    }

    if (lastSeenSequence > 0 && endSequence > lastSeenSequence + 2)
        result.droppedSnapshots = ((endSequence - lastSeenSequence) / 2) - 1;

    lastSeenSequence = endSequence;
    result.available = true;
    result.status = AudioRealtimeDeliveryStatus::delivered;
    result.sequence = endSequence;
    result.snapshot = snapshot;
    return result;
}
}
