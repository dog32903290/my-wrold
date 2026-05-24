#include "C4AIWorkerSaveWorkProofRunner.h"

#include "AIWorkerCommand.h"
#include "CompoundPatch.h"
#include "GraphEndpoint.h"
#include "InteractionContract.h"
#include "ProofReports.h"
#include "StorageContract.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <system_error>

namespace myworld
{
namespace
{
constexpr const char* displayName = "C4 AI worker save_work";
constexpr const char* directoryName = "c4-ai-worker-save-work-proof";
constexpr const char* reportFileName = "ai_worker_save_work_report.json";
constexpr const char* workFixturePath = "fixtures/storage/c2-compound-work/myworld.work.json";
constexpr const char* patchFixturePath = "fixtures/storage/c2-compound-work/patches/main.patch.json";
constexpr const char* loudnessCompoundFixturePath = "fixtures/compounds/loudness.compound.json";
constexpr const char* proofWorkerId = "ai-worker-proof";
constexpr const char* loudnessCompoundNodeId = "library_loud1";

bool nearlyEqual (double lhs, double rhs)
{
    return std::abs (lhs - rhs) < 0.000001;
}

bool hasEdgeId (const GraphContract& graph, const std::string& edgeId)
{
    return std::any_of (graph.editorGraph.edges.begin(),
                        graph.editorGraph.edges.end(),
                        [&edgeId] (const auto& edge) {
                            return edge.id == edgeId;
                        });
}

std::string writeTextFile (const std::filesystem::path& path, const std::string& text)
{
    std::error_code error;
    std::filesystem::create_directories (path.parent_path(), error);
    if (error)
        return "could not create " + path.parent_path().string() + ": " + error.message();

    std::ofstream output (path, std::ios::binary);
    if (! output)
        return "could not write " + path.string();

    output << text;
    if (! output)
        return "could not write " + path.string();

    return {};
}

std::string clearDirectoryIfExists (const std::filesystem::path& directory)
{
    std::error_code error;
    if (std::filesystem::exists (directory, error))
    {
        std::filesystem::remove_all (directory, error);
        if (error)
            return "could not clear " + directory.string() + ": " + error.message();
    }

    return {};
}

std::string createDirectoryIfMissing (const std::filesystem::path& directory)
{
    std::error_code error;
    std::filesystem::create_directories (directory, error);
    if (error)
        return "could not create " + directory.string() + ": " + error.message();

    return {};
}

std::vector<std::filesystem::path> candidatePaths (const std::vector<std::filesystem::path>& roots,
                                                   const char* relativePath)
{
    std::vector<std::filesystem::path> paths;

    for (const auto& root : roots)
    {
        if (! root.empty())
            paths.push_back (root / relativePath);
    }

    paths.push_back (std::filesystem::current_path() / relativePath);
    paths.push_back (std::filesystem::path (relativePath));

    std::vector<std::filesystem::path> uniquePaths;
    for (const auto& path : paths)
    {
        if (std::find (uniquePaths.begin(), uniquePaths.end(), path) == uniquePaths.end())
            uniquePaths.push_back (path);
    }

    return uniquePaths;
}

bool copyFirstCandidate (const std::vector<std::filesystem::path>& roots,
                         const char* relativePath,
                         const std::filesystem::path& target,
                         std::string& error)
{
    std::error_code createError;
    std::filesystem::create_directories (target.parent_path(), createError);
    if (createError)
    {
        error = "could not create " + target.parent_path().string() + ": " + createError.message();
        return false;
    }

    for (const auto& candidate : candidatePaths (roots, relativePath))
    {
        std::error_code copyError;
        std::filesystem::copy_file (candidate, target, std::filesystem::copy_options::overwrite_existing, copyError);
        if (! copyError)
            return true;

        error = "could not copy " + std::string (relativePath) + " from " + candidate.string();
    }

    if (error.empty())
        error = "could not copy " + std::string (relativePath);

    return false;
}

AIWorkerCommandRequest makeMoveNodeProofRequest()
{
    AIWorkerCommandRequest request;
    request.commandId = "c4.3-move-node";
    request.workerId = proofWorkerId;
    request.operation = "move_node";
    request.intent = "Move the loaded loudness compound through the shared interaction command path";
    request.nodeId = loudnessCompoundNodeId;
    request.deltaX = 13.0;
    request.deltaY = 7.0;
    return request;
}

AIWorkerCommandRequest makeSaveWorkProofRequest (const std::filesystem::path& workManifestFile)
{
    AIWorkerCommandRequest request;
    request.commandId = "c4.3-save-work";
    request.workerId = proofWorkerId;
    request.operation = "save_work";
    request.intent = "Persist AI-mutated C2 compound work through the shared save_work command path";
    request.workManifestPath = workManifestFile.string();
    return request;
}

C4AIWorkerSaveWorkProofRunResult makeInitialResult (const C4AIWorkerSaveWorkProofRunRequest& request)
{
    C4AIWorkerSaveWorkProofRunResult result;
    result.outputDirectory = request.outputDirectory;
    result.reportPath = request.outputDirectory / reportFileName;
    result.artifactPaths = { result.reportPath };
    return result;
}
}

const char* c4AIWorkerSaveWorkProofDisplayName()
{
    return displayName;
}

const char* c4AIWorkerSaveWorkProofDirectoryName()
{
    return directoryName;
}

C4AIWorkerSaveWorkProofRunResult runC4AIWorkerSaveWorkProof (
    const C4AIWorkerSaveWorkProofRunRequest& request)
{
    auto result = makeInitialResult (request);

    const auto workDirectory = request.outputDirectory / "work";
    const auto patchDirectory = workDirectory / "patches";
    const auto workManifestFile = workDirectory / "myworld.work.json";
    const auto savedPatchFile = patchDirectory / "main.patch.json";

    const auto moveRequest = makeMoveNodeProofRequest();
    const auto saveRequest = makeSaveWorkProofRequest (workManifestFile);
    const auto allowedOperations = allowedAIWorkerOperations();
    const AIWorkerCommandResult emptyMoveResult;
    const AIWorkerCommandResult emptySaveResult;
    const SaveLogLoadResult emptySaveLog;

    const auto writeReport = [&] (bool ok,
                                  const AIWorkerCommandResult& moveResult,
                                  const AIWorkerCommandResult& saveResult,
                                  const SaveLogLoadResult& saveLog,
                                  const GraphSession& session,
                                  bool publicInputEdge,
                                  bool publicOutputEdge,
                                  bool monoMixLayout,
                                  double monoMixX,
                                  double monoMixY,
                                  bool savedMovePersisted,
                                  double savedMoveX,
                                  double savedMoveY,
                                  const std::string& aiCommandLogStatus,
                                  const std::string& error)
    {
        return writeTextFile (result.reportPath,
                              makeC4AIWorkerSaveWorkReportJson (ok,
                                                                 moveRequest,
                                                                 moveResult,
                                                                 saveRequest,
                                                                 saveResult,
                                                                 allowedOperations,
                                                                 saveLog,
                                                                 session,
                                                                 publicInputEdge,
                                                                 publicOutputEdge,
                                                                 monoMixLayout,
                                                                 monoMixX,
                                                                 monoMixY,
                                                                 savedMovePersisted,
                                                                 savedMoveX,
                                                                 savedMoveY,
                                                                 aiCommandLogStatus,
                                                                 error));
    };

    const auto fail = [&] (const std::string& message, const GraphSession& reportSession)
    {
        const auto writeError = writeReport (false,
                                             emptyMoveResult,
                                             emptySaveResult,
                                             emptySaveLog,
                                             reportSession,
                                             false,
                                             false,
                                             false,
                                             0.0,
                                             0.0,
                                             false,
                                             0.0,
                                             0.0,
                                             {},
                                             message);
        result.ok = false;
        result.status = "failed";
        result.error = writeError.empty() ? message : writeError;
        return result;
    };

    if (const auto error = clearDirectoryIfExists (request.outputDirectory); ! error.empty())
        return fail (error, makeGraphSession (GraphContract {}));

    if (const auto error = createDirectoryIfMissing (patchDirectory); ! error.empty())
        return fail (error, makeGraphSession (GraphContract {}));

    std::string lastError;
    const auto copiedWorkManifest = copyFirstCandidate (request.candidateRoots,
                                                        workFixturePath,
                                                        workManifestFile,
                                                        lastError);
    const auto copiedPatch = copiedWorkManifest
        && copyFirstCandidate (request.candidateRoots, patchFixturePath, savedPatchFile, lastError);

    if (! copiedPatch)
        return fail (lastError.empty() ? "could not copy C4 work fixture" : lastError,
                     makeGraphSession (GraphContract {}));

    const auto loadedPatch = loadMainPatchDocumentForWork (workManifestFile.string());
    if (! loadedPatch.ok)
        return fail (loadedPatch.error, makeGraphSession (GraphContract {}));

    auto activeSession = makeGraphSession (loadedPatch.document.graph);
    const auto moveResult = executeAIWorkerCommand (activeSession, moveRequest);
    if (! moveResult.ok)
    {
        const auto aiCommandLogStatus = activeSession.commandLog.empty() ? std::string {}
                                                                         : activeSession.commandLog.back();
        if (const auto writeError = writeReport (false,
                                                 moveResult,
                                                 emptySaveResult,
                                                 emptySaveLog,
                                                 activeSession,
                                                 false,
                                                 false,
                                                 false,
                                                 0.0,
                                                 0.0,
                                                 false,
                                                 0.0,
                                                 0.0,
                                                 aiCommandLogStatus,
                                                 moveResult.error);
            ! writeError.empty())
        {
            result.error = writeError;
        }
        else
        {
            result.error = moveResult.error;
        }

        result.ok = false;
        result.status = "failed";
        return result;
    }

    const auto saveResult = executeAIWorkerCommand (activeSession, saveRequest);
    const auto reloadedPatch = loadPatchDocument (saveResult.evidence.patchPath);
    const auto saveLog = loadSaveLog (saveResult.evidence.saveLogPath);
    auto reloadedSession = reloadedPatch.ok ? makeGraphSession (reloadedPatch.document.graph)
                                            : makeGraphSession (GraphContract {});

    CompoundPatchLoadResult loadedCompound;
    for (const auto& candidate : candidatePaths (request.candidateRoots, loudnessCompoundFixturePath))
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
    const auto* savedMovedNode = findEditorNode (reloadedSession.graph, loudnessCompoundNodeId);
    const auto* activeMovedNode = findEditorNode (activeSession.graph, loudnessCompoundNodeId);
    const auto savedMoveX = savedMovedNode == nullptr ? 0.0 : savedMovedNode->position.x;
    const auto savedMoveY = savedMovedNode == nullptr ? 0.0 : savedMovedNode->position.y;
    const auto savedMovePersisted = savedMovedNode != nullptr
                                    && activeMovedNode != nullptr
                                    && nearlyEqual (savedMovedNode->position.x, activeMovedNode->position.x)
                                    && nearlyEqual (savedMovedNode->position.y, activeMovedNode->position.y);
    const auto monoMixLayout = monoMix != nullptr && monoMixX == 358.0 && monoMixY == 146.0;
    const auto graphCountsMatch = reloadedSession.graph.editorGraph.edges.size()
                                  == reloadedSession.graph.runtimeGraph.edges.size();
    const auto aiCommandLogStatus = activeSession.commandLog.empty() ? std::string {}
                                                                     : activeSession.commandLog.back();
    const auto saveLogStatus = saveLog.entries.empty() ? std::string {} : saveLog.entries.back().status;
    const auto saveWorkAllowed = std::find (allowedOperations.begin(), allowedOperations.end(), "save_work")
                                 != allowedOperations.end();
    const auto moveNodeAllowed = std::find (allowedOperations.begin(), allowedOperations.end(), "move_node")
                                 != allowedOperations.end();
    const auto hasMoveProof = std::any_of (activeSession.collaborationLog.begin(),
                                           activeSession.collaborationLog.end(),
                                           [] (const auto& item) {
                                               return item.proofEvidence.find ("graphCommandLogStatus=move_node")
                                                      != std::string::npos;
                                           });
    const auto collaborationLogOk = activeSession.collaborationLog.size() >= 4
                                    && activeSession.collaborationLog.front().operation == "move_node"
                                    && activeSession.collaborationLog.front().status == "requested"
                                    && activeSession.collaborationLog.back().operation == "save_work"
                                    && activeSession.collaborationLog.back().status == "save-ok commit-pending"
                                    && hasMoveProof
                                    && activeSession.collaborationLog.back().proofEvidence.find ("patchReloaded=true")
                                        != std::string::npos
                                    && activeSession.collaborationLog.back().proofEvidence.find ("saveLogStatus=save-ok commit-pending")
                                        != std::string::npos;
    const auto ok = moveResult.ok
                    && moveResult.operation == "move_node"
                    && moveResult.status == "ok"
                    && moveResult.evidence.graphCommandLogStatus == "move_node"
                    && moveResult.evidence.graphMutationApplied
                    && saveResult.ok
                    && saveResult.operation == "save_work"
                    && saveResult.status == "save-ok commit-pending"
                    && saveResult.evidence.storageCommandLogStatus == "save_work:save-ok commit-pending"
                    && saveResult.evidence.saveLogStatus == "save-ok commit-pending"
                    && ! saveResult.evidence.usesInteractionState
                    && aiCommandLogStatus == "ai_worker:save_work:save-ok commit-pending"
                    && saveWorkAllowed
                    && moveNodeAllowed
                    && reloadedPatch.ok
                    && saveLog.ok
                    && saveLogStatus == "save-ok commit-pending"
                    && collaborationLogOk
                    && savedMovePersisted
                    && publicInputEdge
                    && publicOutputEdge
                    && monoMixLayout
                    && graphCountsMatch;
    const auto error = ok ? std::string {}
                          : ! moveResult.ok ? moveResult.error
                          : ! saveResult.ok ? saveResult.error
                          : ! reloadedPatch.ok ? reloadedPatch.error
                          : ! saveLog.ok ? saveLog.error
                          : ! loadedCompound.ok ? lastError
                          : "C4 AI worker save_work proof did not match expected command/collaboration evidence";

    if (const auto writeError = writeReport (ok,
                                             moveResult,
                                             saveResult,
                                             saveLog,
                                             activeSession,
                                             publicInputEdge,
                                             publicOutputEdge,
                                             monoMixLayout,
                                             monoMixX,
                                             monoMixY,
                                             savedMovePersisted,
                                             savedMoveX,
                                             savedMoveY,
                                             aiCommandLogStatus,
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
