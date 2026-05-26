#include "MainComponent.h"

#include "A1AudioProofRunner.h"
#include "ActiveWorkService.h"
#include "APP1WorkbenchSessionProofRunner.h"
#include "AppSaveProofRunner.h"
#include "AppStatusProofRunner.h"
#include "AppWorkbenchSessionProofRunner.h"
#include "AppPaths.h"
#include "C2StorageProofRunner.h"
#include "C3SaveWorkProofRunner.h"
#include "C4AIWorkerSaveWorkProofRunner.h"
#include "C6AIRepairLoopProofRunner.h"
#include "C6AnalyzerFamilyProofRunner.h"
#include "C5ModulePublishProofRunner.h"
#include "CompoundModule.h"
#include "CompoundPatch.h"
#include "CreatedProjectOpenProofRunner.h"
#include "GraphEndpoint.h"
#include "GraphContract.h"
#include "InteractionContract.h"
#include "LiveIOProofRunner.h"
#include "PVB1AnalyzerEnvironmentProofRunner.h"
#include "PVDetectorProofRunner.h"
#include "ProofReports.h"
#include "ProjectCreationProofRunner.h"
#include "RuntimeRegistry.h"
#include "ShaderPreviewInputBridge.h"
#include "WorkbenchCanvasSurfaceProofRunner.h"
#include "WorkbenchGraphSurfaceProofRunner.h"
#include "WorkbenchRuntimeSurfaceProofRunner.h"
#include "WorkbenchStatusSurfaceProofRunner.h"

#include <algorithm>
#include <filesystem>
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

juce::Colour workbenchStatusToneColour (const std::string& tone)
{
    if (tone == "blocked")
        return juce::Colour::fromRGB (244, 122, 122);

    if (tone == "dirty")
        return juce::Colour::fromRGB (238, 202, 118);

    if (tone == "ready")
        return juce::Colour::fromRGB (157, 198, 218);

    return juce::Colour::fromRGB (202, 211, 226);
}

LiveIORealtimeIndicatorTelemetry makeRealtimeIndicatorTelemetry (
    const AudioRealtimeDeliveryResult& realtime)
{
    LiveIORealtimeIndicatorTelemetry telemetry;
    telemetry.status = audioRealtimeDeliveryStatusToString (realtime.status);
    telemetry.sequence = realtime.sequence;
    telemetry.droppedSnapshots = realtime.droppedSnapshots;
    telemetry.skippedSnapshots = realtime.skippedSnapshots;
    telemetry.overwrittenSnapshots = realtime.overwrittenSnapshots;
    return telemetry;
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

    for (auto& label : workbenchStatusLabels)
    {
        configureMeterLabel (label, "");
        addAndMakeVisible (label);
    }

    for (auto& label : workbenchGraphLabels)
    {
        configureMeterLabel (label, "");
        addAndMakeVisible (label);
    }

    for (auto& label : workbenchCanvasLabels)
    {
        configureMeterLabel (label, "");
        addAndMakeVisible (label);
    }

    for (auto& label : workbenchRuntimeLabels)
    {
        configureMeterLabel (label, "");
        addAndMakeVisible (label);
    }

    for (auto& label : workbenchCookPlanLabels)
    {
        configureMeterLabel (label, "");
        addAndMakeVisible (label);
    }

    openWorkbenchSession();

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
        saveStoredPerformancePreferences();
    };
    preferencesPanel.onMidiPreferencesChanged = [this] (MidiPreferences preferences)
    {
        applyMidiPreferences (std::move (preferences));
    };
    preferencesPanel.onLiveIOPreferencesChanged = [this] (LiveIOPreferences preferences)
    {
        applyLiveIOPreferences (preferences);
    };
    preferencesPanel.onLearnLoudnessCcRequested = [this]
    {
        armMidiTeach (LiveIOMidiTeachTarget::loudnessCc);
    };
    preferencesPanel.onLearnMapCcRequested = [this]
    {
        armMidiTeach (LiveIOMidiTeachTarget::mapCc);
    };
    preferencesPanel.onMidiTeachCancelRequested = [this]
    {
        cancelMidiTeach();
    };
    addChildComponent (preferencesPanel);

    loadStoredPerformancePreferences();

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
    audioInputAnalyzer.setAnalysisGain (performancePreferences.audio.analysisGain);
    applyMidiPreferences (performancePreferences.midi);
    applyLiveIOPreferences (performancePreferences.liveIO);
    startTimerHz (30);

    scheduleStartupProofs (startupProofOptions);

    setSize (1440, 860);
}

