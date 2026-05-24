#include "AIWorkerCommand.h"

#include "CompoundPatch.h"
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
constexpr const char* publishModuleOperation = "publish_module";

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

std::string normalizedRepairId (const AIWorkerRepairPlan& plan)
{
    return plan.repairId.empty() ? "ai-worker-repair-loop" : plan.repairId;
}

std::string normalizedRepairWorkerId (const AIWorkerRepairPlan& plan)
{
    return plan.workerId.empty() ? "ai-worker" : plan.workerId;
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

std::string lastPublishCommandLogStatus (const GraphSession& session)
{
    const std::string prefix = "publish_module:";
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
        << "; publishCommandLogStatus=" << evidence.publishCommandLogStatus
        << "; saveLogStatus=" << evidence.saveLogStatus
        << "; graphMutationApplied=" << (evidence.graphMutationApplied ? "true" : "false")
        << "; patchReloaded=" << (evidence.patchReloaded ? "true" : "false")
        << "; packageReloaded=" << (evidence.packageReloaded ? "true" : "false")
        << "; libraryReloaded=" << (evidence.libraryReloaded ? "true" : "false")
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

std::string lastCollaborationProofEvidence (const GraphSession& session,
                                            const std::string& operation,
                                            const std::string& commandId)
{
    for (auto item = session.collaborationLog.rbegin(); item != session.collaborationLog.rend(); ++item)
    {
        if (item->operation == operation && item->commandId == commandId)
            return item->proofEvidence;
    }

    return {};
}

std::string makeRepairLoopProofEvidence (const AIWorkerRepairLoopResult& result)
{
    std::ostringstream out;
    out << "attemptsRun=" << result.attemptsRun
        << "; maxAttempts=" << result.maxAttempts
        << "; successfulAttemptIndex=" << result.successfulAttemptIndex
        << "; finalOperation=" << result.finalOperation
        << "; finalCommandLogStatus=" << result.finalCommandLogStatus
        << "; finalProofEvidence=" << result.finalProofEvidence;
    return out.str();
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

PublishModuleRequest makePublishModuleRequest (const AIWorkerCommandRequest& request)
{
    PublishModuleRequest publishRequest;
    publishRequest.workManifestPath = request.workManifestPath;
    publishRequest.sourceNodeId = request.nodeId;
    publishRequest.moduleId = request.moduleId;
    publishRequest.moduleTitle = request.moduleTitle;
    publishRequest.nodeType = request.publishedNodeType;
    publishRequest.packageDirectory = request.packageDirectory;
    publishRequest.targetLibraryPath = request.targetLibraryPath;
    publishRequest.overwriteExisting = request.overwriteExisting;
    return publishRequest;
}

AIWorkerCommandEvidence collectPublishModuleEvidence (const GraphSession& session,
                                                      const PublishModuleResult& publishResult)
{
    AIWorkerCommandEvidence evidence;
    evidence.publishCommandLogStatus = lastPublishCommandLogStatus (session);
    evidence.moduleManifestPath = publishResult.moduleManifestPath;
    evidence.compoundPatchPath = publishResult.compoundPatchPath;
    evidence.targetLibraryPath = publishResult.targetLibraryPath;

    if (! publishResult.moduleManifestPath.empty() && ! publishResult.compoundPatchPath.empty())
    {
        const auto module = loadModulePackageManifest (publishResult.moduleManifestPath);
        const auto compound = loadCompoundPatchSpec (publishResult.compoundPatchPath);
        evidence.packageReloaded = module.ok && compound.ok;
    }

    if (! publishResult.targetLibraryPath.empty())
    {
        const auto library = loadModuleLibraryManifest (publishResult.targetLibraryPath);
        evidence.libraryReloaded = library.ok;
    }

    return evidence;
}
}

std::vector<std::string> allowedAIWorkerOperations()
{
    return { saveWorkOperation, moveNodeOperation, publishModuleOperation };
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

    if (operation == publishModuleOperation)
    {
        const auto publishResult = publishModule (session, makePublishModuleRequest (request));
        auto evidence = collectPublishModuleEvidence (session, publishResult);
        const auto proofEvidence = makeProofEvidence (evidence);

        std::string error = publishResult.error;
        if (publishResult.ok && evidence.publishCommandLogStatus != "publish_module:published")
            error = "AI worker publish_module did not record shared publish command status";
        else if (publishResult.ok && ! evidence.packageReloaded)
            error = "AI worker publish_module could not reload published module package";
        else if (publishResult.ok && ! evidence.libraryReloaded)
            error = "AI worker publish_module could not reload published module library";

        const auto ok = publishResult.ok
                        && evidence.publishCommandLogStatus == "publish_module:published"
                        && evidence.packageReloaded
                        && evidence.libraryReloaded;
        const auto resultStatus = publishResult.status.empty() ? "validation-failed" : publishResult.status;

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

AIWorkerRepairLoopResult executeAIWorkerRepairLoop (GraphSession& session, const AIWorkerRepairPlan& plan)
{
    const auto repairId = normalizedRepairId (plan);
    const auto workerId = normalizedRepairWorkerId (plan);

    AIWorkerRepairLoopResult result;
    result.repairId = repairId;
    result.workerId = workerId;
    result.maxAttempts = plan.maxAttempts;

    session.commandLog.push_back ("ai_worker_repair_loop:started");
    appendCollaborationLog (session,
                            workerId,
                            repairId,
                            "repair_loop",
                            plan.intent,
                            "started",
                            {},
                            {},
                            {});

    if (plan.maxAttempts == 0)
    {
        result.status = "rejected";
        result.error = "AI repair loop requires maxAttempts > 0";
        result.finalCommandLogStatus = "ai_worker_repair_loop:rejected";
        session.commandLog.push_back (result.finalCommandLogStatus);
        appendCollaborationLog (session,
                                workerId,
                                repairId,
                                "repair_loop",
                                plan.intent,
                                result.status,
                                "failed",
                                makeRepairLoopProofEvidence (result),
                                result.error);
        return result;
    }

    if (plan.attempts.empty())
    {
        result.status = "rejected";
        result.error = "AI repair loop requires at least one attempt";
        result.finalCommandLogStatus = "ai_worker_repair_loop:rejected";
        session.commandLog.push_back (result.finalCommandLogStatus);
        appendCollaborationLog (session,
                                workerId,
                                repairId,
                                "repair_loop",
                                plan.intent,
                                result.status,
                                "failed",
                                makeRepairLoopProofEvidence (result),
                                result.error);
        return result;
    }

    const auto attemptsToRun = std::min (plan.maxAttempts, plan.attempts.size());

    for (size_t index = 0; index < attemptsToRun; ++index)
    {
        auto request = plan.attempts[index];
        if (request.workerId.empty())
            request.workerId = workerId;

        const auto commandResult = executeAIWorkerCommand (session, request);
        const auto operation = commandResult.operation;
        const auto commandId = commandResult.commandId;
        const auto proofEvidence = lastCollaborationProofEvidence (session, operation, commandId);

        result.attemptsRun = index + 1;
        result.finalOperation = operation;
        result.finalProofEvidence = proofEvidence;
        result.attempts.push_back ({ index + 1,
                                     commandResult.ok ? std::string { "repaired" } : std::string { "failed" },
                                     commandResult });

        if (commandResult.ok)
        {
            result.ok = true;
            result.status = "repaired";
            result.successfulAttemptIndex = index + 1;
            result.finalCommandLogStatus = "ai_worker_repair_loop:repaired";
            session.commandLog.push_back (result.finalCommandLogStatus);
            appendCollaborationLog (session,
                                    workerId,
                                    repairId,
                                    "repair_loop",
                                    plan.intent,
                                    result.status,
                                    "ok",
                                    makeRepairLoopProofEvidence (result),
                                    {});
            return result;
        }

        session.commandLog.push_back ("ai_worker_repair_loop:attempt_failed");
        appendCollaborationLog (session,
                                workerId,
                                repairId,
                                "repair_loop",
                                plan.intent,
                                "attempt_failed",
                                "failed",
                                makeRepairLoopProofEvidence (result),
                                commandResult.error);
        result.error = commandResult.error;
    }

    result.ok = false;
    result.status = "failed";
    result.finalCommandLogStatus = "ai_worker_repair_loop:failed";
    if (result.error.empty())
        result.error = "AI repair loop exhausted attempts without a successful command";
    session.commandLog.push_back (result.finalCommandLogStatus);
    appendCollaborationLog (session,
                            workerId,
                            repairId,
                            "repair_loop",
                            plan.intent,
                            result.status,
                            "failed",
                            makeRepairLoopProofEvidence (result),
                            result.error);
    return result;
}
}
