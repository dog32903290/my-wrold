#pragma once

#include "LiveIOMidiTeach.h"
#include "PerformancePreferences.h"

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include <functional>
#include <vector>

namespace myworld
{
class PreferencesPanel final : public juce::Component
{
public:
    explicit PreferencesPanel (juce::AudioDeviceManager& deviceManager);

    float getAnalysisGain() const;
    MidiPreferences getMidiPreferences() const;
    LiveIOPreferences getLiveIOPreferences() const;
    void applyPerformancePreferences (const PerformancePreferences& preferences);
    void applyLearnedMidiCc (LiveIOMidiTeachTarget target, int channel, int cc);
    void setMidiTeachStatus (juce::String text);

    std::function<void(float)> onAnalysisGainChanged;
    std::function<void(MidiPreferences)> onMidiPreferencesChanged;
    std::function<void(LiveIOPreferences)> onLiveIOPreferencesChanged;
    std::function<void()> onLearnLoudnessCcRequested;
    std::function<void()> onLearnMapCcRequested;
    std::function<void()> onMidiTeachCancelRequested;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void refreshMidiInputs();
    void refreshMidiOutputs();
    void emitAnalysisGain();
    void emitMidiPreferences();
    void emitLiveIOPreferences();
    void configureLabel (juce::Label& label, const juce::String& text);

    juce::AudioDeviceSelectorComponent audioSelector;
    juce::Label titleLabel;
    juce::Label audioLabel;
    juce::Label midiLabel;
    juce::Label gainLabel;
    juce::Label midiInputLabel;
    juce::Label midiOutputLabel;
    juce::Label midiChannelLabel;
    juce::Label loudnessCcLabel;
    juce::Label mapCcLabel;
    juce::Label liveIOLabel;
    juce::Label liveIOSendModeLabel;
    juce::Label liveIOOperatorLabel;
    juce::Label midiTeachLabel;
    juce::Label midiTeachStatusLabel;
    juce::Slider analysisGainSlider;
    juce::ComboBox midiInputBox;
    juce::ComboBox midiOutputBox;
    juce::ComboBox liveIOSendModeBox;
    juce::ComboBox liveIOOperatorBox;
    juce::Slider midiChannelSlider;
    juce::Slider loudnessCcSlider;
    juce::Slider mapCcSlider;
    juce::ToggleButton midiStreamButton;
    juce::ToggleButton mapModeButton;
    juce::TextButton refreshMidiButton;
    juce::TextButton learnLoudnessCcButton;
    juce::TextButton learnMapCcButton;
    juce::TextButton cancelMidiTeachButton;
    std::vector<juce::MidiDeviceInfo> midiInputs;
    std::vector<juce::MidiDeviceInfo> midiOutputs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PreferencesPanel)
};
}
