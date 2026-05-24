#include "MainComponent.h"

#include "AIWorkerCommand.h"
#include "CompoundPatch.h"
#include "GraphEndpoint.h"
#include "GraphContract.h"
#include "InteractionContract.h"
#include "JsonWriter.h"
#include "RuntimeRegistry.h"
#include "StorageCommand.h"
#include "StorageContract.h"

#include <algorithm>
#include <sstream>
#include <vector>

namespace myworld
{
namespace
{
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

juce::File projectDirectory()
{
    const auto environmentPath = juce::SystemStats::getEnvironmentVariable ("MY_WORLD_PROJECT_DIR", {});

    if (environmentPath.isNotEmpty())
        return juce::File (environmentPath);

    return juce::File::getSpecialLocation (juce::File::userDesktopDirectory)
        .getChildFile (juce::String::fromUTF8 ("\xe6\x88\x91\xe7\x9a\x84\xe4\xb8\x96\xe7\x95\x8c"));
}

juce::File proofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile ("v1-shader-proof");
}

juce::File audioProofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile ("a1-audio-proof");
}

juce::File c2StorageProofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile ("c2-storage-proof");
}

juce::File c3SaveWorkProofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile ("c3-save-work-proof");
}

juce::File c4AIWorkerSaveWorkProofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile ("c4-ai-worker-save-work-proof");
}

juce::File defaultActiveWorkManifestFile()
{
    return projectDirectory().getChildFile ("debug").getChildFile ("c3-active-work").getChildFile ("myworld.work.json");
}

juce::File activeWorkManifestFile()
{
    const auto environmentPath = juce::SystemStats::getEnvironmentVariable ("MY_WORLD_ACTIVE_WORK_MANIFEST", {});

    if (environmentPath.isNotEmpty())
        return juce::File (environmentPath);

    return defaultActiveWorkManifestFile();
}

juce::File parentDirectory (juce::File file, const int levels)
{
    for (int i = 0; i < levels; ++i)
        file = file.getParentDirectory();

    return file;
}

juce::String defaultModuleLibraryPath()
{
    return "fixtures/module-libraries/default.module-library.json";
}

std::vector<std::string> moduleLibraryCandidatePaths (const juce::String& libraryPath)
{
    const auto executableDir = juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory();
    const auto buildAppRepoRoot = parentDirectory (executableDir, 5);

    return {
        juce::File::getCurrentWorkingDirectory().getChildFile (libraryPath).getFullPathName().toStdString(),
        buildAppRepoRoot.getChildFile (libraryPath).getFullPathName().toStdString()
    };
}

std::vector<std::string> repoCandidatePaths (const juce::String& relativePath)
{
    return moduleLibraryCandidatePaths (relativePath);
}

bool hasEdgeId (const GraphContract& graph, const std::string& edgeId)
{
    return std::any_of (graph.editorGraph.edges.begin(),
                        graph.editorGraph.edges.end(),
                        [&edgeId] (const auto& edge) {
                            return edge.id == edgeId;
                        });
}

bool writeTextFile (const juce::File& file, const std::string& text)
{
    return file.replaceWithText (juce::String::fromUTF8 (text.c_str()), false, false, "\n");
}

bool copyTextFile (const juce::File& source, const juce::File& target)
{
    if (! target.getParentDirectory().createDirectory())
        return false;

    if (target.existsAsFile() && ! target.deleteFile())
        return false;

    return source.copyFileTo (target);
}

bool prepareDefaultActiveWorkProject (const juce::File& workManifestFile, std::string& error)
{
    if (workManifestFile.existsAsFile())
        return true;

    const auto patchFile = workManifestFile.getParentDirectory().getChildFile ("patches").getChildFile ("main.patch.json");

    for (const auto& candidate : repoCandidatePaths ("fixtures/storage/c2-compound-work/myworld.work.json"))
    {
        const auto sourceManifest = juce::File (candidate);
        const auto sourcePatch = sourceManifest.getParentDirectory().getChildFile ("patches").getChildFile ("main.patch.json");

        if (copyTextFile (sourceManifest, workManifestFile) && copyTextFile (sourcePatch, patchFile))
            return true;

        error = "could not copy active work fixture from " + candidate;
    }

    if (error.empty())
        error = "could not prepare default active work";

    return false;
}

std::string makeC2StorageReportJson (bool ok,
                                     const std::string& workManifestPath,
                                     const std::string& savedPatchPath,
                                     const std::string& saveStatus,
                                     const GraphSession& session,
                                     bool publicInputEdge,
                                     bool publicOutputEdge,
                                     bool monoMixLayout,
                                     double monoMixX,
                                     double monoMixY,
                                     const std::string& error)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"c2StorageProof\",\n";
    out << "  \"ok\": " << (ok ? "true" : "false") << ",\n";
    out << "  \"source\": \"PatchDocument\",\n";
    out << "  \"usesInteractionState\": false,\n";
    out << "  \"workManifestPath\": " << jsonQuoted (workManifestPath) << ",\n";
    out << "  \"savedPatchPath\": " << jsonQuoted (savedPatchPath) << ",\n";
    out << "  \"saveStatus\": " << jsonQuoted (saveStatus) << ",\n";
    out << "  \"editorNodeCount\": " << session.graph.editorGraph.nodes.size() << ",\n";
    out << "  \"editorEdgeCount\": " << session.graph.editorGraph.edges.size() << ",\n";
    out << "  \"runtimeNodeCount\": " << session.graph.runtimeGraph.nodes.size() << ",\n";
    out << "  \"runtimeEdgeCount\": " << session.graph.runtimeGraph.edges.size() << ",\n";
    out << "  \"publicInputEdge\": " << (publicInputEdge ? "true" : "false") << ",\n";
    out << "  \"publicOutputEdge\": " << (publicOutputEdge ? "true" : "false") << ",\n";
    out << "  \"expandedLayout\": {\n";
    out << "    \"nodeId\": \"library_loud1/mono_mix\",\n";
    out << "    \"matches\": " << (monoMixLayout ? "true" : "false") << ",\n";
    out << "    \"x\": " << monoMixX << ",\n";
    out << "    \"y\": " << monoMixY << "\n";
    out << "  },\n";
    out << "  \"error\": " << jsonQuoted (error) << "\n";
    out << "}\n";
    return out.str();
}

