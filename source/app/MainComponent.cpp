#include "MainComponent.h"

#include "A1AudioProofRunner.h"
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
#include "PVB1AnalyzerEnvironmentProofRunner.h"
#include "PVDetectorProofRunner.h"
#include "ProofReports.h"
#include "RuntimeRegistry.h"
#include "StorageCommand.h"
#include "StorageContract.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <utility>
#include <vector>

namespace myworld
{
namespace
{
juce::Font monoFont (float height)
{
    return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), height, juce::Font::plain));
}

bool looksLikeProjectDirectory (const juce::File& directory)
{
    return directory.getChildFile ("CMakeLists.txt").existsAsFile()
           && directory.getChildFile ("source").isDirectory()
           && directory.getChildFile ("fixtures").isDirectory();
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

    const auto workingDirectory = juce::File::getCurrentWorkingDirectory();

    if (looksLikeProjectDirectory (workingDirectory))
        return workingDirectory;

    return juce::File::getSpecialLocation (juce::File::userDesktopDirectory)
        .getChildFile (juce::String::fromUTF8 ("\xe6\x88\x91\xe7\x9a\x84\xe4\xb8\x96\xe7\x95\x8c"));
}

juce::File proofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile ("v1-shader-proof");
}

juce::File audioProofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile (a1AudioProofDirectoryName());
}

juce::File c2StorageProofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile (c2StorageProofDirectoryName());
}

juce::File c3SaveWorkProofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile (c3SaveWorkProofDirectoryName());
}

juce::File c4AIWorkerSaveWorkProofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile (c4AIWorkerSaveWorkProofDirectoryName());
}

juce::File c5ModulePublishProofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile (
        c5ModulePublishProofDirectoryName (C5ModulePublishProofKind::modulePublish));
}

juce::File c5AIWorkerModulePublishProofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile (
        c5ModulePublishProofDirectoryName (C5ModulePublishProofKind::aiWorkerModulePublish));
}

juce::File c5VisibleModulePublishDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile ("c5-visible-module-publish");
}

juce::File c5VisibleModulePublishProofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile (
        c5ModulePublishProofDirectoryName (C5ModulePublishProofKind::visibleModulePublish));
}

juce::File c6AnalyzerFamilyProofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile (c6AnalyzerFamilyProofDirectoryName());
}

juce::File c6AIRepairLoopProofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile (c6AIRepairLoopProofDirectoryName());
}

juce::File pvDetectorProofDumpDirectory (PVDetectorProofKind kind)
{
    return projectDirectory().getChildFile ("debug").getChildFile (pvDetectorProofDirectoryName (kind));
}

juce::File pvB1AnalyzerEnvironmentProofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile (pvB1AnalyzerEnvironmentProofDirectoryName());
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

std::vector<std::filesystem::path> proofCandidateRoots()
{
    const auto executableDir = juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory();

    return {
        projectDirectory().getFullPathName().toStdString(),
        juce::File::getCurrentWorkingDirectory().getFullPathName().toStdString(),
        parentDirectory (executableDir, 5).getFullPathName().toStdString()
    };
}

bool copyTextFile (const juce::File& source, const juce::File& target)
{
    if (! target.getParentDirectory().createDirectory())
        return false;

    if (target.existsAsFile() && ! target.deleteFile())
        return false;

    return source.copyFileTo (target);
}

bool copyFirstRepoCandidate (const juce::String& relativePath, const juce::File& target, std::string& error)
{
    for (const auto& candidate : repoCandidatePaths (relativePath))
    {
        const auto source = juce::File (candidate);
        if (copyTextFile (source, target))
            return true;

        error = "could not copy " + relativePath.toStdString() + " from " + candidate;
    }

    if (error.empty())
        error = "could not copy " + relativePath.toStdString();

    return false;
}

