#include "LiveIOAppController.h"

namespace myworld
{
namespace
{
LiveIOControlTimerSendMode sendModeFromPreference (LiveIOSendModePreference sendMode)
{
    if (sendMode == LiveIOSendModePreference::controlledSend)
        return LiveIOControlTimerSendMode::controlledSend;

    return LiveIOControlTimerSendMode::dryRun;
}

LiveIOMidiOutputInventory selectedMidiOutputInventory (const MidiPreferences& midiPreferences)
{
    LiveIOMidiOutputInventory inventory;

    if (! midiPreferences.outputIdentifier.empty())
        inventory.devices.push_back ({ midiPreferences.outputName, midiPreferences.outputIdentifier });

    return inventory;
}

std::vector<LiveIOBinding> makeAppLiveIOBindings (const MidiPreferences& midiPreferences,
                                                  const LiveIOPreferences& liveIOPreferences)
{
    return {
        makeLiveIOMidiCcBinding ("midi.loudness",
                                 "out",
                                 midiPreferences.channel,
                                 midiPreferences.loudnessCc),
        makeLiveIOOscFloatBinding ("osc.loudness", "out", liveIOPreferences.oscLoudnessAddress),
        makeLiveIOShaderUniformBinding ("uniform.loudness", "out", "u_loudness")
    };
}

std::string midiTeachStatusText (const LiveIOMidiTeachState& state, int inputCount)
{
    if (state.status == "armed")
    {
        auto text = "teach " + liveIOMidiTeachTargetToString (state.target) + " waiting";

        if (inputCount == 0)
            text += " / no inputs";

        return text;
    }

    if (state.status == "learned")
    {
        return "learned ch" + std::to_string (state.learnedChannel)
             + " cc" + std::to_string (state.learnedCc);
    }

    if (state.status == "ignored")
        return "teach waiting for CC";

    if (state.status == "cancelled")
        return "teach cancelled";

    if (state.status == "failed")
        return "teach failed";

    return "teach idle";
}

LiveIOAppMidiTeachView makeMidiTeachView (const LiveIOMidiTeachState& state,
                                          const LiveIOMidiTeachResult& result,
                                          int inputCount)
{
    LiveIOAppMidiTeachView view;
    view.ok = result.ok;
    view.shouldListen = state.armed;
    view.learned = result.learned;
    view.learnedTarget = result.target;
    view.learnedChannel = result.channel;
    view.learnedCc = result.cc;
    view.statusText = midiTeachStatusText (state, inputCount);
    return view;
}
}

void LiveIOAppController::applyLiveIOPreferences (LiveIOPreferences preferences)
{
    PerformancePreferences allPreferences;
    allPreferences.liveIO = preferences;
    allPreferences = sanitizePerformancePreferences (allPreferences);
    liveIOSendMode = sendModeFromPreference (allPreferences.liveIO.sendMode);
}

LiveIOControlTimerSendMode LiveIOAppController::getSendMode() const
{
    return liveIOSendMode;
}

LiveIOAppTimerResult LiveIOAppController::tick (const LiveIOAppTimerRequest& request)
{
    const auto preferences = sanitizePerformancePreferences (request.preferences);

    LiveIOControlTimerConfig config;
    config.bindings = makeAppLiveIOBindings (preferences.midi, preferences.liveIO);
    config.tickIntervalMs = 50;
    config.dispatchMinIntervalMs = 0;
    config.enabled = true;
    config.sendMode = liveIOSendMode;

    if (liveIOSendMode == LiveIOControlTimerSendMode::controlledSend)
    {
        config.midiOutputInventory = selectedMidiOutputInventory (preferences.midi);
        config.midiOutputIdentifier = preferences.midi.outputIdentifier;
        config.oscEnabled = false;
        config.midiSender = request.midiSender;
    }

    LiveIOAppTimerResult result;
    result.tick = tickLiveIOControlTimer (
        timerState,
        config,
        request.timestampMs,
        request.snapshot);
    result.indicator = makeLiveIOStatusIndicatorState (timerState, liveIOSendMode);
    return result;
}

LiveIOAppMidiTeachView LiveIOAppController::armMidiTeach (LiveIOMidiTeachTarget target, int inputCount)
{
    midiTeachInputCount = inputCount;
    const auto result = armLiveIOMidiTeach (midiTeachState, target);
    return makeMidiTeachView (midiTeachState, result, midiTeachInputCount);
}

LiveIOAppMidiTeachView LiveIOAppController::cancelMidiTeach()
{
    midiTeachInputCount = 0;
    const auto result = cancelLiveIOMidiTeach (midiTeachState);
    return makeMidiTeachView (midiTeachState, result, midiTeachInputCount);
}

LiveIOAppMidiTeachView LiveIOAppController::handleMidiTeachMessage (
    const LiveIOMidiTeachIncomingMessage& message)
{
    const auto result = handleLiveIOMidiTeachMessage (midiTeachState, message);

    if (result.learned)
        midiTeachInputCount = 0;

    return makeMidiTeachView (midiTeachState, result, midiTeachInputCount);
}
}
