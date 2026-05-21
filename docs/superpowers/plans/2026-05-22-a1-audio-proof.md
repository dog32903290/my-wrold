# A1 Audio Proof Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the first native audio proof: audio input enters a realtime-safe analyzer, UI shows `rms` / `peak` / `loudness`, and the app can dump audio proof evidence.

**Architecture:** Keep realtime audio measurement separate from JUCE UI. The audio callback writes bounded atomic analyzer state only; the message thread reads snapshots and updates meter rows. Proof dumps record whether live input was actually observed, so microphone permission blocks are explicit instead of hidden.

**Tech Stack:** C++20, JUCE `AudioDeviceManager` / `AudioIODeviceCallback`, CMake, existing native app shell.

---

## Overnight Gates

- Safe to run unattended: pure analyzer tests, CMake wiring, UI build, static proof dump shape.
- May require user presence: first live microphone access. macOS can show a Microphone permission prompt for `我的世界`; an agent cannot safely click or pre-approve this without the user.
- If microphone permission blocks live input, commit code and plan updates with A1 marked as "implemented but live proof pending permission"; do not mark live A1 as proven.
- Use `caffeinate` during execution if the user explicitly starts an overnight run so the machine does not sleep mid-build.
- Do not ask for or store the user's password. If a specific `sudo` command becomes necessary, pause and explain the exact command and reason.

## File Map

- Modify: `CMakeLists.txt`
  - Add `my_world_audio`.
  - Add `my_world_audio_analyzer_tests`.
  - Link `my-world` with `my_world_audio` and `juce::juce_audio_devices`.

- Create: `source/audio/AudioAnalyzerState.h`
  - Pure C++ realtime-safe analyzer state and snapshot types.
  - No JUCE dependency.

- Create: `source/audio/AudioAnalyzerState.cpp`
  - RMS / peak / loudness calculation.
  - Atomic snapshot storage.

- Create: `source/audio/AudioInputAnalyzer.h`
  - JUCE audio callback bridge.
  - Owns `AudioAnalyzerState`.

- Create: `source/audio/AudioInputAnalyzer.cpp`
  - Starts/stops default input device.
  - Callback calls `AudioAnalyzerState::processBlock`.
  - Clears any output buffers.

- Modify: `source/app/MainComponent.h`
  - Own `juce::AudioDeviceManager`, `AudioInputAnalyzer`, meter labels, and audio proof dump helper.

- Modify: `source/app/MainComponent.cpp`
  - Start audio input.
  - Update meter rows from analyzer snapshots.
  - Dump `debug/a1-audio-proof/audio_stats.json`.

- Modify: `source/app/Main.cpp`
  - Add `--dump-audio-proof-and-exit`.

- Create: `tests/AudioAnalyzerStateTests.cpp`
  - Tests RMS, peak, loudness, silence, and multi-channel mono mix.

- Modify: `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`
  - Keep A1 status accurate after each task.

---

### Task 1: Pure Analyzer State

**Files:**
- Create: `source/audio/AudioAnalyzerState.h`
- Create: `source/audio/AudioAnalyzerState.cpp`
- Create: `tests/AudioAnalyzerStateTests.cpp`
- Modify: `CMakeLists.txt`
- Modify: `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`

- [ ] **Step 1: Write failing analyzer tests**

Add `tests/AudioAnalyzerStateTests.cpp`:

