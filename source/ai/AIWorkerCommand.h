#pragma once

#include <string>
#include <vector>

namespace myworld
{
struct GraphSession;

struct AIWorkerCommandRequest
{
    std::string commandId;
    std::string workerId;
    std::string operation;
    std::string intent;
    std::string workManifestPath;
    std::string nodeId;
    std::string moduleId;
    std::string moduleTitle;
    std::string publishedNodeType;
    std::string packageDirectory;
    std::string targetLibraryPath;
    double deltaX = 0.0;
    double deltaY = 0.0;
    bool overwriteExisting = false;
};

struct AIWorkerCommandEvidence
{
    std::string graphCommandLogStatus;
    std::string storageCommandLogStatus;
    std::string publishCommandLogStatus;
    std::string saveLogPath;
    std::string saveLogStatus;
    std::string patchPath;
    std::string moduleManifestPath;
    std::string compoundPatchPath;
    std::string targetLibraryPath;
    bool graphMutationApplied = false;
    bool patchReloaded = false;
    bool packageReloaded = false;
    bool libraryReloaded = false;
    bool usesInteractionState = false;
};

struct AIWorkerCommandResult
{
    bool ok = false;
    std::string commandId;
    std::string workerId;
    std::string operation;
    std::string status;
    std::string error;
    AIWorkerCommandEvidence evidence;
};

struct AIWorkerRepairAttemptResult
{
    size_t attemptIndex = 0;
    std::string status;
    AIWorkerCommandResult commandResult;
};

struct AIWorkerRepairPlan
{
    std::string repairId;
    std::string workerId;
    std::string intent;
    size_t maxAttempts = 1;
    std::vector<AIWorkerCommandRequest> attempts;
};

struct AIWorkerRepairLoopResult
{
    bool ok = false;
    std::string repairId;
    std::string workerId;
    std::string status;
    std::string error;
    size_t attemptsRun = 0;
    size_t maxAttempts = 0;
    size_t successfulAttemptIndex = 0;
    std::string finalOperation;
    std::string finalCommandLogStatus;
    std::string finalProofEvidence;
    std::vector<AIWorkerRepairAttemptResult> attempts;
};

std::vector<std::string> allowedAIWorkerOperations();
AIWorkerCommandResult executeAIWorkerCommand (GraphSession& session, const AIWorkerCommandRequest& request);
AIWorkerRepairLoopResult executeAIWorkerRepairLoop (GraphSession& session, const AIWorkerRepairPlan& plan);
}