MainComponent::~MainComponent()
{
    stopTimer();
    stopMidiTeachListening();
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
        case StartupProofTaskId::app1WorkbenchSession:
            dumpAPP1WorkbenchSessionProof();
            break;
        case StartupProofTaskId::app2WorkbenchOpenStatus:
            dumpAPPWorkbenchSessionProof();
            break;
        case StartupProofTaskId::appWorkbenchSession:
            dumpAPPWorkbenchSessionProof();
            break;
        case StartupProofTaskId::projectCreation:
            dumpProjectCreationProof();
            break;
        case StartupProofTaskId::createdProjectOpen:
            dumpCreatedProjectOpenProof();
            break;
        case StartupProofTaskId::appSave:
            dumpAppSaveProof();
            break;
        case StartupProofTaskId::appStatus:
            dumpAppStatusProof();
            break;
        case StartupProofTaskId::workbenchStatusSurface:
            dumpWorkbenchStatusSurfaceProof();
            break;
        case StartupProofTaskId::workbenchGraphSurface:
            dumpWorkbenchGraphSurfaceProof();
            break;
        case StartupProofTaskId::workbenchCanvasSurface:
            dumpWorkbenchCanvasSurfaceProof();
            break;
        case StartupProofTaskId::workbenchRuntimeSurface:
            dumpWorkbenchRuntimeSurfaceProof();
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

    auto workbenchRow = area.removeFromTop (24);
    workbenchStatusLabels[0].setBounds (workbenchRow.removeFromLeft (230));
    workbenchStatusLabels[1].setBounds (workbenchRow.removeFromLeft (250));
    workbenchStatusLabels[2].setBounds (workbenchRow.removeFromLeft (250));
    workbenchStatusLabels[3].setBounds (workbenchRow.removeFromLeft (150));
    workbenchStatusLabels[4].setBounds (workbenchRow.removeFromLeft (170));
    workbenchStatusLabels[5].setBounds (workbenchRow);

    area.removeFromTop (8);

    auto graphRow = area.removeFromTop (24);
    workbenchGraphLabels[0].setBounds (graphRow.removeFromLeft (230));
    workbenchGraphLabels[1].setBounds (graphRow.removeFromLeft (170));
    workbenchGraphLabels[2].setBounds (graphRow.removeFromLeft (180));
    workbenchGraphLabels[3].setBounds (graphRow.removeFromLeft (150));
    workbenchGraphLabels[4].setBounds (graphRow.removeFromLeft (260));
    workbenchGraphLabels[5].setBounds (graphRow);

    area.removeFromTop (8);

    auto canvasRow = area.removeFromTop (24);
    workbenchCanvasLabels[0].setBounds (canvasRow.removeFromLeft (230));
    workbenchCanvasLabels[1].setBounds (canvasRow.removeFromLeft (90));
    workbenchCanvasLabels[2].setBounds (canvasRow.removeFromLeft (90));
    workbenchCanvasLabels[3].setBounds (canvasRow.removeFromLeft (260));
    workbenchCanvasLabels[4].setBounds (canvasRow.removeFromLeft (310));
    workbenchCanvasLabels[5].setBounds (canvasRow);

    area.removeFromTop (8);

    auto runtimeRow = area.removeFromTop (24);
    workbenchRuntimeLabels[0].setBounds (runtimeRow.removeFromLeft (230));
    workbenchRuntimeLabels[1].setBounds (runtimeRow.removeFromLeft (170));
    workbenchRuntimeLabels[2].setBounds (runtimeRow.removeFromLeft (150));
    workbenchRuntimeLabels[3].setBounds (runtimeRow.removeFromLeft (150));
    workbenchRuntimeLabels[4].setBounds (runtimeRow.removeFromLeft (170));
    workbenchRuntimeLabels[5].setBounds (runtimeRow);

    area.removeFromTop (8);

    auto cookRow = area.removeFromTop (24);
    workbenchCookPlanLabels[0].setBounds (cookRow.removeFromLeft (230));
    workbenchCookPlanLabels[1].setBounds (cookRow.removeFromLeft (220));
    workbenchCookPlanLabels[2].setBounds (cookRow.removeFromLeft (140));
    workbenchCookPlanLabels[3].setBounds (cookRow.removeFromLeft (170));
    workbenchCookPlanLabels[4].setBounds (cookRow.removeFromLeft (170));
    workbenchCookPlanLabels[5].setBounds (cookRow);

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

void MainComponent::dumpAPP1WorkbenchSessionProof()
{
    const auto directory = proofDumpDirectory (app1WorkbenchSessionProofDirectoryName());

    APP1WorkbenchSessionProofRunRequest request;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runAPP1WorkbenchSessionProof (request);
    finishProofDump (juce::String (app1WorkbenchSessionProofDisplayName()), result.status, result.error, directory);
}

void MainComponent::dumpAPP2WorkbenchOpenStatusProof()
{
    dumpAPPWorkbenchSessionProof();
}

void MainComponent::dumpAPPWorkbenchSessionProof()
{
    const auto directory = proofDumpDirectory (appWorkbenchSessionProofDirectoryName());

    const auto request = workbenchController.makeOpenStatusProofRequest (
        directory.getFullPathName().toStdString());
    auto proofRequest = request;
    proofRequest.activeWorkPreparation = activeWorkPreparation;

    const auto result = runAppWorkbenchSessionProof (proofRequest);
    finishProofDump (juce::String (appWorkbenchSessionProofDisplayName()), result.status, result.error, directory);
}

void MainComponent::dumpProjectCreationProof()
{
    const auto directory = proofDumpDirectory (projectCreationProofDirectoryName());

    ProjectCreationProofRunRequest request;
    request.outputDirectory = directory.getFullPathName().toStdString();

    const auto result = runProjectCreationProof (request);
    if (! result.ok && ! result.statusText.empty())
    {
        finishProofDump (juce::String (projectCreationProofDisplayName()), "failed", result.statusText, directory);
        return;
    }

    const auto status = result.statusText.empty() ? result.status : result.statusText;
    finishProofDump (juce::String (projectCreationProofDisplayName()), status, result.error, directory);
}

void MainComponent::dumpCreatedProjectOpenProof()
{
    const auto directory = proofDumpDirectory (createdProjectOpenProofDirectoryName());

    CreatedProjectOpenProofRunRequest request;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runCreatedProjectOpenProof (request);
    if (! result.ok && ! result.statusText.empty())
    {
        finishProofDump (juce::String (createdProjectOpenProofDisplayName()), "failed", result.statusText, directory);
        return;
    }

    const auto status = result.statusText.empty() ? result.status : result.statusText;
    finishProofDump (juce::String (createdProjectOpenProofDisplayName()), status, result.error, directory);
}

void MainComponent::dumpAppSaveProof()
{
    const auto directory = proofDumpDirectory (appSaveProofDirectoryName());

    AppSaveProofRunRequest request;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runAppSaveProof (request);
    if (! result.ok && ! result.statusText.empty())
    {
        finishProofDump (juce::String (appSaveProofDisplayName()), "failed", result.statusText, directory);
        return;
    }

    const auto status = result.statusText.empty() ? result.status : result.statusText;
    finishProofDump (juce::String (appSaveProofDisplayName()), status, result.error, directory);
}

void MainComponent::dumpAppStatusProof()
{
    const auto directory = proofDumpDirectory (appStatusProofDirectoryName());

    AppStatusProofRunRequest request;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runAppStatusProof (request);
    if (! result.ok && ! result.statusText.empty())
    {
        finishProofDump (juce::String (appStatusProofDisplayName()), "failed", result.statusText, directory);
        return;
    }

    const auto status = result.statusText.empty() ? result.status : result.statusText;
    finishProofDump (juce::String (appStatusProofDisplayName()), status, result.error, directory);
}

void MainComponent::dumpWorkbenchStatusSurfaceProof()
{
    const auto directory = proofDumpDirectory (workbenchStatusSurfaceProofDirectoryName());

    WorkbenchStatusSurfaceProofRunRequest request;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runWorkbenchStatusSurfaceProof (request);
    if (! result.ok && ! result.statusText.empty())
    {
        finishProofDump (juce::String (workbenchStatusSurfaceProofDisplayName()),
                         "failed",
                         result.statusText,
                         directory);
        return;
    }

    const auto status = result.statusText.empty() ? result.status : result.statusText;
    finishProofDump (juce::String (workbenchStatusSurfaceProofDisplayName()),
                     status,
                     result.error,
                     directory);
}

void MainComponent::dumpWorkbenchGraphSurfaceProof()
{
    const auto directory = proofDumpDirectory (workbenchGraphSurfaceProofDirectoryName());

    WorkbenchGraphSurfaceProofRunRequest request;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runWorkbenchGraphSurfaceProof (request);
    if (! result.ok && ! result.statusText.empty())
    {
        finishProofDump (juce::String (workbenchGraphSurfaceProofDisplayName()),
                         "failed",
                         result.statusText,
                         directory);
        return;
    }

    const auto status = result.statusText.empty() ? result.status : result.statusText;
    finishProofDump (juce::String (workbenchGraphSurfaceProofDisplayName()),
                     status,
                     result.error,
                     directory);
}

void MainComponent::dumpWorkbenchCanvasSurfaceProof()
{
    const auto directory = proofDumpDirectory (workbenchCanvasSurfaceProofDirectoryName());

    WorkbenchCanvasSurfaceProofRunRequest request;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runWorkbenchCanvasSurfaceProof (request);
    if (! result.ok && ! result.statusText.empty())
    {
        finishProofDump (juce::String (workbenchCanvasSurfaceProofDisplayName()),
                         "failed",
                         result.statusText,
                         directory);
        return;
    }

    const auto status = result.statusText.empty() ? result.status : result.statusText;
    finishProofDump (juce::String (workbenchCanvasSurfaceProofDisplayName()),
                     status,
                     result.error,
                     directory);
}

void MainComponent::dumpWorkbenchRuntimeSurfaceProof()
{
    const auto directory = proofDumpDirectory (workbenchRuntimeSurfaceProofDirectoryName());

    WorkbenchRuntimeSurfaceProofRunRequest request;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runWorkbenchRuntimeSurfaceProof (request);
    if (! result.ok && ! result.statusText.empty())
    {
        finishProofDump (juce::String (workbenchRuntimeSurfaceProofDisplayName()),
                         "failed",
                         result.statusText,
                         directory);
        return;
    }

    const auto status = result.statusText.empty() ? result.status : result.statusText;
    finishProofDump (juce::String (workbenchRuntimeSurfaceProofDisplayName()),
                     status,
                     result.error,
                     directory);
}

CommandResult MainComponent::saveActiveWork (GraphSession& session)
{
    const auto result = workbenchController.saveCurrentSession (session);
    statusLabel.setText ((result.ok ? "save_work: " : "save_work failed: ") + juce::String (result.message),
                         juce::dontSendNotification);
    updateWorkbenchStatusSurface();
    updateWorkbenchGraphSurface();
    updateWorkbenchCanvasSurface();
    updateWorkbenchRuntimeSurface();
    updateWorkbenchCookPlanSurface();

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

void MainComponent::updateWorkbenchStatusSurface()
{
    const auto surface = makeWorkbenchStatusSurface (workbenchController.appStatusSnapshot());

    for (std::size_t index = 0; index < workbenchStatusLabels.size(); ++index)
    {
        auto& label = workbenchStatusLabels[index];

        if (index >= surface.rows.size())
        {
            label.setText ({}, juce::dontSendNotification);
            continue;
        }

        const auto& row = surface.rows[index];
        label.setText (juce::String (row.text), juce::dontSendNotification);
        label.setColour (juce::Label::textColourId, workbenchStatusToneColour (row.tone));
    }
}

void MainComponent::updateWorkbenchGraphSurface()
{
    const auto surface = makeWorkbenchGraphSurface (workbenchController.currentSession());

    for (std::size_t index = 0; index < workbenchGraphLabels.size(); ++index)
    {
        auto& label = workbenchGraphLabels[index];

        if (index >= surface.rows.size())
        {
            label.setText ({}, juce::dontSendNotification);
            continue;
        }

        const auto& row = surface.rows[index];
        label.setText (juce::String (row.text), juce::dontSendNotification);
        label.setColour (juce::Label::textColourId, workbenchStatusToneColour (row.tone));
    }
}

void MainComponent::updateWorkbenchCanvasSurface()
{
    const auto surface = makeWorkbenchCanvasSurface (workbenchController.currentSession());

    for (std::size_t index = 0; index < workbenchCanvasLabels.size(); ++index)
    {
        auto& label = workbenchCanvasLabels[index];

        if (index >= surface.rows.size())
        {
            label.setText ({}, juce::dontSendNotification);
            continue;
        }

        const auto& row = surface.rows[index];
        label.setText (juce::String (row.text), juce::dontSendNotification);
        label.setColour (juce::Label::textColourId, workbenchStatusToneColour (row.tone));
    }
}

void MainComponent::updateWorkbenchRuntimeSurface()
{
    const auto surface = makeWorkbenchRuntimeSurface (workbenchController.currentSession());

    for (std::size_t index = 0; index < workbenchRuntimeLabels.size(); ++index)
    {
        auto& label = workbenchRuntimeLabels[index];

        if (index >= surface.rows.size())
        {
            label.setText ({}, juce::dontSendNotification);
            continue;
        }

        const auto& row = surface.rows[index];
        label.setText (juce::String (row.text), juce::dontSendNotification);
        label.setColour (juce::Label::textColourId, workbenchStatusToneColour (row.tone));
    }
}

void MainComponent::updateWorkbenchCookPlanSurface()
{
    const auto surface = makeWorkbenchCookPlanSurface (workbenchController.currentSession());

    for (std::size_t index = 0; index < workbenchCookPlanLabels.size(); ++index)
    {
        auto& label = workbenchCookPlanLabels[index];

        if (index >= surface.rows.size())
        {
            label.setText ({}, juce::dontSendNotification);
            continue;
        }

        const auto& row = surface.rows[index];
        label.setText (juce::String (row.text), juce::dontSendNotification);
        label.setColour (juce::Label::textColourId, workbenchStatusToneColour (row.tone));
    }
}

void MainComponent::openWorkbenchSession()
{
    activeWorkPreparation = prepareActiveWorkProjectForOpen();

    WorkbenchSessionOpenStatusRequest request;
    request.activeWorkManifestPath = activeWorkManifestFile().getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();
    request.saveStatus = "clean";
    request.proofStatus = "g1-ready";
    request.previewStatus = "preview-ready";

    (void) workbenchController.openCurrentSession (request);

    auto statusText = juce::String (workbenchController.statusText());
    if (! activeWorkPreparation.ok)
        statusText = "active work prepare failed: " + juce::String (activeWorkPreparation.error)
                     + "; "
                     + statusText;

    statusLabel.setText (statusText,
                         juce::dontSendNotification);
    updateWorkbenchStatusSurface();
    updateWorkbenchGraphSurface();
    updateWorkbenchCanvasSurface();
    updateWorkbenchRuntimeSurface();
    updateWorkbenchCookPlanSurface();
}

void MainComponent::loadStoredPerformancePreferences()
{
    const auto result = loadPerformancePreferences (
        std::filesystem::path (performancePreferencesFile().getFullPathName().toStdString()));

    performancePreferences = result.ok
                                 ? sanitizePerformancePreferences (result.preferences)
                                 : makeDefaultPerformancePreferences();
    preferencesPanel.applyPerformancePreferences (performancePreferences);
}

void MainComponent::saveStoredPerformancePreferences()
{
    (void) savePerformancePreferences (
        std::filesystem::path (performancePreferencesFile().getFullPathName().toStdString()),
        performancePreferences);
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
    const auto realtime = audioInputAnalyzer.consumeRealtimeDelivery (audioRealtimeLastSeenSequence);
    const auto realtimeStatusText = makeAudioRealtimeDeliveryStatusText (realtime);
    const auto analyzerSnapshot = realtime.available ? realtime.snapshot : audioInputAnalyzer.getSnapshot();
    const auto bridge = makeLoudnessRuntimeBridgeSnapshot (RuntimeExecutionSnapshot{}, analyzerSnapshot);
    const auto& snapshot = bridge.analyzer;

    rmsLabel.setText ("rms " + juce::String (snapshot.rms, 4), juce::dontSendNotification);
    peakLabel.setText ("peak " + juce::String (snapshot.peak, 4), juce::dontSendNotification);
    loudnessLabel.setText ("loudness " + juce::String (snapshot.loudness, 4), juce::dontSendNotification);
    activeLabel.setText (juce::String ("active ") + (snapshot.active ? "yes" : "no"), juce::dontSendNotification);
    preview.setLoudness (snapshot.loudness);
    sendMidiForSnapshot (snapshot);
    tickLiveIOControl (snapshot, realtime);
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
                                      + " samples / "
                                      + juce::String (realtimeStatusText),
                                  juce::dontSendNotification);
    }
}