bool prepareDefaultActiveWorkProject (const juce::File& workManifestFile, std::string& error)
{
    const auto patchFile = workManifestFile.getParentDirectory().getChildFile ("patches").getChildFile ("main.patch.json");
    const auto debugDirectory = workManifestFile.getParentDirectory().getParentDirectory();
    const auto moduleLibraryFile = debugDirectory.getChildFile ("module-libraries").getChildFile ("default.module-library.json");
    const auto moduleManifestFile = debugDirectory.getChildFile ("module-libraries")
                                          .getChildFile ("modules")
                                          .getChildFile ("loudness")
                                          .getChildFile ("module.json");

    if (! workManifestFile.existsAsFile()
        && ! copyFirstRepoCandidate ("fixtures/storage/c2-compound-work/myworld.work.json", workManifestFile, error))
    {
        return false;
    }

    if (! patchFile.existsAsFile()
        && ! copyFirstRepoCandidate ("fixtures/storage/c2-compound-work/patches/main.patch.json", patchFile, error))
    {
        return false;
    }

    if (! moduleLibraryFile.existsAsFile()
        && ! copyFirstRepoCandidate ("fixtures/module-libraries/default.module-library.json", moduleLibraryFile, error))
    {
        return false;
    }

    if (! moduleManifestFile.existsAsFile()
        && ! copyFirstRepoCandidate ("fixtures/modules/loudness/module.json", moduleManifestFile, error))
    {
        return false;
    }

    return true;
}

std::string safeIdentifier (const std::string& text)
{
    std::string result;
    result.reserve (text.size());

    for (const auto character : text)
    {
        const auto byte = static_cast<unsigned char> (character);
        if (std::isalnum (byte) != 0 || character == '-' || character == '_')
            result.push_back (static_cast<char> (std::tolower (byte)));
        else
            result.push_back ('-');
    }

    return result.empty() ? "module" : result;
}

}