std::string makeC3SaveWorkReportJson (bool ok,
                                      const std::string& workManifestPath,
                                      const std::string& savedPatchPath,
                                      const std::string& saveLogPath,
                                      const std::string& saveStatus,
                                      const std::string& commandLogStatus,
                                      const SaveLogLoadResult& saveLog,
                                      const GraphSession& session,
                                      bool publicInputEdge,
                                      bool publicOutputEdge,
                                      bool monoMixLayout,
                                      double monoMixX,
                                      double monoMixY,
                                      const std::string& error)
{
    const auto saveLogStatus = saveLog.entries.empty() ? std::string {}
                                                       : saveLog.entries.back().status;
    const auto commitStatus = saveLog.entries.empty() ? std::string {}
                                                      : saveLog.entries.back().commitStatus;

    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"c3SaveWorkProof\",\n";
    out << "  \"ok\": " << (ok ? "true" : "false") << ",\n";
    out << "  \"source\": \"PatchDocument\",\n";
    out << "  \"usesInteractionState\": false,\n";
    out << "  \"workManifestPath\": " << jsonQuoted (workManifestPath) << ",\n";
    out << "  \"savedPatchPath\": " << jsonQuoted (savedPatchPath) << ",\n";
    out << "  \"saveLogPath\": " << jsonQuoted (saveLogPath) << ",\n";
    out << "  \"saveStatus\": " << jsonQuoted (saveStatus) << ",\n";
    out << "  \"commandLogStatus\": " << jsonQuoted (commandLogStatus) << ",\n";
    out << "  \"saveLogOk\": " << (saveLog.ok ? "true" : "false") << ",\n";
    out << "  \"saveLogEntries\": " << saveLog.entries.size() << ",\n";
    out << "  \"saveLogStatus\": " << jsonQuoted (saveLogStatus) << ",\n";
    out << "  \"commitStatus\": " << jsonQuoted (commitStatus) << ",\n";
    out << "  \"editorNodeCount\": " << session.graph.editorGraph.nodes.size() << ",\n";
    out << "  \"editorEdgeCount\": " << session.graph.editorGraph.edges.size() << ",\n";
    out << "  \"runtimeNodeCount\": " << session.graph.runtimeGraph.nodes.size() << ",\n";
    out << "  \"runtimeEdgeCount\": " << session.graph.runtimeGraph.edges.size() << ",\n";
    out << "  \"publicInputEdge\": " << (publicInputEdge ? "true" : "false") << ",\n";
    out << "  \"publicOutputEdge\": " << (publicOutputEdge ? "true" : "false") << ",\n";
    out << "  \"expandedLayout\": {\n";
    out << "    \"nodeId\": \"library_loud1/mono_mix\",\n";
    out << "    \"matches\": " << (monoMixLayout ? "true" : "false") << ",\n";
    out << "    \"x\": " << monoMixX << ",\n";
    out << "    \"y\": " << monoMixY << "\n";
    out << "  },\n";
    out << "  \"error\": " << jsonQuoted (error) << "\n";
    out << "}\n";
    return out.str();
}

