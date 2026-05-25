#include "AudioRealtimeDelivery.h"

namespace myworld
{
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
    if (startSequence == 0 || (startSequence % 2) != 0 || startSequence == lastSeenSequence)
        return result;

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
        return result;

    lastSeenSequence = endSequence;
    result.available = true;
    result.sequence = endSequence;
    result.snapshot = snapshot;
    return result;
}
}