void MainComponent::applyMidiPreferences (MidiPreferences preferences)
{
    performancePreferences.midi = std::move (preferences);
    performancePreferences = sanitizePerformancePreferences (performancePreferences);

    const auto identifier = juce::String (performancePreferences.midi.outputIdentifier);

    if (identifier == openedMidiOutputIdentifier)
    {
        saveStoredPerformancePreferences();
        return;
    }

    midiOutput.reset();
    openedMidiOutputIdentifier = identifier;

    if (identifier.isEmpty())
    {
        midiStatus = "midi off";
        saveStoredPerformancePreferences();
        return;
    }

    midiOutput = juce::MidiOutput::openDevice (identifier);

    if (midiOutput == nullptr)
    {
        midiStatus = "midi open failed";
        saveStoredPerformancePreferences();
        return;
    }

    midiStatus = "midi " + juce::String (performancePreferences.midi.outputName);
    saveStoredPerformancePreferences();
}

void MainComponent::applyLiveIOPreferences (LiveIOPreferences preferences)
{
    performancePreferences.liveIO = preferences;
    performancePreferences = sanitizePerformancePreferences (performancePreferences);
    liveIOController.applyLiveIOPreferences (performancePreferences.liveIO);
    saveStoredPerformancePreferences();
}

void MainComponent::armMidiTeach (LiveIOMidiTeachTarget target)
{
    stopMidiTeachListening();

    const auto inputCount = startMidiTeachListening();
    const auto view = liveIOController.armMidiTeach (target, inputCount);
    midiTeachArmedForCallback.store (view.shouldListen);

    if (! view.shouldListen)
        stopMidiTeachListening();

    preferencesPanel.setMidiTeachStatus (juce::String (view.statusText));
}

