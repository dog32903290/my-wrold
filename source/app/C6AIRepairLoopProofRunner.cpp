#include "C6AIRepairLoopProofRunner.h"

#include "AIWorkerCommand.h"
#include "GraphEndpoint.h"
#include "GraphContract.h"
#include "ProofReports.h"
#include "ProofRunSupport.h"
#include "StorageContract.h"

namespace myworld
{
namespace
{
constexpr const char* displayName = "C6 AI repair loop";
constexpr const char* directoryName = "c6-ai-repair-loop-proof";
constexpr const char* reportFileName = "ai_repair_loop_report.json";
constexpr const char* workFixturePath = "fixtures/storage/c2-compound-work/myworld.work.json";
constexpr const char* proofWorkerId = "ai-worker-proof";
constexpr const char* repairId = "c6.2-ai-repair-loop";
constexpr const char* repairMissingNodeId = "missing_loudness";
constexpr const char* repairTargetNodeId = "library_loud1";
constexpr double repairDeltaX = 17.0;
constexpr double repairDeltaY = 5.0;
constexpr double unusedRepairDelta = 100.0;

AIWorkerCommandRequest makeMoveNodeRequest (std::string commandId,
                                            std::string intent,
                                            std::string nodeId,
                                            double deltaX,
                                            double deltaY)
{
    AIWorkerCommandRequest request;
    request.commandId = std::move (commandId);
    request.workerId = proofWorkerId;
    request.operation = "move_node";
    request.intent = std::move (intent);
    request.nodeId = std::move (nodeId);
    request.deltaX = deltaX;
    request.deltaY = deltaY;
    return request;
}

AIWorkerRepairPlan makeRepairLoopProofPlan()
{
    const auto failedAttempt = makeMoveNodeRequest ("c6.2-move-missing-node",
                                                    "First repair attempt intentionally targets a missing node",
                                                    repairMissingNodeId,
                                                    repairDeltaX,
                                                    repairDeltaY);
    const auto repairedAttempt = makeMoveNodeRequest ("c6.2-move-library-loudness",
                                                      "Second repair attempt targets the loaded loudness node",
                                                      repairTargetNodeId,
                                                      repairDeltaX,
                                                      repairDeltaY);
    auto unusedAttempt = repairedAttempt;
    unusedAttempt.commandId = "c6.2-unused-attempt";
    unusedAttempt.deltaX = unusedRepairDelta;
    unusedAttempt.deltaY = unusedRepairDelta;

    AIWorkerRepairPlan repairPlan;
    repairPlan.repairId = repairId;
    repairPlan.workerId = proofWorkerId;
    repairPlan.intent = "Repair a failed move_node command by retrying through the shared AI command path";
    repairPlan.maxAttempts = 3;
    repairPlan.attempts = { failedAttempt, repairedAttempt, unusedAttempt };
    return repairPlan;
}

C6AIRepairLoopProofRunResult makeInitialResult (const C6AIRepairLoopProofRunRequest& request)
{
    C6AIRepairLoopProofRunResult result;
    result.outputDirectory = request.outputDirectory;
    result.reportPath = request.outputDirectory / reportFileName;
    result.artifactPaths = { result.reportPath };
    return result;
}
}

const char* c6AIRepairLoopProofDisplayName()
{
    return displayName;
}

const char* c6AIRepairLoopProofDirectoryName()
{
    return directoryName;
}

C6AIRepairLoopProofRunResult runC6AIRepairLoopProof (const C6AIRepairLoopProofRunRequest& request)
{
    auto result = makeInitialResult (request);
    const AIWorkerRepairLoopResult emptyRepairResult;

    const auto writeReport = [&] (bool ok,
                                  const AIWorkerRepairLoopResult& repairResult,
                                  bool graphMutationApplied,
                                  size_t collaborationLogEntries,
                                  double finalNodeX,
                                  double finalNodeY,
                                  const std::string& error)
    {
        return writeProofTextFile (result.reportPath,
                                   makeC6AIRepairLoopReportJson (ok,
                                                                 repairResult,
                                                                 graphMutationApplied,
                                                                 collaborationLogEntries,
                                                                 false,
                                                                 finalNodeX,
                                                                 finalNodeY,
                                                                 error));
    };

    const auto fail = [&] (const AIWorkerRepairLoopResult& repairResult, const std::string& message)
    {
        const auto writeError = writeReport (false, repairResult, false, 0, 0.0, 0.0, message);
        result.ok = false;
        result.status = "failed";
        result.error = writeError.empty() ? message : writeError;
        return result;
    };

    if (const auto error = clearProofDirectoryIfExists (request.outputDirectory); ! error.empty())
        return fail (emptyRepairResult, error);

    if (const auto error = createProofDirectoryIfMissing (request.outputDirectory); ! error.empty())
        return fail (emptyRepairResult, error);

    PatchDocumentLoadResult loadedPatch;
    std::string lastError;
    for (const auto& candidate : proofCandidatePaths (request.candidateRoots, workFixturePath))
    {
        const auto loaded = loadMainPatchDocumentForWork (candidate.string());
        if (loaded.ok)
        {
            loadedPatch = loaded;
            break;
        }

        lastError = loaded.error;
    }

    if (! loadedPatch.ok)
        return fail (emptyRepairResult, lastError.empty() ? "could not load C2 work fixture" : lastError);

    auto session = makeGraphSession (loadedPatch.document.graph);
    const auto repairResult = executeAIWorkerRepairLoop (session, makeRepairLoopProofPlan());
    const auto* finalNode = findEditorNode (session.graph, repairTargetNodeId);
    const auto finalNodeX = finalNode == nullptr ? 0.0 : finalNode->position.x;
    const auto finalNodeY = finalNode == nullptr ? 0.0 : finalNode->position.y;
    const auto graphMutationApplied = ! repairResult.attempts.empty()
        && repairResult.attempts.back().commandResult.evidence.graphMutationApplied;
    const auto firstAttemptFailed = ! repairResult.attempts.empty()
        && repairResult.attempts.front().status == "failed";
    const auto ok = repairResult.ok
                    && repairResult.status == "repaired"
                    && repairResult.attemptsRun == 2
                    && repairResult.maxAttempts == 3
                    && firstAttemptFailed
                    && repairResult.successfulAttemptIndex == 2
                    && repairResult.finalOperation == "move_node"
                    && repairResult.finalCommandLogStatus == "ai_worker_repair_loop:repaired"
                    && graphMutationApplied
                    && session.collaborationLog.size() >= 6
                    && finalNode != nullptr;
    const auto error = ok ? std::string {}
                          : ! repairResult.ok ? repairResult.error
                          : "C6 AI repair loop proof did not match expected retry/repair evidence";

    if (const auto writeError = writeReport (ok,
                                             repairResult,
                                             graphMutationApplied,
                                             session.collaborationLog.size(),
                                             finalNodeX,
                                             finalNodeY,
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