std::string makeC4AIWorkerSaveWorkReportJson (bool ok,
                                              const AIWorkerCommandRequest& request,
                                              const AIWorkerCommandResult& result,
                                              const std::vector<std::string>& allowedOperations,
                                              const SaveLogLoadResult& saveLog,
                                              const GraphSession& session,
                                              bool publicInputEdge,
                                              bool publicOutputEdge,
                                              bool monoMixLayout,
                                              double monoMixX,
                                              double monoMixY,
                                              const std::string& aiCommandLogStatus,
                                              const std::string& error)
{
    const auto saveLogStatus = saveLog.entries.empty() ? std::string {}
                                                       : saveLog.entries.back().status;
    const auto collaborationIntentStatus = session.collaborationLog.empty() ? std::string {}
                                                                            : session.collaborationLog.front().status;
    const auto collaborationResultStatus = session.collaborationLog.empty() ? std::string {}
                                                                            : session.collaborationLog.back().status;
    const auto collaborationProofEvidence = session.collaborationLog.empty() ? std::string {}
                                                                             : session.collaborationLog.back().proofEvidence;
    const auto saveWorkAllowed = std::find (allowedOperations.begin(), allowedOperations.end(), "save_work")
                                 != allowedOperations.end();

    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"c4AIWorkerSaveWorkProof\",\n";
    out << "  \"ok\": " << (ok ? "true" : "false") << ",\n";
    out << "  \"source\": \"PatchDocument\",\n";
    out << "  \"usesInteractionState\": " << (result.evidence.usesInteractionState ? "true" : "false") << ",\n";
    out << "  \"allowedSaveWork\": " << (saveWorkAllowed ? "true" : "false") << ",\n";
    out << "  \"operation\": " << jsonQuoted (result.operation) << ",\n";
    out << "  \"commandId\": " << jsonQuoted (result.commandId) << ",\n";
    out << "  \"workerId\": " << jsonQuoted (result.workerId) << ",\n";
    out << "  \"intent\": " << jsonQuoted (request.intent) << ",\n";
    out << "  \"workManifestPath\": " << jsonQuoted (request.workManifestPath) << ",\n";
    out << "  \"savedPatchPath\": " << jsonQuoted (result.evidence.patchPath) << ",\n";
    out << "  \"saveLogPath\": " << jsonQuoted (result.evidence.saveLogPath) << ",\n";
    out << "  \"status\": " << jsonQuoted (result.status) << ",\n";
    out << "  \"storageCommandLogStatus\": " << jsonQuoted (result.evidence.storageCommandLogStatus) << ",\n";
    out << "  \"aiCommandLogStatus\": " << jsonQuoted (aiCommandLogStatus) << ",\n";
    out << "  \"patchReloaded\": " << (result.evidence.patchReloaded ? "true" : "false") << ",\n";
    out << "  \"saveLogOk\": " << (saveLog.ok ? "true" : "false") << ",\n";
    out << "  \"saveLogEntries\": " << saveLog.entries.size() << ",\n";
    out << "  \"saveLogStatus\": " << jsonQuoted (saveLogStatus) << ",\n";
    out << "  \"collaborationLogEntries\": " << session.collaborationLog.size() << ",\n";
    out << "  \"collaborationIntentStatus\": " << jsonQuoted (collaborationIntentStatus) << ",\n";
    out << "  \"collaborationResultStatus\": " << jsonQuoted (collaborationResultStatus) << ",\n";
    out << "  \"collaborationProofEvidence\": " << jsonQuoted (collaborationProofEvidence) << ",\n";
    out << "  \"editorNodeCount\": " << session.graph.editorGraph.nodes.size() << ",\n";
    out << "  \"editorEdgeCount\": " << session.graph.editorGraph.edges.size() << ",\n";
    out << "  \"runtimeNodeCount\": " << session.graph.runtimeGraph.nodes.size() << ",\n";
    out << "  \"runtimeEdgeCount\": " << session.graph.runtimeGraph.edges.size() << ",\n";
    out << "  \"publicInputEdge\": " << (publicInputEdge ? "true" : "false") << ",\n";
    out << "  \"publicOutputEdge\": " << (publicOutputEdge ? "true" : "false") << ",\n";
    out << "  \"expandedLayout\": {\n";
    out << "    \"nodeId\": \"library_loud1/mono_mix\",\n";
    out << "    \"matches\": " << (monoMixLayout ? "true" : "false") << ",\n";
    out << "    \"x\": " << monoMixX << ",\n";
    out << "    \"y\": " << monoMixY << "\n";
    out << "  },\n";
    out << "  \"error\": " << jsonQuoted (error) << "\n";
    out << "}\n";
    return out.str();
}

RuntimeRegistryLoadResult loadAudioProofRuntimeRegistry()
{
    std::string lastError;

    for (const auto& path : moduleLibraryCandidatePaths (defaultModuleLibraryPath()))
    {
        const auto registry = loadRuntimeRegistryFromModuleLibrary (path);
        if (registry.ok)
            return registry;

        lastError = registry.error;
    }

    return { false, {}, lastError.empty() ? "could not load module library: " + defaultModuleLibraryPath().toStdString()
                                          : lastError };
}
}

MainComponent::MainComponent (bool dumpProofOnStart,
                              bool dumpAudioProofOnStart,
                              bool dumpC2StorageProofOnStart,
                              bool dumpC3SaveWorkProofOnStart,
                              bool dumpC4AIWorkerSaveWorkProofOnStart,
                              bool quitAfterStartupDump)
    : preferencesPanel (audioDeviceManager),
      graph (makeDefaultShaderOutputGraph()),
      performancePreferences (makeDefaultPerformancePreferences()),
      shouldQuitAfterStartupDump (quitAfterStartupDump)
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
    addAndMakeVisible (rmsLabel);
    addAndMakeVisible (peakLabel);
    addAndMakeVisible (loudnessLabel);
    addAndMakeVisible (activeLabel);
    addAndMakeVisible (midiStatusLabel);

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
    addAndMakeVisible (preview);

    startAudioInput();
    audioInputAnalyzer.setAnalysisGain (preferencesPanel.getAnalysisGain());
    applyMidiPreferences (preferencesPanel.getMidiPreferences());
    startTimerHz (30);

    if (dumpProofOnStart)
    {
        juce::Timer::callAfterDelay (750, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpProof();
        });
    }

    if (dumpAudioProofOnStart)
    {
        juce::Timer::callAfterDelay (2500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpAudioProof();
        });
    }

    if (dumpC2StorageProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpC2StorageProof();
        });
    }

    if (dumpC3SaveWorkProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpC3SaveWorkProof();
        });
    }

    if (dumpC4AIWorkerSaveWorkProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpC4AIWorkerSaveWorkProof();
        });
    }

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
    midiStatusLabel.setBounds (audioRow);

    area.removeFromTop (10);

    preview.setBounds (area);
}

void MainComponent::dumpProof()
{
    const auto directory = proofDumpDirectory();
    statusLabel.setText ("proof dump requested: " + directory.getFullPathName(), juce::dontSendNotification);
    preview.requestProofDump (directory, graph);
}

