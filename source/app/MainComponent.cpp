#include "MainComponent.h"

#include "A1AudioProofRunner.h"
#include "ActiveWorkService.h"
#include "AppPaths.h"
#include "C2StorageProofRunner.h"
#include "C3SaveWorkProofRunner.h"
#include "C4AIWorkerSaveWorkProofRunner.h"
#include "C6AIRepairLoopProofRunner.h"
#include "C6AnalyzerFamilyProofRunner.h"
#include "C5ModulePublishProofRunner.h"
#include "CompoundModule.h"
#include "CompoundPatch.h"
#include "GraphEndpoint.h"
#include "GraphContract.h"
#include "InteractionContract.h"
#include "LiveIOProofRunner.h"
#include "PVB1AnalyzerEnvironmentProofRunner.h"
#include "PVDetectorProofRunner.h"
#include "ProofReports.h"
#include "RuntimeRegistry.h"

#include <string>
#include <utility>
#include <vector>

namespace myworld
{
namespace
{
LiveIOMidiOutputInventory availableMidiOutputInventory()
{
    LiveIOMidiOutputInventory inventory;

    for (const auto& device : juce::MidiOutput::getAvailableDevices())
        inventory.devices.push_back ({ device.name.toStdString(), device.identifier.toStdString() });

    return inventory;
}

juce::Font monoFont (float height)
{
    return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), height, juce::Font::plain));
}

void configureMeterLabel (juce::Label& label, juce::String text)
{
    label.setText (std::move (text), juce::dontSendNotification);
    label.setColour (juce::Label::textColourId, juce::Colour::fromRGB (202, 211, 226));
    label.setFont (monoFont (13.0f));
}

juce::Colour liveIOToneColour (const juce::String& tone)
{
    if (tone == "failed")
        return juce::Colour::fromRGB (244, 122, 122);

    if (tone == "sending")
        return juce::Colour::fromRGB (113, 218, 166);

    if (tone == "dry_run")
        return juce::Colour::fromRGB (157, 198, 218);

    if (tone == "inactive" || tone == "disabled")
        return juce::Colour::fromRGB (145, 153, 166);

    return juce::Colour::fromRGB (202, 211, 226);
}

std::vector<LiveIOBinding> makeAppLiveIOBindings()
{
    return {
        makeLiveIOMidiCcBinding ("midi.loudness", "out", 1, 20),
        makeLiveIOOscFloatBinding ("osc.loudness", "out", "/my-world/loudness"),
        makeLiveIOShaderUniformBinding ("uniform.loudness", "out", "u_loudness")
    };
}

}

