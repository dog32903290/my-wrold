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
    if (liveIOPreferences.outputOperator == LiveIOOutputOperatorPreference::midiNoteOn)
        return { makeLiveIOMidiNoteOnBinding ("midi.note.loudness", "out", midiPreferences.channel, 60) };

    if (liveIOPreferences.outputOperator == LiveIOOutputOperatorPreference::oscFloat)
        return { makeLiveIOOscFloatBinding ("osc.loudness", "out", liveIOPreferences.oscLoudnessAddress) };

    if (liveIOPreferences.outputOperator == LiveIOOutputOperatorPreference::shaderUniform)
        return { makeLiveIOShaderUniformBinding ("uniform.loudness", "out", "u_loudness") };

    return { makeLiveIOMidiCcBinding ("midi.loudness",
                                      "out",
                                      midiPreferences.channel,
                                      midiPreferences.loudnessCc) };
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
    const auto nextReceiverConfig = makeOscReceiverConfig (allPreferences.liveIO);

    if (oscReceiver.isOpen()
        && (oscReceiver.config().host != nextReceiverConfig.host
            || oscReceiver.config().port != nextReceiverConfig.port
            || oscReceiver.config().address != nextReceiverConfig.address
            || oscReceiver.config().valueId != nextReceiverConfig.valueId))
    {
        oscReceiver.close();
    }

    liveIOSendMode = sendModeFromPreference (allPreferences.liveIO.sendMode);
}

LiveIOControlTimerSendMode LiveIOAppController::getSendMode() const
{
    return liveIOSendMode;
}

LiveIOOscReceiverConfig LiveIOAppController::makeOscReceiverConfig (
    const LiveIOPreferences& preferences) const
{
    LiveIOOscReceiverConfig config;
    config.host = preferences.oscHost;
    config.port = preferences.oscPort;
    config.address = preferences.oscLoudnessAddress;
    config.valueId = "osc.loudness";
    return config;
}

LiveIOOscReceiverOpenResult LiveIOAppController::ensureOscReceiverOpen (
    const LiveIOOscReceiverConfig& config)
{
    if (oscReceiver.isOpen()
        && oscReceiver.config().host == config.host
        && oscReceiver.config().port == config.port
        && oscReceiver.config().address == config.address
        && oscReceiver.config().valueId == config.valueId)
    {
        return { true, "open", "osc_receiver_open", oscReceiver.boundPort() };
    }

    return oscReceiver.open (config);
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
        config.oscEnabled = request.oscSender != nullptr;
        config.oscHost = preferences.liveIO.oscHost;
        config.oscPort = preferences.liveIO.oscPort;
        config.midiSender = request.midiSender;
        config.oscSender = request.oscSender;
    }

    LiveIOAppTimerResult result;
    const auto receiverConfig = makeOscReceiverConfig (preferences.liveIO);
    const auto receiverOpen = ensureOscReceiverOpen (receiverConfig);
    result.oscReceiverOpen = receiverOpen.ok;
    result.oscReceiverPort = receiverOpen.port;
    result.oscReceiverStatus = receiverOpen.status;
    result.oscReceiverMessage = receiverOpen.message;

    if (receiverOpen.ok)
    {
        const auto poll = oscReceiver.pollOnce (0);
        result.oscReceived = poll.received;
        result.oscReceiverStatus = poll.status;
        result.oscReceiverMessage = poll.message;
        result.oscFrame = makeLiveIOValueFrameFromOscReceive (oscReceiver.config(), poll.packet);
    }

    result.tick = tickLiveIOControlTimer (
        timerState,
        config,
        request.timestampMs,
        request.snapshot);
    result.indicator = withLiveIOOutputOperator (
        makeLiveIOStatusIndicatorState (timerState, liveIOSendMode),
        liveIOOutputOperatorPreferenceToString (preferences.liveIO.outputOperator));
    if (timerState.hasLastShaderUniform)
    {
        result.indicator = withLiveIOShaderUniformEvidence (
            result.indicator,
            timerState.lastShaderUniformBindingId,
            timerState.lastShaderUniformName,
            timerState.lastShaderUniformValue,
            timerState.lastShaderUniformSampleCounter);
    }
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
