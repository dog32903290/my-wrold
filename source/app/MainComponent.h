#pragma once

#include "AudioInputAnalyzer.h"
#include "InteractionContract.h"
#include "LiveIOControlTimer.h"
#include "LiveIOMidiTeach.h"
#include "LiveIOStatusIndicator.h"
#include "OpenGLShaderPreview.h"
#include "PerformancePreferences.h"
#include "PreferencesPanel.h"
#include "StartupProof.h"

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include <atomic>
#include <string>
#include <vector>

namespace myworld
{
enum class PVDetectorProofKind;

class MainComponent final : public juce::Component,
                            private juce::MidiInputCallback,
                            private juce::Timer
{
public:
    explicit MainComponent (StartupProofOptions startupProofOptions = {});
    ~MainComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;
    void dumpProof();
    void dumpAudioProof();
    void dumpLiveIOProof();
    void dumpC2StorageProof();
    void dumpC3SaveWorkProof();
    void dumpC4AIWorkerSaveWorkProof();
    void dumpC5ModulePublishProof();
    void dumpC5AIWorkerModulePublishProof();
    void dumpC5VisibleModulePublishProof();
    void dumpC6AnalyzerFamilyProof();
    void dumpC6AIRepairLoopProof();
    void dumpPVAttackDetectorProof();
    void dumpPVDensityDetectorProof();
    void dumpPVSilenceDetectorProof();
    void dumpPVSustainDetectorProof();
    void dumpPVResidueDetectorProof();
    void dumpPVAggregatePressureProof();
    void dumpPVB1AnalyzerEnvironmentProof();
    void dumpPVDetectorProof (PVDetectorProofKind kind);
    void scheduleStartupProofs (const StartupProofOptions& options);
    void runStartupProofTask (StartupProofTaskId task);
    void finishProofDump (const juce::String& displayName,
                          const std::string& status,
                          const std::string& error,
                          const juce::File& directory);
    CommandResult saveActiveWork (GraphSession& session);
    CommandResult publishSelectedModule (GraphSession& session, const std::string& sourceNodeId);
    void quitAfterDelay();
    void setShaderStatus (juce::String message);
    void startAudioInput();
    void updateAudioMeters();
    void applyMidiPreferences (MidiPreferences preferences);
    void applyLiveIOPreferences (LiveIOPreferences preferences);
    void armMidiTeach (LiveIOMidiTeachTarget target);
    void cancelMidiTeach();
    int startMidiTeachListening();
    void stopMidiTeachListening();
    void handleMidiTeachMessage (LiveIOMidiTeachIncomingMessage message);
    void handleIncomingMidiMessage (juce::MidiInput* source,
                                    const juce::MidiMessage& message) override;
    void sendMidiForSnapshot (const AudioAnalyzerSnapshot& snapshot);
    void tickLiveIOControl (const AudioAnalyzerSnapshot& snapshot);

    OpenGLShaderPreview preview;
    juce::AudioDeviceManager audioDeviceManager;
    PreferencesPanel preferencesPanel;
    AudioInputAnalyzer audioInputAnalyzer;
    juce::TextEditor shaderEditor;
    juce::TextButton dumpProofButton;
    juce::Label graphLabel;
    juce::Label statusLabel;
    juce::Label audioStatusLabel;
    juce::Label rmsLabel;
    juce::Label peakLabel;
    juce::Label loudnessLabel;
    juce::Label activeLabel;
    juce::Label midiStatusLabel;
    juce::Label liveIOStatusLabel;
    GraphContract graph;
    PerformancePreferences performancePreferences;
    std::unique_ptr<juce::MidiOutput> midiOutput;
    juce::String openedMidiOutputIdentifier;
    juce::String midiStatus = "midi off";
    LiveIOControlTimerSendMode liveIOSendMode = LiveIOControlTimerSendMode::dryRun;
    LiveIOControlTimerState liveIOTimerState;
    juce::String liveIOStatus = "live io dry idle m0 o0";
    juce::String liveIOStatusTone = "idle";
    LiveIOMidiTeachState midiTeachState;
    std::atomic<bool> midiTeachArmedForCallback { false };
    bool midiTeachCallbackRegistered = false;
    int midiTeachInputCount = 0;
    std::vector<juce::String> midiInputsEnabledForTeach;
    bool shouldQuitAfterStartupDump = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
}