MainComponent::MainComponent (StartupProofOptions startupProofOptions)
    : preferencesPanel (audioDeviceManager),
      graph (makeDefaultShaderOutputGraph()),
      performancePreferences (makeDefaultPerformancePreferences()),
      shouldQuitAfterStartupDump (startupProofOptions.quitAfterStartupDump)
{
    graphLabel.setText ("node canvas workspace",
                        juce::dontSendNotification);
    graphLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    graphLabel.setFont (monoFont (15.0f));
    addAndMakeVisible (graphLabel);

    statusLabel.setText ("waiting for GL context", juce::dontSendNotification);
    statusLabel.setColour (juce::Label::textColourId, juce::Colour::fromRGB (172, 184, 204));
    statusLabel.setFont (juce::Font (juce::FontOptions (13.0f)));
    addAndMakeVisible (statusLabel);

    audioStatusLabel.setText ("audio input starting", juce::dontSendNotification);
    audioStatusLabel.setColour (juce::Label::textColourId, juce::Colour::fromRGB (157, 198, 218));
    audioStatusLabel.setFont (monoFont (13.0f));
    addAndMakeVisible (audioStatusLabel);

    configureMeterLabel (rmsLabel, "rms 0.0000");
    configureMeterLabel (peakLabel, "peak 0.0000");
    configureMeterLabel (loudnessLabel, "loudness 0.0000");
    configureMeterLabel (activeLabel, "active no");
    configureMeterLabel (midiStatusLabel, "midi off");
    configureMeterLabel (liveIOStatusLabel, "live io dry idle m0 o0");
    addAndMakeVisible (rmsLabel);
    addAndMakeVisible (peakLabel);
    addAndMakeVisible (loudnessLabel);
    addAndMakeVisible (activeLabel);
    addAndMakeVisible (midiStatusLabel);
    addAndMakeVisible (liveIOStatusLabel);

    preferencesPanel.onAnalysisGainChanged = [this] (float gain)
    {
        performancePreferences.audio.analysisGain = gain;
        performancePreferences = sanitizePerformancePreferences (performancePreferences);
        audioInputAnalyzer.setAnalysisGain (performancePreferences.audio.analysisGain);
    };
    preferencesPanel.onMidiPreferencesChanged = [this] (MidiPreferences preferences)
    {
        applyMidiPreferences (std::move (preferences));
    };
    addChildComponent (preferencesPanel);

    dumpProofButton.setButtonText ("Dump Proof");
    dumpProofButton.onClick = [this] { dumpProof(); };
    addAndMakeVisible (dumpProofButton);

    shaderEditor.setMultiLine (true);
    shaderEditor.setReturnKeyStartsNewLine (true);
    shaderEditor.setTabKeyUsedAsCharacter (true);
    shaderEditor.setScrollbarsShown (true);
    shaderEditor.setFont (monoFont (14.0f));
    shaderEditor.setText (defaultFragmentShader(), juce::dontSendNotification);
    shaderEditor.onTextChange = [this]
    {
        preview.setFragmentShader (shaderEditor.getText().toStdString());
    };
    addChildComponent (shaderEditor);

    preview.onStatusMessage = [safe = juce::Component::SafePointer<MainComponent> (this)] (juce::String incomingStatus)
    {
        juce::MessageManager::callAsync ([safe, statusMessage = std::move (incomingStatus)]
        {
            if (safe != nullptr)
                safe->setShaderStatus (statusMessage);
        });
    };
    preview.onSaveWorkRequested = [safe = juce::Component::SafePointer<MainComponent> (this)] (GraphSession& session)
    {
        if (safe == nullptr)
            return CommandResult { false, "main component is gone" };

        return safe->saveActiveWork (session);
    };
    preview.onPublishModuleRequested = [safe = juce::Component::SafePointer<MainComponent> (this)] (GraphSession& session,
                                                                                                   const std::string& sourceNodeId)
    {
        if (safe == nullptr)
            return CommandResult { false, "main component is gone" };

        return safe->publishSelectedModule (session, sourceNodeId);
    };
    addAndMakeVisible (preview);

    startAudioInput();
    audioInputAnalyzer.setAnalysisGain (preferencesPanel.getAnalysisGain());
    applyMidiPreferences (preferencesPanel.getMidiPreferences());
    startTimerHz (30);

    scheduleStartupProofs (startupProofOptions);

    setSize (1440, 860);
}

MainComponent::~MainComponent()
{
    stopTimer();
    audioDeviceManager.removeAudioCallback (&audioInputAnalyzer);
    midiOutput.reset();
    preview.onStatusMessage = nullptr;
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB (13, 15, 20));
}

void MainComponent::scheduleStartupProofs (const StartupProofOptions& options)
{
    for (const auto& task : startupProofTasks (options))
    {
        juce::Timer::callAfterDelay (task.delayMilliseconds,
                                     [safe = juce::Component::SafePointer<MainComponent> (this),
                                      taskId = task.id]
        {
            if (safe != nullptr)
                safe->runStartupProofTask (taskId);
        });
    }
}