void MainComponent::dumpAudioProof()
{
    const auto directory = audioProofDumpDirectory();

    if (! directory.createDirectory())
    {
        statusLabel.setText ("audio proof failed: could not create " + directory.getFullPathName(),
                             juce::dontSendNotification);
        return;
    }

    const auto snapshot = audioInputAnalyzer.getSnapshot();
    const auto runtimeRegistry = loadAudioProofRuntimeRegistry();

    if (! runtimeRegistry.ok)
    {
        statusLabel.setText ("audio proof failed: " + juce::String (runtimeRegistry.error),
                             juce::dontSendNotification);
        return;
    }

    const auto runtimeInput = makeRuntimeSyntheticAudioInputFromAnalyzerSnapshot (snapshot, 64);
    const auto runtimeExecution = executeRuntimeRegistryWithSyntheticAudio (runtimeRegistry.registry, runtimeInput);

    if (! runtimeExecution.ok)
    {
        statusLabel.setText ("audio proof failed: " + juce::String (runtimeExecution.error),
                             juce::dontSendNotification);
        return;
    }

    const auto bridge = makeLoudnessRuntimeBridgeSnapshot (runtimeExecution.snapshot, snapshot);
    const auto json = juce::String()
        + "{\n"
        + "  \"sampleRate\": " + juce::String (audioInputAnalyzer.getSampleRate(), 0) + ",\n"
        + "  \"bufferSize\": " + juce::String (audioInputAnalyzer.getBufferSize()) + ",\n"
        + "  \"rms\": " + juce::String (snapshot.rms, 6) + ",\n"
        + "  \"peak\": " + juce::String (snapshot.peak, 6) + ",\n"
        + "  \"loudness\": " + juce::String (snapshot.loudness, 6) + ",\n"
        + "  \"gate\": " + juce::String (snapshot.gate, 6) + ",\n"
        + "  \"confidence\": " + juce::String (snapshot.confidence, 6) + ",\n"
        + "  \"active\": " + juce::String (snapshot.active ? "true" : "false") + ",\n"
        + "  \"analysisGain\": " + juce::String (performancePreferences.audio.analysisGain, 3) + ",\n"
        + "  \"midi\": {\n"
        + "    \"streamEnabled\": " + juce::String (performancePreferences.midi.streamEnabled ? "true" : "false") + ",\n"
        + "    \"mapModeEnabled\": " + juce::String (performancePreferences.midi.mapModeEnabled ? "true" : "false") + ",\n"
        + "    \"channel\": " + juce::String (performancePreferences.midi.channel) + ",\n"
        + "    \"loudnessCc\": " + juce::String (performancePreferences.midi.loudnessCc) + ",\n"
        + "    \"mapCc\": " + juce::String (performancePreferences.midi.mapCc) + ",\n"
        + "    \"outputName\": \"" + juce::String (performancePreferences.midi.outputName) + "\"\n"
        + "  },\n"
        + "  \"sampleCounter\": " + juce::String (static_cast<juce::int64> (snapshot.sampleCounter)) + "\n"
        + "}\n";

    const auto audioStatsFile = directory.getChildFile ("audio_stats.json");
    const auto loudnessCompoundFile = directory.getChildFile ("loudness_compound.json");
    const auto loudnessRuntimeExecutionFile = directory.getChildFile ("loudness_runtime_execution.json");
    const auto loudnessRuntimeBridgeFile = directory.getChildFile ("loudness_runtime_bridge.json");

    if (! audioStatsFile.replaceWithText (json, false, false, "\n"))
    {
        statusLabel.setText ("audio proof failed: could not write " + audioStatsFile.getFullPathName(),
                             juce::dontSendNotification);
        return;
    }

    if (! loudnessCompoundFile.replaceWithText (juce::String::fromUTF8 (makeCompoundPatchJson (makeLoudnessCompoundPatchSpec()).c_str()),
                                                false,
                                                false,
                                                "\n"))
    {
        statusLabel.setText ("audio proof failed: could not write " + loudnessCompoundFile.getFullPathName(),
                             juce::dontSendNotification);
        return;
    }

    const auto runtimeExecutionJson = makeRuntimeExecutionJson (runtimeExecution.snapshot);

    if (! loudnessRuntimeExecutionFile.replaceWithText (juce::String::fromUTF8 (runtimeExecutionJson.c_str()),
                                                        false,
                                                        false,
                                                        "\n"))
    {
        statusLabel.setText ("audio proof failed: could not write " + loudnessRuntimeExecutionFile.getFullPathName(),
                             juce::dontSendNotification);
        return;
    }

    const auto runtimeBridgeJson = makeLoudnessRuntimeBridgeJson (bridge);

    if (! loudnessRuntimeBridgeFile.replaceWithText (juce::String::fromUTF8 (runtimeBridgeJson.c_str()), false, false, "\n"))
    {
        statusLabel.setText ("audio proof failed: could not write " + loudnessRuntimeBridgeFile.getFullPathName(),
                             juce::dontSendNotification);
        return;
    }

    statusLabel.setText ("audio proof dumped: " + directory.getFullPathName(), juce::dontSendNotification);

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

void MainComponent::dumpC2StorageProof()
{
    const auto directory = c2StorageProofDumpDirectory();
    const auto reportFile = directory.getChildFile ("reload_report.json");
    const auto savedPatchFile = directory.getChildFile ("saved_main.patch.json");

    if (! directory.createDirectory())
    {
        statusLabel.setText ("c2 storage proof failed: could not create " + directory.getFullPathName(),
                             juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    std::string workManifestPath;
    PatchDocumentLoadResult loadedPatch;
    std::string lastError;

    for (const auto& candidate : repoCandidatePaths ("fixtures/storage/c2-compound-work/myworld.work.json"))
    {
        const auto loaded = loadMainPatchDocumentForWork (candidate);
        if (loaded.ok)
        {
            workManifestPath = candidate;
            loadedPatch = loaded;
            break;
        }

        lastError = loaded.error;
    }

    if (! loadedPatch.ok)
    {
        const auto report = makeC2StorageReportJson (false,
                                                     {},
                                                     savedPatchFile.getFullPathName().toStdString(),
                                                     {},
                                                     makeGraphSession (GraphContract {}),
                                                     false,
                                                     false,
                                                     false,
                                                     0.0,
                                                     0.0,
                                                     lastError.empty() ? "could not load C2 work fixture" : lastError);
        writeTextFile (reportFile, report);
        statusLabel.setText ("c2 storage proof failed: " + juce::String (lastError), juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    auto activeSession = makeGraphSession (loadedPatch.document.graph);
    const auto activeDocument = makePatchDocument (loadedPatch.document.id,
                                                  loadedPatch.document.title,
                                                  activeSession.graph);
    const auto saveResult = savePatchDocument (savedPatchFile.getFullPathName().toStdString(), activeDocument);

    if (! saveResult.ok)
    {
        const auto report = makeC2StorageReportJson (false,
                                                     workManifestPath,
                                                     savedPatchFile.getFullPathName().toStdString(),
                                                     saveResult.status,
                                                     activeSession,
                                                     false,
                                                     false,
                                                     false,
                                                     0.0,
                                                     0.0,
                                                     saveResult.error);
        writeTextFile (reportFile, report);
        statusLabel.setText ("c2 storage proof failed: " + juce::String (saveResult.error), juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    const auto reloadedPatch = loadPatchDocument (savedPatchFile.getFullPathName().toStdString());
    if (! reloadedPatch.ok)
    {
        const auto report = makeC2StorageReportJson (false,
                                                     workManifestPath,
                                                     savedPatchFile.getFullPathName().toStdString(),
                                                     saveResult.status,
                                                     activeSession,
                                                     false,
                                                     false,
                                                     false,
                                                     0.0,
                                                     0.0,
                                                     reloadedPatch.error);
        writeTextFile (reportFile, report);
        statusLabel.setText ("c2 storage proof failed: " + juce::String (reloadedPatch.error), juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    auto reloadedSession = makeGraphSession (reloadedPatch.document.graph);

    CompoundPatchLoadResult loadedCompound;
    for (const auto& candidate : repoCandidatePaths ("fixtures/compounds/loudness.compound.json"))
    {
        const auto loaded = loadCompoundPatchSpec (candidate);
        if (loaded.ok)
        {
            loadedCompound = loaded;
            break;
        }

        lastError = loaded.error;
    }

    if (! loadedCompound.ok)
    {
        const auto report = makeC2StorageReportJson (false,
                                                     workManifestPath,
                                                     savedPatchFile.getFullPathName().toStdString(),
                                                     saveResult.status,
                                                     reloadedSession,
                                                     false,
                                                     false,
                                                     false,
                                                     0.0,
                                                     0.0,
                                                     lastError.empty() ? "could not load loudness compound fixture" : lastError);
        writeTextFile (reportFile, report);
        statusLabel.setText ("c2 storage proof failed: " + juce::String (lastError), juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    const auto relayoutGraph = makeCompoundPatchInteractionGraph (loadedCompound.spec,
                                                                  "library_loud1",
                                                                  reloadedSession.graph);
    const auto* monoMix = findEditorNode (relayoutGraph, "library_loud1/mono_mix");
    const auto monoMixX = monoMix == nullptr ? 0.0 : monoMix->position.x;
    const auto monoMixY = monoMix == nullptr ? 0.0 : monoMix->position.y;
    const auto publicInputEdge = hasEdgeId (reloadedSession.graph, "edge.live_audio.channels.library_loud1.audio.in");
    const auto publicOutputEdge = hasEdgeId (reloadedSession.graph, "edge.library_loud1.out.midi_loudness.value");
    const auto monoMixLayout = monoMix != nullptr && monoMixX == 358.0 && monoMixY == 146.0;
    const auto graphCountsMatch = reloadedSession.graph.editorGraph.edges.size()
                                  == reloadedSession.graph.runtimeGraph.edges.size();
    const auto ok = publicInputEdge && publicOutputEdge && monoMixLayout && graphCountsMatch;

    const auto report = makeC2StorageReportJson (ok,
                                                 workManifestPath,
                                                 savedPatchFile.getFullPathName().toStdString(),
                                                 saveResult.status,
                                                 reloadedSession,
                                                 publicInputEdge,
                                                 publicOutputEdge,
                                                 monoMixLayout,
                                                 monoMixX,
                                                 monoMixY,
                                                 ok ? std::string {} : "reloaded C2 graph did not match expected compound work");

    if (! writeTextFile (reportFile, report))
    {
        statusLabel.setText ("c2 storage proof failed: could not write " + reportFile.getFullPathName(),
                             juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    statusLabel.setText ((ok ? "c2 storage proof dumped: " : "c2 storage proof mismatch: ")
                             + directory.getFullPathName(),
                         juce::dontSendNotification);

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

void MainComponent::dumpC3SaveWorkProof()
{
    const auto directory = c3SaveWorkProofDumpDirectory();
    const auto workDirectory = directory.getChildFile ("work");
    const auto patchDirectory = workDirectory.getChildFile ("patches");
    const auto reportFile = directory.getChildFile ("save_work_report.json");
    const auto workManifestFile = workDirectory.getChildFile ("myworld.work.json");
    const auto savedPatchFile = patchDirectory.getChildFile ("main.patch.json");

    if (directory.exists() && ! directory.deleteRecursively())
    {
        statusLabel.setText ("c3 save_work proof failed: could not clear " + directory.getFullPathName(),
                             juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    if (! patchDirectory.createDirectory())
    {
        statusLabel.setText ("c3 save_work proof failed: could not create " + patchDirectory.getFullPathName(),
                             juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    std::string lastError;
    bool copiedFixture = false;
    for (const auto& candidate : repoCandidatePaths ("fixtures/storage/c2-compound-work/myworld.work.json"))
    {
        const auto sourceManifest = juce::File (candidate);
        const auto sourcePatch = sourceManifest.getParentDirectory().getChildFile ("patches").getChildFile ("main.patch.json");
        copiedFixture = copyTextFile (sourceManifest, workManifestFile) && copyTextFile (sourcePatch, savedPatchFile);
        if (copiedFixture)
            break;

        lastError = "could not copy C2 work fixture from " + candidate;
    }

    if (! copiedFixture)
    {
        const SaveLogLoadResult emptySaveLog;
        const auto report = makeC3SaveWorkReportJson (false,
                                                      workManifestFile.getFullPathName().toStdString(),
                                                      savedPatchFile.getFullPathName().toStdString(),
                                                      {},
                                                      {},
                                                      {},
                                                      emptySaveLog,
                                                      makeGraphSession (GraphContract {}),
                                                      false,
                                                      false,
                                                      false,
                                                      0.0,
                                                      0.0,
                                                      lastError.empty() ? "could not copy C3 work fixture" : lastError);
        writeTextFile (reportFile, report);
        statusLabel.setText ("c3 save_work proof failed: " + juce::String (lastError), juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    const auto loadedPatch = loadMainPatchDocumentForWork (workManifestFile.getFullPathName().toStdString());
    if (! loadedPatch.ok)
    {
        const SaveLogLoadResult emptySaveLog;
        const auto report = makeC3SaveWorkReportJson (false,
                                                      workManifestFile.getFullPathName().toStdString(),
                                                      savedPatchFile.getFullPathName().toStdString(),
                                                      {},
                                                      {},
                                                      {},
                                                      emptySaveLog,
                                                      makeGraphSession (GraphContract {}),
                                                      false,
                                                      false,
                                                      false,
                                                      0.0,
                                                      0.0,
                                                      loadedPatch.error);
        writeTextFile (reportFile, report);
        statusLabel.setText ("c3 save_work proof failed: " + juce::String (loadedPatch.error), juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    auto activeSession = makeGraphSession (loadedPatch.document.graph);
    const auto moveResult = moveNode (activeSession, "library_loud1", 13.0, 7.0);
    if (! moveResult.ok)
    {
        const SaveLogLoadResult emptySaveLog;
        const auto report = makeC3SaveWorkReportJson (false,
                                                      workManifestFile.getFullPathName().toStdString(),
                                                      savedPatchFile.getFullPathName().toStdString(),
                                                      {},
                                                      {},
                                                      {},
                                                      emptySaveLog,
                                                      activeSession,
                                                      false,
                                                      false,
                                                      false,
                                                      0.0,
                                                      0.0,
                                                      moveResult.message);
        writeTextFile (reportFile, report);
        statusLabel.setText ("c3 save_work proof failed: " + juce::String (moveResult.message), juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    const auto saveResult = saveWork (activeSession, workManifestFile.getFullPathName().toStdString());
    const auto reloadedPatch = loadPatchDocument (savedPatchFile.getFullPathName().toStdString());
    const auto saveLog = loadSaveLog (saveResult.saveLogPath);
    auto reloadedSession = reloadedPatch.ok ? makeGraphSession (reloadedPatch.document.graph) : makeGraphSession (GraphContract {});

    CompoundPatchLoadResult loadedCompound;
    for (const auto& candidate : repoCandidatePaths ("fixtures/compounds/loudness.compound.json"))
    {
        const auto loaded = loadCompoundPatchSpec (candidate);
        if (loaded.ok)
        {
            loadedCompound = loaded;
            break;
        }

        lastError = loaded.error;
    }

    const auto relayoutGraph = loadedCompound.ok
        ? makeCompoundPatchInteractionGraph (loadedCompound.spec, "library_loud1", reloadedSession.graph)
        : GraphContract {};
    const auto* monoMix = findEditorNode (relayoutGraph, "library_loud1/mono_mix");
    const auto monoMixX = monoMix == nullptr ? 0.0 : monoMix->position.x;
    const auto monoMixY = monoMix == nullptr ? 0.0 : monoMix->position.y;
    const auto publicInputEdge = hasEdgeId (reloadedSession.graph, "edge.live_audio.channels.library_loud1.audio.in");
    const auto publicOutputEdge = hasEdgeId (reloadedSession.graph, "edge.library_loud1.out.midi_loudness.value");
    const auto monoMixLayout = monoMix != nullptr && monoMixX == 358.0 && monoMixY == 146.0;
    const auto graphCountsMatch = reloadedSession.graph.editorGraph.edges.size()
                                  == reloadedSession.graph.runtimeGraph.edges.size();
    const auto commandLogStatus = activeSession.commandLog.empty() ? std::string {} : activeSession.commandLog.back();
    const auto saveLogStatus = saveLog.entries.empty() ? std::string {} : saveLog.entries.back().status;
    const auto saveLogCommitStatus = saveLog.entries.empty() ? std::string {} : saveLog.entries.back().commitStatus;
    const auto ok = saveResult.ok
                    && reloadedPatch.ok
                    && saveLog.ok
                    && commandLogStatus == "save_work:save-ok commit-pending"
                    && saveLogStatus == "save-ok commit-pending"
                    && saveLogCommitStatus == "not-started"
                    && publicInputEdge
                    && publicOutputEdge
                    && monoMixLayout
                    && graphCountsMatch;
    const auto error = ok ? std::string {}
                          : ! saveResult.ok ? saveResult.error
                          : ! reloadedPatch.ok ? reloadedPatch.error
                          : ! saveLog.ok ? saveLog.error
                          : ! loadedCompound.ok ? lastError
                          : "C3 save_work proof did not match expected command/storage evidence";

    const auto report = makeC3SaveWorkReportJson (ok,
                                                  workManifestFile.getFullPathName().toStdString(),
                                                  savedPatchFile.getFullPathName().toStdString(),
                                                  saveResult.saveLogPath,
                                                  saveResult.status,
                                                  commandLogStatus,
                                                  saveLog,
                                                  reloadedSession,
                                                  publicInputEdge,
                                                  publicOutputEdge,
                                                  monoMixLayout,
                                                  monoMixX,
                                                  monoMixY,
                                                  error);

    if (! writeTextFile (reportFile, report))
    {
        statusLabel.setText ("c3 save_work proof failed: could not write " + reportFile.getFullPathName(),
                             juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    statusLabel.setText ((ok ? "c3 save_work proof dumped: " : "c3 save_work proof mismatch: ")
                             + directory.getFullPathName(),
                         juce::dontSendNotification);

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

void MainComponent::dumpC4AIWorkerSaveWorkProof()
{
    const auto directory = c4AIWorkerSaveWorkProofDumpDirectory();
    const auto workDirectory = directory.getChildFile ("work");
    const auto patchDirectory = workDirectory.getChildFile ("patches");
    const auto reportFile = directory.getChildFile ("ai_worker_save_work_report.json");
    const auto workManifestFile = workDirectory.getChildFile ("myworld.work.json");
    const auto savedPatchFile = patchDirectory.getChildFile ("main.patch.json");

    AIWorkerCommandRequest request;
    request.commandId = "c4.1-save-work";
    request.workerId = "ai-worker-proof";
    request.operation = "save_work";
    request.intent = "Persist dirty C2 compound work through the shared save_work command path";
    request.workManifestPath = workManifestFile.getFullPathName().toStdString();

    const auto allowedOperations = allowedAIWorkerOperations();

    if (directory.exists() && ! directory.deleteRecursively())
    {
        const SaveLogLoadResult emptySaveLog;
        const AIWorkerCommandResult emptyResult;
        const auto report = makeC4AIWorkerSaveWorkReportJson (false,
                                                              request,
                                                              emptyResult,
                                                              allowedOperations,
                                                              emptySaveLog,
                                                              makeGraphSession (GraphContract {}),
                                                              false,
                                                              false,
                                                              false,
                                                              0.0,
                                                              0.0,
                                                              {},
                                                              "could not clear " + directory.getFullPathName().toStdString());
        writeTextFile (reportFile, report);
        statusLabel.setText ("c4 AI worker proof failed: could not clear " + directory.getFullPathName(),
                             juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    if (! patchDirectory.createDirectory())
    {
        const SaveLogLoadResult emptySaveLog;
        const AIWorkerCommandResult emptyResult;
        const auto report = makeC4AIWorkerSaveWorkReportJson (false,
                                                              request,
                                                              emptyResult,
                                                              allowedOperations,
                                                              emptySaveLog,
                                                              makeGraphSession (GraphContract {}),
                                                              false,
                                                              false,
                                                              false,
                                                              0.0,
                                                              0.0,
                                                              {},
                                                              "could not create " + patchDirectory.getFullPathName().toStdString());
        writeTextFile (reportFile, report);
        statusLabel.setText ("c4 AI worker proof failed: could not create " + patchDirectory.getFullPathName(),
                             juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    std::string lastError;
    bool copiedFixture = false;
    for (const auto& candidate : repoCandidatePaths ("fixtures/storage/c2-compound-work/myworld.work.json"))
    {
        const auto sourceManifest = juce::File (candidate);
        const auto sourcePatch = sourceManifest.getParentDirectory().getChildFile ("patches").getChildFile ("main.patch.json");
        copiedFixture = copyTextFile (sourceManifest, workManifestFile) && copyTextFile (sourcePatch, savedPatchFile);
        if (copiedFixture)
            break;

        lastError = "could not copy C2 work fixture from " + candidate;
    }

    if (! copiedFixture)
    {
        const SaveLogLoadResult emptySaveLog;
        const AIWorkerCommandResult emptyResult;
        const auto report = makeC4AIWorkerSaveWorkReportJson (false,
                                                              request,
                                                              emptyResult,
                                                              allowedOperations,
                                                              emptySaveLog,
                                                              makeGraphSession (GraphContract {}),
                                                              false,
                                                              false,
                                                              false,
                                                              0.0,
                                                              0.0,
                                                              {},
                                                              lastError.empty() ? "could not copy C4 work fixture" : lastError);
        writeTextFile (reportFile, report);
        statusLabel.setText ("c4 AI worker proof failed: " + juce::String (lastError), juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    const auto loadedPatch = loadMainPatchDocumentForWork (workManifestFile.getFullPathName().toStdString());
    if (! loadedPatch.ok)
    {
        const SaveLogLoadResult emptySaveLog;
        const AIWorkerCommandResult emptyResult;
        const auto report = makeC4AIWorkerSaveWorkReportJson (false,
                                                              request,
                                                              emptyResult,
                                                              allowedOperations,
                                                              emptySaveLog,
                                                              makeGraphSession (GraphContract {}),
                                                              false,
                                                              false,
                                                              false,
                                                              0.0,
                                                              0.0,
                                                              {},
                                                              loadedPatch.error);
        writeTextFile (reportFile, report);
        statusLabel.setText ("c4 AI worker proof failed: " + juce::String (loadedPatch.error), juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    auto activeSession = makeGraphSession (loadedPatch.document.graph);
    const auto moveResult = moveNode (activeSession, "library_loud1", 13.0, 7.0);
    if (! moveResult.ok)
    {
        const SaveLogLoadResult emptySaveLog;
        const AIWorkerCommandResult emptyResult;
        const auto report = makeC4AIWorkerSaveWorkReportJson (false,
                                                              request,
                                                              emptyResult,
                                                              allowedOperations,
                                                              emptySaveLog,
                                                              activeSession,
                                                              false,
                                                              false,
                                                              false,
                                                              0.0,
                                                              0.0,
                                                              {},
                                                              moveResult.message);
        writeTextFile (reportFile, report);
        statusLabel.setText ("c4 AI worker proof failed: " + juce::String (moveResult.message), juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    const auto result = executeAIWorkerCommand (activeSession, request);
    const auto reloadedPatch = loadPatchDocument (result.evidence.patchPath);
    const auto saveLog = loadSaveLog (result.evidence.saveLogPath);
    auto reloadedSession = reloadedPatch.ok ? makeGraphSession (reloadedPatch.document.graph) : makeGraphSession (GraphContract {});

    CompoundPatchLoadResult loadedCompound;
    for (const auto& candidate : repoCandidatePaths ("fixtures/compounds/loudness.compound.json"))
    {
        const auto loaded = loadCompoundPatchSpec (candidate);
        if (loaded.ok)
        {
            loadedCompound = loaded;
            break;
        }

        lastError = loaded.error;
    }

    const auto relayoutGraph = loadedCompound.ok
        ? makeCompoundPatchInteractionGraph (loadedCompound.spec, "library_loud1", reloadedSession.graph)
        : GraphContract {};
    const auto* monoMix = findEditorNode (relayoutGraph, "library_loud1/mono_mix");
    const auto monoMixX = monoMix == nullptr ? 0.0 : monoMix->position.x;
    const auto monoMixY = monoMix == nullptr ? 0.0 : monoMix->position.y;
    const auto publicInputEdge = hasEdgeId (reloadedSession.graph, "edge.live_audio.channels.library_loud1.audio.in");
    const auto publicOutputEdge = hasEdgeId (reloadedSession.graph, "edge.library_loud1.out.midi_loudness.value");
    const auto monoMixLayout = monoMix != nullptr && monoMixX == 358.0 && monoMixY == 146.0;
    const auto graphCountsMatch = reloadedSession.graph.editorGraph.edges.size()
                                  == reloadedSession.graph.runtimeGraph.edges.size();
    const auto aiCommandLogStatus = activeSession.commandLog.empty() ? std::string {} : activeSession.commandLog.back();
    const auto saveLogStatus = saveLog.entries.empty() ? std::string {} : saveLog.entries.back().status;
    const auto saveWorkAllowed = std::find (allowedOperations.begin(), allowedOperations.end(), "save_work")
                                 != allowedOperations.end();
    const auto collaborationLogOk = activeSession.collaborationLog.size() >= 2
                                    && activeSession.collaborationLog.front().status == "requested"
                                    && activeSession.collaborationLog.back().status == "save-ok commit-pending"
                                    && activeSession.collaborationLog.back().proofEvidence.find ("patchReloaded=true")
                                        != std::string::npos
                                    && activeSession.collaborationLog.back().proofEvidence.find ("saveLogStatus=save-ok commit-pending")
                                        != std::string::npos;
    const auto ok = result.ok
                    && result.operation == "save_work"
                    && result.status == "save-ok commit-pending"
                    && result.evidence.storageCommandLogStatus == "save_work:save-ok commit-pending"
                    && result.evidence.saveLogStatus == "save-ok commit-pending"
                    && ! result.evidence.usesInteractionState
                    && aiCommandLogStatus == "ai_worker:save_work:save-ok commit-pending"
                    && saveWorkAllowed
                    && reloadedPatch.ok
                    && saveLog.ok
                    && saveLogStatus == "save-ok commit-pending"
                    && collaborationLogOk
                    && publicInputEdge
                    && publicOutputEdge
                    && monoMixLayout
                    && graphCountsMatch;
    const auto error = ok ? std::string {}
                          : ! result.ok ? result.error
                          : ! reloadedPatch.ok ? reloadedPatch.error
                          : ! saveLog.ok ? saveLog.error
                          : ! loadedCompound.ok ? lastError
                          : "C4 AI worker save_work proof did not match expected command/collaboration evidence";

    const auto report = makeC4AIWorkerSaveWorkReportJson (ok,
                                                          request,
                                                          result,
                                                          allowedOperations,
                                                          saveLog,
                                                          activeSession,
                                                          publicInputEdge,
                                                          publicOutputEdge,
                                                          monoMixLayout,
                                                          monoMixX,
                                                          monoMixY,
                                                          aiCommandLogStatus,
                                                          error);

    if (! writeTextFile (reportFile, report))
    {
        statusLabel.setText ("c4 AI worker proof failed: could not write " + reportFile.getFullPathName(),
                             juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    statusLabel.setText ((ok ? "c4 AI worker proof dumped: " : "c4 AI worker proof mismatch: ")
                             + directory.getFullPathName(),
                         juce::dontSendNotification);

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

CommandResult MainComponent::saveActiveWork (GraphSession& session)
{
    const auto manifestFile = activeWorkManifestFile();
    std::string error;

    if (manifestFile == defaultActiveWorkManifestFile()
        && ! prepareDefaultActiveWorkProject (manifestFile, error))
    {
        statusLabel.setText ("save_work failed: " + juce::String (error), juce::dontSendNotification);
        return { false, error };
    }

    const auto result = saveWork (session, manifestFile.getFullPathName().toStdString());
    const auto message = result.ok ? result.status : result.error;
    statusLabel.setText ((result.ok ? "save_work: " : "save_work failed: ") + juce::String (message),
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
    midiStatusLabel.setText (midiStatus, juce::dontSendNotification);

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
}
