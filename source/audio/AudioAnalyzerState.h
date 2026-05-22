#pragma once

#include <atomic>
#include <cstdint>

namespace myworld
{
struct AudioAnalyzerSnapshot
{
    float rms = 0.0f;
    float peak = 0.0f;
    float loudness = 0.0f;
    float gate = 0.0f;
    float confidence = 0.0f;
    bool active = false;
    std::uint64_t sampleCounter = 0;
};

class AudioAnalyzerState
{
public:
    void processBlock (const float* const* inputChannels,
                       int numInputChannels,
                       int numSamples,
                       float analysisGain) noexcept;

    AudioAnalyzerSnapshot getSnapshot() const noexcept;

private:
    std::atomic<float> rms { 0.0f };
    std::atomic<float> peak { 0.0f };
    std::atomic<float> loudness { 0.0f };
    std::atomic<float> gate { 0.0f };
    std::atomic<float> confidence { 0.0f };
    std::atomic<bool> active { false };
    std::atomic<std::uint64_t> sampleCounter { 0 };
};
}