```cpp
#include "AudioAnalyzerState.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
void expect (bool condition, const std::string& message)
{
    if (! condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        std::exit (1);
    }
}

void expectNear (float actual, float expected, float tolerance, const std::string& message)
{
    expect (std::abs (actual - expected) <= tolerance, message);
}
}

int main()
{
    myworld::AudioAnalyzerState analyzer;

    const float channel0[] = { 0.0f, 0.5f, -0.5f, 1.0f };
    const float* channels[] = { channel0 };
    analyzer.processBlock (channels, 1, 4, 1.0f);

    auto snapshot = analyzer.getSnapshot();
    expectNear (snapshot.rms, std::sqrt ((0.0f + 0.25f + 0.25f + 1.0f) / 4.0f), 0.0001f, "rms");
    expectNear (snapshot.peak, 1.0f, 0.0001f, "peak");
    expectNear (snapshot.loudness, snapshot.rms, 0.0001f, "loudness follows rms for first proof");
    expect (snapshot.active, "non-silent input should be active");
    expect (snapshot.sampleCounter == 4, "sample counter");

    const float silent[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    const float* silentChannels[] = { silent };
    analyzer.processBlock (silentChannels, 1, 4, 1.0f);

    snapshot = analyzer.getSnapshot();
    expectNear (snapshot.rms, 0.0f, 0.0001f, "silence rms");
    expectNear (snapshot.peak, 0.0f, 0.0001f, "silence peak");
    expect (! snapshot.active, "silence inactive");
    expect (snapshot.sampleCounter == 8, "sample counter accumulates");

    const float left[] = { 1.0f, 1.0f };
    const float right[] = { -1.0f, -1.0f };
    const float* stereo[] = { left, right };
    analyzer.processBlock (stereo, 2, 2, 1.0f);

    snapshot = analyzer.getSnapshot();
    expectNear (snapshot.rms, 0.0f, 0.0001f, "mono mix can cancel opposite phase stereo");
    expectNear (snapshot.peak, 0.0f, 0.0001f, "mono mix peak");

    std::cout << "audio analyzer state ok\n";
    return 0;
}
```

- [ ] **Step 2: Wire the test target and verify RED**

Modify `CMakeLists.txt`:

```cmake
add_library(my_world_audio
    source/audio/AudioAnalyzerState.cpp
)

target_include_directories(my_world_audio
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/audio
)

add_executable(my_world_audio_analyzer_tests
    tests/AudioAnalyzerStateTests.cpp
)

target_link_libraries(my_world_audio_analyzer_tests
    PRIVATE
        my_world_audio
)

add_test(NAME audio_analyzer_state COMMAND my_world_audio_analyzer_tests)
```

Run:

```bash
cmake --build build --target my_world_audio_analyzer_tests
```

Expected: FAIL because `AudioAnalyzerState.h` does not exist yet.

- [ ] **Step 3: Implement minimal analyzer state**

Create `source/audio/AudioAnalyzerState.h`:

```cpp
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
    std::atomic<bool> active { false };
    std::atomic<std::uint64_t> sampleCounter { 0 };
};
}
```

Create `source/audio/AudioAnalyzerState.cpp`:

```cpp
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
    rms.store (blockRms, std::memory_order_relaxed);
    peak.store (blockPeak, std::memory_order_relaxed);
    loudness.store (blockRms, std::memory_order_relaxed);
    active.store (blockPeak > 0.0001f, std::memory_order_relaxed);
    sampleCounter.fetch_add (static_cast<std::uint64_t> (numSamples), std::memory_order_relaxed);
}

AudioAnalyzerSnapshot AudioAnalyzerState::getSnapshot() const noexcept
{
    return {
        rms.load (std::memory_order_relaxed),
        peak.load (std::memory_order_relaxed),
        loudness.load (std::memory_order_relaxed),
        active.load (std::memory_order_relaxed),
        sampleCounter.load (std::memory_order_relaxed)
    };
}
}
```

- [ ] **Step 4: Verify GREEN**

Run:

```bash
cmake --build build --target my_world_audio_analyzer_tests
ctest --test-dir build --output-on-failure
```

Expected: `audio_analyzer_state` and `graph_contract` both pass.

- [ ] **Step 5: Update spec and commit**

Update A1 in `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`:

```text
- Proven: pure realtime-safe analyzer state can calculate rms, peak, loudness, active, and sampleCounter from input buffers.
- Not started: native audio device input bridge.
```

Run:

```bash
git diff --check
cmake --build build
ctest --test-dir build --output-on-failure
git add CMakeLists.txt source/audio/AudioAnalyzerState.h source/audio/AudioAnalyzerState.cpp tests/AudioAnalyzerStateTests.cpp docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md
git commit -m "Add realtime audio analyzer state"
```

---

### Task 2: JUCE Audio Input Bridge

**Files:**
- Create: `source/audio/AudioInputAnalyzer.h`
- Create: `source/audio/AudioInputAnalyzer.cpp`
- Modify: `CMakeLists.txt`
- Modify: `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`

- [ ] **Step 1: Add callback bridge header**

Create `source/audio/AudioInputAnalyzer.h`:

```cpp
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
```

- [ ] **Step 2: Add callback bridge implementation**

Create `source/audio/AudioInputAnalyzer.cpp`:

```cpp
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
```

- [ ] **Step 3: Link JUCE audio devices**

Modify `CMakeLists.txt`:

```cmake
if(EXISTS "${MY_WORLD_JUCE_DIR}/CMakeLists.txt")
    # after add_subdirectory("${MY_WORLD_JUCE_DIR}" ...)
target_sources(my_world_audio
    PRIVATE
        source/audio/AudioInputAnalyzer.cpp
)

target_link_libraries(my_world_audio
    PUBLIC
        juce::juce_audio_devices
)

target_link_libraries(my-world
    PRIVATE
        my_world_audio
        juce::juce_audio_devices
)
endif()
```

Keep `my_world_audio_analyzer_tests` linked only to the pure `AudioAnalyzerState.cpp` path. It must keep running even if the JUCE checkout is missing and only core tests are configured.

- [ ] **Step 4: Verify build**

Run:

```bash
cmake --build build
ctest --test-dir build --output-on-failure
```

Expected: full build and tests pass.

- [ ] **Step 5: Update spec and commit**

Update A1 in `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`:

```text
- Proven: JUCE audio callback bridge exists and only writes bounded analyzer state plus output silence.
- Not proven: live microphone input on this Mac until permission is granted and meter values move.
```

Run:

```bash
git diff --check
cmake --build build
ctest --test-dir build --output-on-failure
git add CMakeLists.txt source/audio/AudioInputAnalyzer.h source/audio/AudioInputAnalyzer.cpp docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md
git commit -m "Add native audio input bridge"
```

---

### Task 3: Meter Rows In Main UI

**Files:**
- Modify: `source/app/MainComponent.h`
- Modify: `source/app/MainComponent.cpp`
- Modify: `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`

- [ ] **Step 1: Add UI ownership**

Modify `source/app/MainComponent.h`:

```cpp
#include "AudioInputAnalyzer.h"
```

Make `MainComponent` inherit `private juce::Timer` and add:

```cpp
void timerCallback() override;
void startAudioInput();
void updateAudioMeters();

juce::AudioDeviceManager audioDeviceManager;
AudioInputAnalyzer audioInputAnalyzer;
juce::Label audioStatusLabel;
juce::Label rmsLabel;
juce::Label peakLabel;
juce::Label loudnessLabel;
```

- [ ] **Step 2: Start audio input**

In `source/app/MainComponent.cpp`, add:

```cpp
void MainComponent::startAudioInput()
{
    const auto error = audioDeviceManager.initialiseWithDefaultDevices (1, 0);

    if (error.isNotEmpty())
    {
        audioStatusLabel.setText ("audio input error: " + error, juce::dontSendNotification);
        return;
    }

    audioDeviceManager.addAudioCallback (&audioInputAnalyzer);
    audioStatusLabel.setText ("audio input ready", juce::dontSendNotification);
}
```

In the constructor:

```cpp
audioStatusLabel.setText ("audio input starting", juce::dontSendNotification);
addAndMakeVisible (audioStatusLabel);
addAndMakeVisible (rmsLabel);
addAndMakeVisible (peakLabel);
addAndMakeVisible (loudnessLabel);
startAudioInput();
startTimerHz (30);
```

In the destructor:

```cpp
stopTimer();
audioDeviceManager.removeAudioCallback (&audioInputAnalyzer);
```

- [ ] **Step 3: Update meter rows**

Add:

```cpp
void MainComponent::timerCallback()
{
    updateAudioMeters();
}

void MainComponent::updateAudioMeters()
{
    const auto snapshot = audioInputAnalyzer.getSnapshot();
    rmsLabel.setText ("rms " + juce::String (snapshot.rms, 4), juce::dontSendNotification);
    peakLabel.setText ("peak " + juce::String (snapshot.peak, 4), juce::dontSendNotification);
    loudnessLabel.setText ("loudness " + juce::String (snapshot.loudness, 4), juce::dontSendNotification);

    if (audioInputAnalyzer.getSampleRate() > 0.0)
        audioStatusLabel.setText ("audio input "
                                  + juce::String (audioInputAnalyzer.getSampleRate(), 0)
                                  + "Hz / "
                                  + juce::String (audioInputAnalyzer.getBufferSize())
                                  + " samples",
                                  juce::dontSendNotification);
}
```