MainComponent::MainComponent (bool dumpProofOnStart,
                              bool dumpAudioProofOnStart,
                              bool dumpC2StorageProofOnStart,
                              bool dumpC3SaveWorkProofOnStart,
                              bool dumpC4AIWorkerSaveWorkProofOnStart,
                              bool dumpC5ModulePublishProofOnStart,
                              bool dumpC5AIWorkerModulePublishProofOnStart,
                              bool dumpC5VisibleModulePublishProofOnStart,
                              bool dumpC6AnalyzerFamilyProofOnStart,
                              bool dumpC6AIRepairLoopProofOnStart,
                              bool dumpPVAttackDetectorProofOnStart,
                              bool dumpPVDensityDetectorProofOnStart,
                              bool dumpPVSilenceDetectorProofOnStart,
                              bool dumpPVSustainDetectorProofOnStart,
                              bool dumpPVResidueDetectorProofOnStart,
                              bool dumpPVAggregatePressureProofOnStart,
                              bool dumpPVB1AnalyzerEnvironmentProofOnStart,
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

    if (dumpC5ModulePublishProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpC5ModulePublishProof();
        });
    }

    if (dumpC5AIWorkerModulePublishProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpC5AIWorkerModulePublishProof();
        });
    }

    if (dumpC5VisibleModulePublishProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpC5VisibleModulePublishProof();
        });
    }

    if (dumpC6AnalyzerFamilyProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpC6AnalyzerFamilyProof();
        });
    }

    if (dumpC6AIRepairLoopProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpC6AIRepairLoopProof();
        });
    }

    if (dumpPVAttackDetectorProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpPVAttackDetectorProof();
        });
    }

    if (dumpPVDensityDetectorProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpPVDensityDetectorProof();
        });
    }

    if (dumpPVSilenceDetectorProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpPVSilenceDetectorProof();
        });
    }

    if (dumpPVSustainDetectorProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpPVSustainDetectorProof();
        });
    }

    if (dumpPVResidueDetectorProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpPVResidueDetectorProof();
        });
    }

    if (dumpPVAggregatePressureProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpPVAggregatePressureProof();
        });
    }

    if (dumpPVB1AnalyzerEnvironmentProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpPVB1AnalyzerEnvironmentProof();
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

    A1AudioProofRunRequest request;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();
    request.snapshot = audioInputAnalyzer.getSnapshot();
    request.sampleRate = audioInputAnalyzer.getSampleRate();
    request.bufferSize = audioInputAnalyzer.getBufferSize();
    request.preferences = performancePreferences;

    const auto result = runA1AudioProof (request);
    const auto displayNameString = juce::String (a1AudioProofDisplayName());

    if (result.status == "failed")
        statusLabel.setText (displayNameString + " proof failed: " + juce::String (result.error),
                             juce::dontSendNotification);
    else
        statusLabel.setText (displayNameString + " proof " + juce::String (result.status) + ": "
                                 + directory.getFullPathName(),
                             juce::dontSendNotification);

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

void MainComponent::dumpC2StorageProof()
{
    const auto directory = c2StorageProofDumpDirectory();

    C2StorageProofRunRequest request;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runC2StorageProof (request);
    const auto displayNameString = juce::String (c2StorageProofDisplayName());

    if (result.status == "failed")
        statusLabel.setText (displayNameString + " proof failed: " + juce::String (result.error),
                             juce::dontSendNotification);
    else
        statusLabel.setText (displayNameString + " proof " + juce::String (result.status) + ": "
                                 + directory.getFullPathName(),
                             juce::dontSendNotification);

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

void MainComponent::dumpC3SaveWorkProof()
{
    const auto directory = c3SaveWorkProofDumpDirectory();

    C3SaveWorkProofRunRequest request;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runC3SaveWorkProof (request);
    const auto displayNameString = juce::String (c3SaveWorkProofDisplayName());

    if (result.status == "failed")
        statusLabel.setText (displayNameString + " proof failed: " + juce::String (result.error),
                             juce::dontSendNotification);
    else
        statusLabel.setText (displayNameString + " proof " + juce::String (result.status) + ": "
                                 + directory.getFullPathName(),
                             juce::dontSendNotification);

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

void MainComponent::dumpC4AIWorkerSaveWorkProof()
{
    const auto directory = c4AIWorkerSaveWorkProofDumpDirectory();

    C4AIWorkerSaveWorkProofRunRequest request;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runC4AIWorkerSaveWorkProof (request);
    const auto displayNameString = juce::String (c4AIWorkerSaveWorkProofDisplayName());

    if (result.status == "failed")
        statusLabel.setText (displayNameString + " proof failed: " + juce::String (result.error),
                             juce::dontSendNotification);
    else
        statusLabel.setText (displayNameString + " proof " + juce::String (result.status) + ": "
                                 + directory.getFullPathName(),
                             juce::dontSendNotification);

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

void MainComponent::dumpC5ModulePublishProof()
{
    const auto directory = c5ModulePublishProofDumpDirectory();

    C5ModulePublishProofRunRequest request;
    request.kind = C5ModulePublishProofKind::modulePublish;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runC5ModulePublishProof (request);
    const auto displayNameString = juce::String (c5ModulePublishProofDisplayName (request.kind));

    if (result.status == "failed")
        statusLabel.setText (displayNameString + " proof failed: " + juce::String (result.error),
                             juce::dontSendNotification);
    else
        statusLabel.setText (displayNameString + " proof " + juce::String (result.status) + ": "
                                 + directory.getFullPathName(),
                             juce::dontSendNotification);

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

void MainComponent::dumpC5AIWorkerModulePublishProof()
{
    const auto directory = c5AIWorkerModulePublishProofDumpDirectory();

    C5ModulePublishProofRunRequest request;
    request.kind = C5ModulePublishProofKind::aiWorkerModulePublish;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runC5ModulePublishProof (request);
    const auto displayNameString = juce::String (c5ModulePublishProofDisplayName (request.kind));

    if (result.status == "failed")
        statusLabel.setText (displayNameString + " proof failed: " + juce::String (result.error),
                             juce::dontSendNotification);
    else
        statusLabel.setText (displayNameString + " proof " + juce::String (result.status) + ": "
                                 + directory.getFullPathName(),
                             juce::dontSendNotification);

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

void MainComponent::dumpC5VisibleModulePublishProof()
{
    const auto proofDirectory = c5VisibleModulePublishProofDumpDirectory();

    C5ModulePublishProofRunRequest request;
    request.kind = C5ModulePublishProofKind::visibleModulePublish;
    request.outputDirectory = proofDirectory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runC5ModulePublishProof (request);
    const auto displayNameString = juce::String (c5ModulePublishProofDisplayName (request.kind));

    if (result.status == "failed")
        statusLabel.setText (displayNameString + " proof failed: " + juce::String (result.error),
                             juce::dontSendNotification);
    else
        statusLabel.setText (displayNameString + " proof " + juce::String (result.status) + ": "
                                 + proofDirectory.getFullPathName(),
                             juce::dontSendNotification);

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

void MainComponent::dumpC6AnalyzerFamilyProof()
{
    const auto directory = c6AnalyzerFamilyProofDumpDirectory();

    C6AnalyzerFamilyProofRunRequest request;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runC6AnalyzerFamilyProof (request);
    const auto displayNameString = juce::String (c6AnalyzerFamilyProofDisplayName());

    if (result.status == "failed")
        statusLabel.setText (displayNameString + " proof failed: " + juce::String (result.error),
                             juce::dontSendNotification);
    else
        statusLabel.setText (displayNameString + " proof " + juce::String (result.status) + ": "
                                 + directory.getFullPathName(),
                             juce::dontSendNotification);

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

void MainComponent::dumpC6AIRepairLoopProof()
{
    const auto directory = c6AIRepairLoopProofDumpDirectory();

    C6AIRepairLoopProofRunRequest request;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runC6AIRepairLoopProof (request);
    const auto displayNameString = juce::String (c6AIRepairLoopProofDisplayName());

    if (result.status == "failed")
        statusLabel.setText (displayNameString + " proof failed: " + juce::String (result.error),
                             juce::dontSendNotification);
    else
        statusLabel.setText (displayNameString + " proof " + juce::String (result.status) + ": "
                                 + directory.getFullPathName(),
                             juce::dontSendNotification);

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
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
    const auto directory = pvDetectorProofDumpDirectory (kind);

    PVDetectorProofRunRequest request;
    request.kind = kind;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runPVDetectorProof (request);
    const auto displayName = juce::String (pvDetectorProofDisplayName (kind));

    if (result.status == "failed")
    {
        statusLabel.setText (displayName + " proof failed: " + juce::String (result.error),
                             juce::dontSendNotification);
    }
    else
    {
        statusLabel.setText (displayName + " proof " + juce::String (result.status) + ": "
                                 + directory.getFullPathName(),
                             juce::dontSendNotification);
    }

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

void MainComponent::dumpPVB1AnalyzerEnvironmentProof()
{
    const auto directory = pvB1AnalyzerEnvironmentProofDumpDirectory();

    PVB1AnalyzerEnvironmentProofRunRequest request;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runPVB1AnalyzerEnvironmentProof (request);
    const auto displayNameString = juce::String (pvB1AnalyzerEnvironmentProofDisplayName());

    if (result.status == "failed")
        statusLabel.setText (displayNameString + " proof failed: " + juce::String (result.error),
                             juce::dontSendNotification);
    else
        statusLabel.setText (displayNameString + " proof " + juce::String (result.status) + ": "
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

PublishModuleResult MainComponent::publishSelectedModuleResult (GraphSession& session, const std::string& sourceNodeId)
{
    const auto manifestFile = activeWorkManifestFile();
    std::string error;
    std::string workManifestPath;

    if (manifestFile == defaultActiveWorkManifestFile())
    {
        for (const auto& candidate : repoCandidatePaths ("fixtures/storage/c2-compound-work/myworld.work.json"))
        {
            const auto work = loadWorkProjectManifest (candidate);
            if (work.ok)
            {
                workManifestPath = candidate;
                break;
            }

            error = work.error;
        }
    }
    else
    {
        workManifestPath = manifestFile.getFullPathName().toStdString();
    }

    if (workManifestPath.empty())
    {
        return { false,
                 "publish_module",
                 "validation-failed",
                 sourceNodeId,
                 {},
                 {},
                 {},
                 {},
                 {},
                 {},
                 error.empty() ? "could not resolve publish source work manifest" : error };
    }

    const auto safeNodeId = safeIdentifier (sourceNodeId);
    const auto publishDirectory = c5VisibleModulePublishDirectory();

    PublishModuleRequest request;
    request.workManifestPath = workManifestPath;
    request.sourceNodeId = sourceNodeId;
    request.moduleId = "module.visible-" + safeNodeId;
    request.moduleTitle = "Visible " + sourceNodeId;
    request.nodeType = "compound.visible-" + safeNodeId;
    request.packageDirectory = publishDirectory.getChildFile ("modules")
                                   .getChildFile (safeNodeId)
                                   .getFullPathName()
                                   .toStdString();
    request.targetLibraryPath = publishDirectory.getChildFile ("module-libraries")
                                    .getChildFile ("visible.module-library.json")
                                    .getFullPathName()
                                    .toStdString();
    request.overwriteExisting = true;

    return publishModule (session, request);
}

CommandResult MainComponent::publishSelectedModule (GraphSession& session, const std::string& sourceNodeId)
{
    const auto result = publishSelectedModuleResult (session, sourceNodeId);
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
