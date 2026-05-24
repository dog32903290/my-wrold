#include "AIWorkerCommand.h"

#include "InteractionContract.h"
#include "StorageCommand.h"
#include "StorageContract.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace myworld
{
namespace
{
constexpr const char* saveWorkOperation = "save_work";
constexpr const char* moveNodeOperation = "move_node";

std::string normalizedOperation (const AIWorkerCommandRequest& request)
{
    return request.operation.empty() ? saveWorkOperation : request.operation;
}

std::string normalizedWorkerId (const AIWorkerCommandRequest& request)
{
    return request.workerId.empty() ? "ai-worker" : request.workerId;
}

std::string normalizedCommandId (const AIWorkerCommandRequest& request)
{
    return request.commandId.empty() ? "ai-worker-command" : request.commandId;
}

std::string readTextFile (const std::string& path)
{
    std::ifstream input (path);
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

std::string lastStorageCommandLogStatus (const GraphSession& session)
{
    const std::string prefix = "save_work:";
    for (auto item = session.commandLog.rbegin(); item != session.commandLog.rend(); ++item)
        if (item->rfind (prefix, 0) == 0)
            return *item;

    return {};
}

std::string lastGraphCommandLogStatus (const GraphSession& session, const std::string& command)
{
    for (auto item = session.commandLog.rbegin(); item != session.commandLog.rend(); ++item)
        if (*item == command)
            return *item;

    return {};
}

std::string makeProofEvidence (const AIWorkerCommandEvidence& evidence)
{
    std::ostringstream out;
    out << "graphCommandLogStatus=" << evidence.graphCommandLogStatus
        << "; storageCommandLogStatus=" << evidence.storageCommandLogStatus
        << "; saveLogStatus=" << evidence.saveLogStatus
        << "; graphMutationApplied=" << (evidence.graphMutationApplied ? "true" : "false")
        << "; patchReloaded=" << (evidence.patchReloaded ? "true" : "false")
        << "; usesInteractionState=" << (evidence.usesInteractionState ? "true" : "false");
    return out.str();
}

void appendCollaborationLog (GraphSession& session,
                             const std::string& actor,
                             const std::string& commandId,
                             const std::string& operation,
                             const std::string& intent,
                             const std::string& status,
                             const std::string& result,
                             const std::string& proofEvidence,
                             const std::string& error)
{
    session.collaborationLog.push_back ({ actor,
                                          commandId,
                                          operation,
                                          intent,
                                          status,
                                          result,
                                          proofEvidence,
                                          error });
}

AIWorkerCommandEvidence collectSaveWorkEvidence (const GraphSession& session, const SaveWorkResult& saveResult)
{
    AIWorkerCommandEvidence evidence;
    evidence.storageCommandLogStatus = lastStorageCommandLogStatus (session);
    evidence.saveLogPath = saveResult.saveLogPath;
    evidence.patchPath = saveResult.patchPath;

    if (! saveResult.patchPath.empty())
    {
        const auto reloadedPatch = loadPatchDocument (saveResult.patchPath);
        evidence.patchReloaded = reloadedPatch.ok;

        const auto savedText = readTextFile (saveResult.patchPath);
        evidence.usesInteractionState = savedText.find ("interaction-state-v1") != std::string::npos;
    }

    if (! saveResult.saveLogPath.empty())
    {
        const auto saveLog = loadSaveLog (saveResult.saveLogPath);
        if (saveLog.ok && ! saveLog.entries.empty())
            evidence.saveLogStatus = saveLog.entries.back().status;
    }

    return evidence;
}
}

std::vector<std::string> allowedAIWorkerOperations()
{
    return { saveWorkOperation, moveNodeOperation };
}

AIWorkerCommandResult executeAIWorkerCommand (GraphSession& session, const AIWorkerCommandRequest& request)
{
    const auto operation = normalizedOperation (request);
    const auto workerId = normalizedWorkerId (request);
    const auto commandId = normalizedCommandId (request);

    session.commandLog.push_back ("ai_worker:" + operation + ":requested");
    appendCollaborationLog (session,
                            workerId,
                            commandId,
                            operation,
                            request.intent,
                            "requested",
                            {},
                            {},
                            {});

    const auto allowed = allowedAIWorkerOperations();
    if (std::find (allowed.begin(), allowed.end(), operation) == allowed.end())
    {
        const auto error = "AI worker operation is not allowed: " + operation;
        session.commandLog.push_back ("ai_worker:" + operation + ":rejected");
        appendCollaborationLog (session,
                                workerId,
                                commandId,
                                operation,
                                request.intent,
                                "rejected",
                                "failed",
                                {},
                                error);
        return { false, commandId, workerId, operation, "rejected", error, {} };
    }

    if (operation == moveNodeOperation)
    {
        const auto commandResult = moveNode (session, request.nodeId, request.deltaX, request.deltaY);
        AIWorkerCommandEvidence evidence;
        evidence.graphCommandLogStatus = lastGraphCommandLogStatus (session, moveNodeOperation);
        evidence.graphMutationApplied = commandResult.ok && evidence.graphCommandLogStatus == moveNodeOperation;
        const auto proofEvidence = makeProofEvidence (evidence);
        const auto ok = commandResult.ok && evidence.graphMutationApplied;
        const auto status = ok ? "ok" : "failed";
        const auto error = ok ? std::string {} : commandResult.message;

        appendCollaborationLog (session,
                                workerId,
                                commandId,
                                operation,
                                request.intent,
                                status,
                                ok ? "ok" : "failed",
                                proofEvidence,
                                error);
        session.commandLog.push_back ("ai_worker:" + operation + ":" + status);

        return { ok, commandId, workerId, operation, status, error, std::move (evidence) };
    }

    const auto saveResult = saveWork (session, request.workManifestPath);
    auto evidence = collectSaveWorkEvidence (session, saveResult);
    const auto proofEvidence = makeProofEvidence (evidence);

    std::string error = saveResult.error;
    if (saveResult.ok && ! evidence.patchReloaded)
        error = "AI worker save_work could not reload saved PatchDocument";
    else if (saveResult.ok && evidence.usesInteractionState)
        error = "AI worker save_work wrote interaction-state-v1 evidence";
    else if (saveResult.ok && evidence.saveLogStatus.empty())
        error = "AI worker save_work could not read save log evidence";

    const auto ok = saveResult.ok
                    && evidence.patchReloaded
                    && ! evidence.usesInteractionState
                    && ! evidence.saveLogStatus.empty();
    const auto resultStatus = saveResult.status.empty() ? "validation-failed" : saveResult.status;

    appendCollaborationLog (session,
                            workerId,
                            commandId,
                            operation,
                            request.intent,
                            resultStatus,
                            ok ? "ok" : "failed",
                            proofEvidence,
                            error);
    session.commandLog.push_back ("ai_worker:" + operation + ":" + resultStatus);

    return { ok, commandId, workerId, operation, resultStatus, error, std::move (evidence) };
}
}