Place the labels in `resized()` above or below the shader editor without covering the preview.

- [ ] **Step 4: Verify UI build**

Run:

```bash
cmake --build build
ctest --test-dir build --output-on-failure
```

Then run:

```bash
perl -e 'alarm 8; exec @ARGV' ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界
```

Expected:
- App opens.
- If macOS asks for Microphone permission, stop and ask the user to click Allow.
- If permission is already granted, meter rows appear and build remains stable.

- [ ] **Step 5: Update spec and commit**

Update A1:

```text
- Proven: UI reads analyzer snapshots outside the realtime callback and displays rms / peak / loudness rows.
- Not proven: live microphone values if macOS permission was not granted.
```

Run:

```bash
git diff --check
cmake --build build
ctest --test-dir build --output-on-failure
git add source/app/MainComponent.h source/app/MainComponent.cpp docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md
git commit -m "Show audio analyzer meter rows"
```

---

### Task 4: A1 Audio Proof Dump

**Files:**
- Modify: `source/app/Main.cpp`
- Modify: `source/app/MainComponent.h`
- Modify: `source/app/MainComponent.cpp`
- Modify: `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`

- [ ] **Step 1: Add command-line mode**

In `source/app/Main.cpp`, parse:

```cpp
const auto dumpAudioProofAndExit = commandLine.contains ("--dump-audio-proof-and-exit");
```

Pass it into `MainComponent`.

- [ ] **Step 2: Add audio proof dump helper**

In `MainComponent`, add:

```cpp
void dumpAudioProof();
```

Implementation:

```cpp
void MainComponent::dumpAudioProof()
{
    const auto directory = projectDirectory().getChildFile ("debug").getChildFile ("a1-audio-proof");
    directory.createDirectory();

    const auto snapshot = audioInputAnalyzer.getSnapshot();
    const auto json = juce::String()
        + "{\n"
        + "  \"sampleRate\": " + juce::String (audioInputAnalyzer.getSampleRate(), 0) + ",\n"
        + "  \"bufferSize\": " + juce::String (audioInputAnalyzer.getBufferSize()) + ",\n"
        + "  \"rms\": " + juce::String (snapshot.rms, 6) + ",\n"
        + "  \"peak\": " + juce::String (snapshot.peak, 6) + ",\n"
        + "  \"loudness\": " + juce::String (snapshot.loudness, 6) + ",\n"
        + "  \"active\": " + juce::String (snapshot.active ? "true" : "false") + ",\n"
        + "  \"sampleCounter\": " + juce::String (static_cast<juce::int64> (snapshot.sampleCounter)) + "\n"
        + "}\n";

    directory.getChildFile ("audio_stats.json").replaceWithText (json, false, false, "\n");
    statusLabel.setText ("audio proof dumped: " + directory.getFullPathName(), juce::dontSendNotification);
}
```

- [ ] **Step 3: Trigger delayed dump**

When `--dump-audio-proof-and-exit` is set, use:

```cpp
juce::Timer::callAfterDelay (2500, [safe = juce::Component::SafePointer<MainComponent> (this)]
{
    if (safe != nullptr)
        safe->dumpAudioProof();
});
```

After dump, quit the app.

- [ ] **Step 4: Verify proof dump**

Run:

```bash
rm -rf debug/a1-audio-proof
perl -e 'alarm 12; exec @ARGV' ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-audio-proof-and-exit
test -s debug/a1-audio-proof/audio_stats.json
sed -n '1,120p' debug/a1-audio-proof/audio_stats.json
```

Expected:
- `audio_stats.json` exists.
- `sampleCounter` is greater than `0` if audio callback ran.
- `active` may be false in a silent room; that is not a failure.
- If sample rate is `0`, mark live device proof blocked and do not claim A1 live proof.

- [ ] **Step 5: Update spec and commit**

Update A1:

```text
- Proven: A1 can dump `debug/a1-audio-proof/audio_stats.json`.
- Proven or blocked: live input status, based on current `audio_stats.json`.
```

Run:

