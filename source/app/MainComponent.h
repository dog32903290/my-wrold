#pragma once

#include "AudioInputAnalyzer.h"
#include "OpenGLShaderPreview.h"
#include "PerformancePreferences.h"
#include "PreferencesPanel.h"

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_gui_extra/juce_gui_extra.h>

namespace myworld
{
class MainComponent final : public juce::Component,
                            private juce::Timer
{
public:
    MainComponent (bool dumpProofOnStart = false,
                   bool dumpAudioProofOnStart = false,
                   bool dumpC2StorageProofOnStart = false,
                   bool quitAfterStartupDump = false);
    ~MainComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;
    void dumpProof();
    void dumpAudioProof();
    void dumpC2StorageProof();
    void quitAfterDelay();
    void setShaderStatus (juce::String message);
    void startAudioInput();
    void updateAudioMeters();
    void applyMidiPreferences (MidiPreferences preferences);
    void sendMidiForSnapshot (const AudioAnalyzerSnapshot& snapshot);

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
    GraphContract graph;
    PerformancePreferences performancePreferences;
    std::unique_ptr<juce::MidiOutput> midiOutput;
    juce::String openedMidiOutputIdentifier;
    juce::String midiStatus = "midi off";
    bool shouldQuitAfterStartupDump = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
}