void MainComponent::cancelMidiTeach()
{
    const auto view = liveIOController.cancelMidiTeach();
    stopMidiTeachListening();
    preferencesPanel.setMidiTeachStatus (juce::String (view.statusText));
}

int MainComponent::startMidiTeachListening()
{
    midiInputsEnabledForTeach.clear();
    const auto inputs = juce::MidiInput::getAvailableDevices();
    std::vector<std::string> availableIdentifiers;

    for (const auto& input : inputs)
        availableIdentifiers.push_back (input.identifier.toStdString());

    const auto selectedIdentifiers = midiTeachInputIdentifiers (
        performancePreferences,
        availableIdentifiers);

    if (selectedIdentifiers.empty())
        return 0;

    if (! midiTeachCallbackRegistered)
    {
        audioDeviceManager.addMidiInputDeviceCallback ({}, this);
        midiTeachCallbackRegistered = true;
    }

    for (const auto& input : inputs)
    {
        if (std::find (selectedIdentifiers.begin(),
                       selectedIdentifiers.end(),
                       input.identifier.toStdString()) == selectedIdentifiers.end())
            continue;

        if (! audioDeviceManager.isMidiInputDeviceEnabled (input.identifier))
        {
            audioDeviceManager.setMidiInputDeviceEnabled (input.identifier, true);
            midiInputsEnabledForTeach.push_back (input.identifier);
        }
    }

    return static_cast<int> (selectedIdentifiers.size());
}

