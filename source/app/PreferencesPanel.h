#pragma once

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

    std::function<void(float)> onAnalysisGainChanged;
    std::function<void(MidiPreferences)> onMidiPreferencesChanged;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void refreshMidiOutputs();
    void emitAnalysisGain();
    void emitMidiPreferences();
    void configureLabel (juce::Label& label, const juce::String& text);

    juce::AudioDeviceSelectorComponent audioSelector;
    juce::Label titleLabel;
    juce::Label audioLabel;
    juce::Label midiLabel;
    juce::Label gainLabel;
    juce::Label midiOutputLabel;
    juce::Label midiChannelLabel;
    juce::Label loudnessCcLabel;
    juce::Label mapCcLabel;
    juce::Slider analysisGainSlider;
    juce::ComboBox midiOutputBox;
    juce::Slider midiChannelSlider;
    juce::Slider loudnessCcSlider;
    juce::Slider mapCcSlider;
    juce::ToggleButton midiStreamButton;
    juce::ToggleButton mapModeButton;
    juce::TextButton refreshMidiButton;
    std::vector<juce::MidiDeviceInfo> midiOutputs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PreferencesPanel)
};
}
