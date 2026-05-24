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
};

struct AIWorkerCommandEvidence
{
    std::string storageCommandLogStatus;
    std::string saveLogPath;
    std::string saveLogStatus;
    std::string patchPath;
    bool patchReloaded = false;
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

std::vector<std::string> allowedAIWorkerOperations();
AIWorkerCommandResult executeAIWorkerCommand (GraphSession& session, const AIWorkerCommandRequest& request);
}