void MainComponent::runStartupProofTask (StartupProofTaskId task)
{
    switch (task)
    {
        case StartupProofTaskId::v1Shader:
            dumpProof();
            break;
        case StartupProofTaskId::a1Audio:
            dumpAudioProof();
            break;
        case StartupProofTaskId::liveIO:
            dumpLiveIOProof();
            break;
        case StartupProofTaskId::c2Storage:
            dumpC2StorageProof();
            break;
        case StartupProofTaskId::c3SaveWork:
            dumpC3SaveWorkProof();
            break;
        case StartupProofTaskId::c4AIWorkerSaveWork:
            dumpC4AIWorkerSaveWorkProof();
            break;
        case StartupProofTaskId::c5ModulePublish:
            dumpC5ModulePublishProof();
            break;
        case StartupProofTaskId::c5AIWorkerModulePublish:
            dumpC5AIWorkerModulePublishProof();
            break;
        case StartupProofTaskId::c5VisibleModulePublish:
            dumpC5VisibleModulePublishProof();
            break;
        case StartupProofTaskId::c6AnalyzerFamily:
            dumpC6AnalyzerFamilyProof();
            break;
        case StartupProofTaskId::c6AIRepairLoop:
            dumpC6AIRepairLoopProof();
            break;
        case StartupProofTaskId::pvAttackDetector:
            dumpPVAttackDetectorProof();
            break;
        case StartupProofTaskId::pvDensityDetector:
            dumpPVDensityDetectorProof();
            break;
        case StartupProofTaskId::pvSilenceDetector:
            dumpPVSilenceDetectorProof();
            break;
        case StartupProofTaskId::pvSustainDetector:
            dumpPVSustainDetectorProof();
            break;
        case StartupProofTaskId::pvResidueDetector:
            dumpPVResidueDetectorProof();
            break;
        case StartupProofTaskId::pvAggregatePressure:
            dumpPVAggregatePressureProof();
            break;
        case StartupProofTaskId::pvB1AnalyzerEnvironment:
            dumpPVB1AnalyzerEnvironmentProof();
            break;
    }
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced (12);
    auto header = area.removeFromTop (28);

    graphLabel.setBounds (header.removeFromLeft (260));
    dumpProofButton.setBounds (header.removeFromRight (112));
    header.removeFromRight (10);
    statusLabel.setBounds (header);

    area.removeFromTop (8);

    auto audioRow = area.removeFromTop (24);
    audioStatusLabel.setBounds (audioRow.removeFromLeft (270));
    rmsLabel.setBounds (audioRow.removeFromLeft (118));
    peakLabel.setBounds (audioRow.removeFromLeft (118));
    loudnessLabel.setBounds (audioRow.removeFromLeft (160));
    activeLabel.setBounds (audioRow.removeFromLeft (104));
    midiStatusLabel.setBounds (audioRow.removeFromLeft (230));
    liveIOStatusLabel.setBounds (audioRow);

    area.removeFromTop (10);

    preview.setBounds (area);
}

void MainComponent::dumpProof()
{
    const auto directory = proofDumpDirectory ("v1-shader-proof");
    statusLabel.setText ("proof dump requested: " + directory.getFullPathName(), juce::dontSendNotification);
    preview.requestProofDump (directory, graph);
}

