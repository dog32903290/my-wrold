#pragma once

#include "AudioInputAnalyzer.h"
#include "OpenGLShaderPreview.h"

#include <juce_gui_extra/juce_gui_extra.h>

namespace myworld
{
class MainComponent final : public juce::Component,
                            private juce::Timer
{
public:
    explicit MainComponent (bool dumpProofOnStart = false, bool quitAfterProofDump = false);
    ~MainComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;
    void dumpProof();
    void setShaderStatus (juce::String message);
    void startAudioInput();
    void updateAudioMeters();

    OpenGLShaderPreview preview;
    AudioInputAnalyzer audioInputAnalyzer;
    juce::AudioDeviceManager audioDeviceManager;
    juce::TextEditor shaderEditor;
    juce::TextButton dumpProofButton;
    juce::Label graphLabel;
    juce::Label statusLabel;
    juce::Label audioStatusLabel;
    juce::Label rmsLabel;
    juce::Label peakLabel;
    juce::Label loudnessLabel;
    GraphContract graph;
    bool shouldQuitAfterProofDump = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
}