void MainComponent::stopMidiTeachListening()
{
    midiTeachArmedForCallback.store (false);

    if (midiTeachCallbackRegistered)
    {
        audioDeviceManager.removeMidiInputDeviceCallback ({}, this);
        midiTeachCallbackRegistered = false;
    }

    for (const auto& identifier : midiInputsEnabledForTeach)
        audioDeviceManager.setMidiInputDeviceEnabled (identifier, false);

    midiInputsEnabledForTeach.clear();
}

void MainComponent::handleMidiTeachMessage (LiveIOMidiTeachIncomingMessage message)
{
    const auto view = liveIOController.handleMidiTeachMessage (message);

    if (view.learned)
    {
        stopMidiTeachListening();
        preferencesPanel.applyLearnedMidiCc (view.learnedTarget, view.learnedChannel, view.learnedCc);
    }

    preferencesPanel.setMidiTeachStatus (juce::String (view.statusText));
}

void MainComponent::handleIncomingMidiMessage (juce::MidiInput*, const juce::MidiMessage& message)
{
    if (! midiTeachArmedForCallback.load())
        return;

    if (! message.isController())
        return;

    const auto incoming = makeLiveIOMidiTeachControlChange (
        message.getChannel(),
        message.getControllerNumber(),
        message.getControllerValue());

    // MIDI input arrives on a high-priority thread; teach state and UI update on the message thread.
    juce::MessageManager::callAsync (
        [safe = juce::Component::SafePointer<MainComponent> (this), incoming]
        {
            if (safe != nullptr)
                safe->handleMidiTeachMessage (incoming);
        });
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

void MainComponent::tickLiveIOControl (const AudioAnalyzerSnapshot& snapshot,
                                       const AudioRealtimeDeliveryResult& realtime)
{
    LiveIOAppTimerRequest request;
    request.preferences = performancePreferences;
    request.snapshot = snapshot;
    request.timestampMs = static_cast<std::int64_t> (juce::Time::getMillisecondCounterHiRes());
    request.midiSender = [this] (const LiveIOMidiOutputDevice& device,
                                 const LiveIOMidiCcMessage& message)
    {
        if (midiOutput == nullptr)
        {
            return LiveIOMidiOutputDeviceSendResult {
                false,
                false,
                "midi output is not open: " + device.identifier
            };
        }

        midiOutput->sendMessageNow (juce::MidiMessage::controllerEvent (
            message.channel,
            message.cc,
            message.value));
        return LiveIOMidiOutputDeviceSendResult { true, true, "" };
    };

    const auto result = liveIOController.tick (request);
    const auto indicator = withLiveIORealtimeTelemetry (
        result.indicator,
        makeRealtimeIndicatorTelemetry (realtime));
    if (indicator.hasShaderUniform)
    {
        preview.setInputSnapshot (makeShaderPreviewInputFromUniformEvidence (
            indicator.shaderUniformBindingId,
            indicator.shaderUniformName,
            indicator.shaderUniformValue,
            indicator.shaderUniformSampleCounter));
    }
    liveIOStatus = juce::String (indicator.text);
    liveIOStatusTone = juce::String (indicator.tone);
}
}