void MainComponent::finishProofDump (const juce::String& displayName,
                                     const std::string& status,
                                     const std::string& error,
                                     const juce::File& directory)
{
    if (status == "failed")
    {
        statusLabel.setText (displayName + " proof failed: " + juce::String (error),
                             juce::dontSendNotification);
    }
    else
    {
        statusLabel.setText (displayName + " proof " + juce::String (status) + ": "
                                 + directory.getFullPathName(),
                             juce::dontSendNotification);
    }

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

void MainComponent::dumpAudioProof()
{
    const auto directory = proofDumpDirectory (a1AudioProofDirectoryName());

    A1AudioProofRunRequest request;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();
    request.snapshot = audioInputAnalyzer.getSnapshot();
    request.sampleRate = audioInputAnalyzer.getSampleRate();
    request.bufferSize = audioInputAnalyzer.getBufferSize();
    request.preferences = performancePreferences;

    const auto result = runA1AudioProof (request);
    finishProofDump (juce::String (a1AudioProofDisplayName()), result.status, result.error, directory);
}

void MainComponent::dumpLiveIOProof()
{
    const auto directory = proofDumpDirectory (liveIOProofDirectoryName());

    LiveIOProofRunRequest request;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();
    request.midiOutputInventory = availableMidiOutputInventory();
    request.midiOutputSender = [] (const LiveIOMidiOutputDevice& device,
                                   const LiveIOMidiCcMessage& message)
    {
        auto output = juce::MidiOutput::openDevice (juce::String (device.identifier));
        if (output == nullptr)
            return LiveIOMidiOutputDeviceSendResult {
                false,
                false,
                "midi output open failed: " + device.identifier
            };

        output->sendMessageNow (juce::MidiMessage::controllerEvent (
            message.channel,
            message.cc,
            message.value));
        return LiveIOMidiOutputDeviceSendResult { true, true, "" };
    };
    request.controlOscSender = [] (const LiveIOOscFloatMessage& message)
    {
        if (message.oscAddress.empty())
            return LiveIOOscFloatSendResult { false, "osc address is required: " + message.bindingId };

        return LiveIOOscFloatSendResult { true, "" };
    };
    request.loudness = 0.5f;

    const auto result = runLiveIOProof (request);
    finishProofDump (juce::String (liveIOProofDisplayName()), result.status, result.error, directory);
}

void MainComponent::dumpC2StorageProof()
{
    const auto directory = proofDumpDirectory (c2StorageProofDirectoryName());

    C2StorageProofRunRequest request;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runC2StorageProof (request);
    finishProofDump (juce::String (c2StorageProofDisplayName()), result.status, result.error, directory);
}

void MainComponent::dumpC3SaveWorkProof()
{
    const auto directory = proofDumpDirectory (c3SaveWorkProofDirectoryName());

    C3SaveWorkProofRunRequest request;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runC3SaveWorkProof (request);
    finishProofDump (juce::String (c3SaveWorkProofDisplayName()), result.status, result.error, directory);
}

void MainComponent::dumpC4AIWorkerSaveWorkProof()
{
    const auto directory = proofDumpDirectory (c4AIWorkerSaveWorkProofDirectoryName());

    C4AIWorkerSaveWorkProofRunRequest request;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runC4AIWorkerSaveWorkProof (request);
    finishProofDump (juce::String (c4AIWorkerSaveWorkProofDisplayName()), result.status, result.error, directory);
}

void MainComponent::dumpC5ModulePublishProof()
{
    const auto directory = proofDumpDirectory (
        c5ModulePublishProofDirectoryName (C5ModulePublishProofKind::modulePublish));

    C5ModulePublishProofRunRequest request;
    request.kind = C5ModulePublishProofKind::modulePublish;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runC5ModulePublishProof (request);
    finishProofDump (juce::String (c5ModulePublishProofDisplayName (request.kind)),
                     result.status,
                     result.error,
                     directory);
}

void MainComponent::dumpC5AIWorkerModulePublishProof()
{
    const auto directory = proofDumpDirectory (
        c5ModulePublishProofDirectoryName (C5ModulePublishProofKind::aiWorkerModulePublish));

    C5ModulePublishProofRunRequest request;
    request.kind = C5ModulePublishProofKind::aiWorkerModulePublish;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runC5ModulePublishProof (request);
    finishProofDump (juce::String (c5ModulePublishProofDisplayName (request.kind)),
                     result.status,
                     result.error,
                     directory);
}

void MainComponent::dumpC5VisibleModulePublishProof()
{
    const auto proofDirectory = proofDumpDirectory (
        c5ModulePublishProofDirectoryName (C5ModulePublishProofKind::visibleModulePublish));

    C5ModulePublishProofRunRequest request;
    request.kind = C5ModulePublishProofKind::visibleModulePublish;
    request.outputDirectory = proofDirectory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runC5ModulePublishProof (request);
    finishProofDump (juce::String (c5ModulePublishProofDisplayName (request.kind)),
                     result.status,
                     result.error,
                     proofDirectory);
}

void MainComponent::dumpC6AnalyzerFamilyProof()
{
    const auto directory = proofDumpDirectory (c6AnalyzerFamilyProofDirectoryName());

    C6AnalyzerFamilyProofRunRequest request;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runC6AnalyzerFamilyProof (request);
    finishProofDump (juce::String (c6AnalyzerFamilyProofDisplayName()), result.status, result.error, directory);
}

void MainComponent::dumpC6AIRepairLoopProof()
{
    const auto directory = proofDumpDirectory (c6AIRepairLoopProofDirectoryName());

    C6AIRepairLoopProofRunRequest request;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runC6AIRepairLoopProof (request);
    finishProofDump (juce::String (c6AIRepairLoopProofDisplayName()), result.status, result.error, directory);
}

void MainComponent::dumpPVAttackDetectorProof()
{
    dumpPVDetectorProof (PVDetectorProofKind::attack);
}

void MainComponent::dumpPVDensityDetectorProof()
{
    dumpPVDetectorProof (PVDetectorProofKind::density);
}

void MainComponent::dumpPVSilenceDetectorProof()
{
    dumpPVDetectorProof (PVDetectorProofKind::silence);
}

void MainComponent::dumpPVSustainDetectorProof()
{
    dumpPVDetectorProof (PVDetectorProofKind::sustain);
}

void MainComponent::dumpPVResidueDetectorProof()
{
    dumpPVDetectorProof (PVDetectorProofKind::residue);
}

void MainComponent::dumpPVAggregatePressureProof()
{
    dumpPVDetectorProof (PVDetectorProofKind::aggregatePressure);
}

void MainComponent::dumpPVDetectorProof (PVDetectorProofKind kind)
{
    const auto directory = proofDumpDirectory (pvDetectorProofDirectoryName (kind));

    PVDetectorProofRunRequest request;
    request.kind = kind;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runPVDetectorProof (request);
    finishProofDump (juce::String (pvDetectorProofDisplayName (kind)), result.status, result.error, directory);
}

void MainComponent::dumpPVB1AnalyzerEnvironmentProof()
{
    const auto directory = proofDumpDirectory (pvB1AnalyzerEnvironmentProofDirectoryName());

    PVB1AnalyzerEnvironmentProofRunRequest request;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runPVB1AnalyzerEnvironmentProof (request);
    finishProofDump (juce::String (pvB1AnalyzerEnvironmentProofDisplayName()), result.status, result.error, directory);
}

CommandResult MainComponent::saveActiveWork (GraphSession& session)
{
    const auto result = saveActiveWorkProject (session);
    statusLabel.setText ((result.ok ? "save_work: " : "save_work failed: ") + juce::String (result.message),
                         juce::dontSendNotification);

    return result;
}

CommandResult MainComponent::publishSelectedModule (GraphSession& session, const std::string& sourceNodeId)
{
    const auto result = publishSelectedModuleFromActiveWork (session, sourceNodeId);
    const auto message = result.ok ? result.status : result.error;
    statusLabel.setText ((result.ok ? "publish_module: " : "publish_module failed: ") + juce::String (message),
                         juce::dontSendNotification);

    return { result.ok, message };
}

void MainComponent::timerCallback()
{
    updateAudioMeters();
}

void MainComponent::quitAfterDelay()
{
    juce::Timer::callAfterDelay (250, []
    {
        if (auto* app = juce::JUCEApplicationBase::getInstance())
            app->systemRequestedQuit();
    });
}

void MainComponent::setShaderStatus (juce::String message)
{
    const auto shouldQuit = shouldQuitAfterStartupDump && message.startsWith ("proof dumped:");
    statusLabel.setText (std::move (message), juce::dontSendNotification);

    if (shouldQuit)
        quitAfterDelay();
}

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

void MainComponent::updateAudioMeters()
{
    const auto bridge = makeLoudnessRuntimeBridgeSnapshot (RuntimeExecutionSnapshot{}, audioInputAnalyzer.getSnapshot());
    const auto& snapshot = bridge.analyzer;

    rmsLabel.setText ("rms " + juce::String (snapshot.rms, 4), juce::dontSendNotification);
    peakLabel.setText ("peak " + juce::String (snapshot.peak, 4), juce::dontSendNotification);
    loudnessLabel.setText ("loudness " + juce::String (snapshot.loudness, 4), juce::dontSendNotification);
    activeLabel.setText (juce::String ("active ") + (snapshot.active ? "yes" : "no"), juce::dontSendNotification);
    preview.setLoudness (snapshot.loudness);
    sendMidiForSnapshot (snapshot);
    tickLiveIOControl (snapshot);
    midiStatusLabel.setText (midiStatus, juce::dontSendNotification);
    liveIOStatusLabel.setText (liveIOStatus, juce::dontSendNotification);
    liveIOStatusLabel.setColour (juce::Label::textColourId, liveIOToneColour (liveIOStatusTone));

    const auto sampleRate = audioInputAnalyzer.getSampleRate();

    if (sampleRate > 0.0)
    {
        audioStatusLabel.setText ("audio input "
                                      + juce::String (sampleRate, 0)
                                      + "Hz / "
                                      + juce::String (audioInputAnalyzer.getBufferSize())
                                      + " samples",
                                  juce::dontSendNotification);
    }
}

void MainComponent::applyMidiPreferences (MidiPreferences preferences)
{
    performancePreferences.midi = std::move (preferences);
    performancePreferences = sanitizePerformancePreferences (performancePreferences);

    const auto identifier = juce::String (performancePreferences.midi.outputIdentifier);

    if (identifier == openedMidiOutputIdentifier)
        return;

    midiOutput.reset();
    openedMidiOutputIdentifier = identifier;

    if (identifier.isEmpty())
    {
        midiStatus = "midi off";
        return;
    }

    midiOutput = juce::MidiOutput::openDevice (identifier);

    if (midiOutput == nullptr)
    {
        midiStatus = "midi open failed";
        return;
    }

    midiStatus = "midi " + juce::String (performancePreferences.midi.outputName);
}

void MainComponent::sendMidiForSnapshot (const AudioAnalyzerSnapshot& snapshot)
{
    const auto frame = makeLoudnessMidiCcFrame (snapshot.loudness, performancePreferences);

    if (! frame.shouldSend)
        return;

    if (midiOutput == nullptr)
    {
        midiStatus = "midi no output";
        return;
    }

    midiOutput->sendMessageNow (juce::MidiMessage::controllerEvent (frame.channel, frame.cc, frame.value));
    midiStatus = "midi ch" + juce::String (frame.channel)
                 + " cc" + juce::String (frame.cc)
                 + " " + juce::String (frame.value);
}

void MainComponent::tickLiveIOControl (const AudioAnalyzerSnapshot& snapshot)
{
    LiveIOControlTimerConfig config;
    config.bindings = makeAppLiveIOBindings();
    config.tickIntervalMs = 50;
    config.dispatchMinIntervalMs = 0;
    config.enabled = true;
    config.sendMode = liveIOSendMode;

    const auto nowMs = static_cast<std::int64_t> (juce::Time::getMillisecondCounterHiRes());
    (void) tickLiveIOControlTimer (liveIOTimerState, config, nowMs, snapshot);

    const auto indicator = makeLiveIOStatusIndicatorState (liveIOTimerState, liveIOSendMode);
    liveIOStatus = juce::String (indicator.text);
    liveIOStatusTone = juce::String (indicator.tone);
}
}
