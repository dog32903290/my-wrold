#include "C3SaveWorkProofRunner.h"

#include "CompoundPatch.h"
#include "GraphEndpoint.h"
#include "InteractionContract.h"
#include "ProofRunSupport.h"
#include "ProofReports.h"
#include "StorageCommand.h"
#include "StorageContract.h"

#include <algorithm>

namespace myworld
{
namespace
{
constexpr const char* displayName = "C3 save_work";
constexpr const char* directoryName = "c3-save-work-proof";
constexpr const char* reportFileName = "save_work_report.json";
constexpr const char* workFixturePath = "fixtures/storage/c2-compound-work/myworld.work.json";
constexpr const char* patchFixturePath = "fixtures/storage/c2-compound-work/patches/main.patch.json";
constexpr const char* loudnessCompoundFixturePath = "fixtures/compounds/loudness.compound.json";
constexpr const char* loudnessCompoundNodeId = "library_loud1";

bool hasEdgeId (const GraphContract& graph, const std::string& edgeId)
{
    return std::any_of (graph.editorGraph.edges.begin(),
                        graph.editorGraph.edges.end(),
                        [&edgeId] (const auto& edge) {
                            return edge.id == edgeId;
                        });
}

C3SaveWorkProofRunResult makeInitialResult (const C3SaveWorkProofRunRequest& request)
{
    C3SaveWorkProofRunResult result;
    result.outputDirectory = request.outputDirectory;
    result.reportPath = request.outputDirectory / reportFileName;
    result.artifactPaths = { result.reportPath };
    return result;
}
}

const char* c3SaveWorkProofDisplayName()
{
    return displayName;
}

const char* c3SaveWorkProofDirectoryName()
{
    return directoryName;
}

C3SaveWorkProofRunResult runC3SaveWorkProof (const C3SaveWorkProofRunRequest& request)
{
    auto result = makeInitialResult (request);

    const auto workDirectory = request.outputDirectory / "work";
    const auto patchDirectory = workDirectory / "patches";
    const auto workManifestFile = workDirectory / "myworld.work.json";
    const auto savedPatchFile = patchDirectory / "main.patch.json";
    const SaveLogLoadResult emptySaveLog;

    const auto writeReport = [&] (bool ok,
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
        return writeProofTextFile (result.reportPath,
                                   makeC3SaveWorkReportJson (ok,
                                                             workManifestFile.string(),
                                                             savedPatchFile.string(),
                                                             saveLogPath,
                                                             saveStatus,
                                                             commandLogStatus,
                                                             saveLog,
                                                             session,
                                                             publicInputEdge,
                                                             publicOutputEdge,
                                                             monoMixLayout,
                                                             monoMixX,
                                                             monoMixY,
                                                             error));
    };

    const auto fail = [&] (const std::string& message, const GraphSession& reportSession)
    {
        const auto writeError = writeReport (false,
                                             {},
                                             {},
                                             {},
                                             emptySaveLog,
                                             reportSession,
                                             false,
                                             false,
                                             false,
                                             0.0,
                                             0.0,
                                             message);
        result.ok = false;
        result.status = "failed";
        result.error = writeError.empty() ? message : writeError;
        return result;
    };

    if (const auto error = clearProofDirectoryIfExists (request.outputDirectory); ! error.empty())
        return fail (error, makeGraphSession (GraphContract {}));

    if (const auto error = createProofDirectoryIfMissing (patchDirectory); ! error.empty())
        return fail (error, makeGraphSession (GraphContract {}));

    std::string lastError;
    const auto copiedWorkManifest = copyFirstProofCandidate (request.candidateRoots,
                                                             workFixturePath,
                                                             workManifestFile,
                                                             lastError);
    const auto copiedPatch = copiedWorkManifest
        && copyFirstProofCandidate (request.candidateRoots, patchFixturePath, savedPatchFile, lastError);

    if (! copiedPatch)
        return fail (lastError.empty() ? "could not copy C3 work fixture" : lastError,
                     makeGraphSession (GraphContract {}));

    const auto loadedPatch = loadMainPatchDocumentForWork (workManifestFile.string());
    if (! loadedPatch.ok)
        return fail (loadedPatch.error, makeGraphSession (GraphContract {}));

    auto activeSession = makeGraphSession (loadedPatch.document.graph);
    const auto moveResult = moveNode (activeSession, loudnessCompoundNodeId, 13.0, 7.0);
    if (! moveResult.ok)
        return fail (moveResult.message, activeSession);

    const auto saveResult = saveWork (activeSession, workManifestFile.string());
    const auto reloadedPatch = loadPatchDocument (savedPatchFile.string());
    const auto saveLog = loadSaveLog (saveResult.saveLogPath);
    auto reloadedSession = reloadedPatch.ok ? makeGraphSession (reloadedPatch.document.graph)
                                            : makeGraphSession (GraphContract {});

    CompoundPatchLoadResult loadedCompound;
    for (const auto& candidate : proofCandidatePaths (request.candidateRoots, loudnessCompoundFixturePath))
    {
        const auto loaded = loadCompoundPatchSpec (candidate.string());
        if (loaded.ok)
        {
            loadedCompound = loaded;
            break;
        }

        lastError = loaded.error;
    }

    const auto relayoutGraph = loadedCompound.ok
        ? makeCompoundPatchInteractionGraph (loadedCompound.spec, loudnessCompoundNodeId, reloadedSession.graph)
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

    if (const auto writeError = writeReport (ok,
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
        ! writeError.empty())
    {
        result.ok = false;
        result.status = "failed";
        result.error = writeError;
        return result;
    }

    result.ok = ok;
    result.status = ok ? "dumped" : "mismatch";
    result.error = error;
    return result;
}
}