```bash
git diff --check
cmake --build build
ctest --test-dir build --output-on-failure
git add source/app/Main.cpp source/app/MainComponent.h source/app/MainComponent.cpp docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md
git commit -m "Add audio analyzer proof dump"
```

---

### Task 5: Loudness Uniform Closure

**Files:**
- Modify: `source/core/GraphContract.cpp`
- Modify: `source/render/OpenGLShaderPreview.h`
- Modify: `source/render/OpenGLShaderPreview.cpp`
- Modify: `source/app/MainComponent.cpp`
- Modify: `tests/GraphContractTests.cpp`
- Modify: `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`

- [ ] **Step 1: Extend graph contract test**

In `tests/GraphContractTests.cpp`, add:

```cpp
expect (myworld::usesSystemUniform (shader, "u_loudness"), "default shader declares u_loudness");
```

Expected RED: test fails because `u_loudness` is not in the shader node yet.

- [ ] **Step 2: Add `u_loudness` to default shader graph and shader**

In `source/core/GraphContract.cpp`, add `u_loudness` to the Shader node uniforms and default shader source:

```glsl
uniform float u_loudness;
```

Use it subtly in the default shader:

```glsl
float pulse = 1.0 + clamp(u_loudness, 0.0, 1.0) * 0.35;
vec3 color = mix(vec3(0.02, 0.025, 0.035), base * pulse, ring);
```

- [ ] **Step 3: Add preview uniform setter**

In `OpenGLShaderPreview.h`, add:

```cpp
void setLoudness (float newLoudness);
std::atomic<float> loudness { 0.0f };
std::unique_ptr<juce::OpenGLShaderProgram::Uniform> loudnessUniform;
```

In `OpenGLShaderPreview.cpp`, bind `u_loudness` after compile and set it during render.

- [ ] **Step 4: Feed analyzer snapshot into preview**

In `MainComponent::updateAudioMeters()`:

```cpp
preview.setLoudness (snapshot.loudness);
```

- [ ] **Step 5: Verify visual/audio closure**

Run:

```bash
cmake --build build
ctest --test-dir build --output-on-failure
rm -rf debug/v1-shader-proof
perl -e 'alarm 12; exec @ARGV' ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-proof-and-exit
test -s debug/v1-shader-proof/frame.png
test -s debug/v1-shader-proof/node_stats.json
```

Expected:
- Build and tests pass.
- V1 dump still works.
- `node_stats.json` includes `u_loudness` in `systemUniforms`.

- [ ] **Step 6: Update spec and commit**

Update A1:

```text
- Proven: analyzer loudness snapshot is wired to shader uniform `u_loudness`.
```

If live microphone permission is not granted, write:

```text
- Code path exists: `loudness -> u_loudness`, but live acoustic response remains blocked by microphone permission.
```

Run:

```bash
git diff --check
cmake --build build
ctest --test-dir build --output-on-failure
git add source/core/GraphContract.cpp source/render/OpenGLShaderPreview.h source/render/OpenGLShaderPreview.cpp source/app/MainComponent.cpp tests/GraphContractTests.cpp docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md
git commit -m "Wire loudness to shader uniform"
```

---

## Stop Conditions

- Stop before claiming live A1 proof if `audio_stats.json` shows `sampleRate: 0` or `sampleCounter: 0`.
- Stop before claiming realtime safety if any callback code allocates, locks, logs, opens files, parses JSON, or calls UI.
- Stop before C1 compound work if A1 has no committed analyzer state and no updated spec.
- Stop before node editor work; ImGui remains parked until A1 has evidence.

## Final Verification

Run:

```bash
git diff --check
cmake --build build
ctest --test-dir build --output-on-failure
rm -rf debug/v1-shader-proof debug/a1-audio-proof
perl -e 'alarm 12; exec @ARGV' ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-proof-and-exit
test -s debug/v1-shader-proof/frame.png
test -s debug/v1-shader-proof/cook_order.json
test -s debug/v1-shader-proof/node_stats.json
perl -e 'alarm 12; exec @ARGV' ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-audio-proof-and-exit
test -s debug/a1-audio-proof/audio_stats.json
```

Expected:
- Build passes.
- All CTest tests pass.
- V1 shader proof still dumps.
- A1 audio proof dumps. If live microphone permission is missing, plan/spec states that live proof is blocked rather than complete.
