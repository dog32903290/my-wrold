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

std::vector<std::string> allowedAIWorkerOperations();
AIWorkerCommandResult executeAIWorkerCommand (GraphSession& session, const AIWorkerCommandRequest& request);
}
