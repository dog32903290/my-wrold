#include "AIWorkerCommand.h"

#include "CompoundPatch.h"
#include "GraphEndpoint.h"
#include "InteractionContract.h"
#include "StorageCommand.h"
#include "StorageContract.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace
{
void expect (bool condition, const std::string& message)
{
    if (! condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        std::exit (1);
    }
}

bool contains (const std::vector<std::string>& values, const std::string& value)
{
    return std::find (values.begin(), values.end(), value) != values.end();
}

bool hasEdgeId (const myworld::GraphContract& graph, const std::string& edgeId)
{
    return std::find_if (graph.editorGraph.edges.begin(),
                         graph.editorGraph.edges.end(),
                         [&edgeId] (const auto& edge) {
                             return edge.id == edgeId;
                         }) != graph.editorGraph.edges.end();
}

std::string readTextFile (const std::filesystem::path& path)
{
    std::ifstream input (path);
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

void copyC2WorkFixture (const std::filesystem::path& workRoot)
{
    std::filesystem::create_directories (workRoot / "patches");
    std::filesystem::copy_file ("fixtures/storage/c2-compound-work/myworld.work.json",
                                workRoot / "myworld.work.json",
                                std::filesystem::copy_options::overwrite_existing);
    std::filesystem::copy_file ("fixtures/storage/c2-compound-work/patches/main.patch.json",
                                workRoot / "patches" / "main.patch.json",
                                std::filesystem::copy_options::overwrite_existing);
}
}

int main()
{
    const auto allowed = myworld::allowedAIWorkerOperations();
    expect (contains (allowed, "save_work"), "AI worker allowed operation includes save_work");
    expect (contains (allowed, "move_node"), "AI worker allowed operation includes move_node");
    expect (contains (allowed, "publish_module"), "AI worker allowed operation includes publish_module");

    const auto loadedCompound = myworld::loadCompoundPatchSpec ("fixtures/compounds/loudness.compound.json");
    expect (loadedCompound.ok, loadedCompound.error);

    const auto workRoot = std::filesystem::temp_directory_path() / "my-world-c4-ai-worker-save-work";
    std::filesystem::remove_all (workRoot);
    copyC2WorkFixture (workRoot);

    const auto workManifestPath = workRoot / "myworld.work.json";
    const auto loadedMain = myworld::loadMainPatchDocumentForWork (workManifestPath.string());
    expect (loadedMain.ok, loadedMain.error);

    auto moveSession = myworld::makeGraphSession (loadedMain.document.graph);
    const auto* initialMoveNode = myworld::findEditorNode (moveSession.graph, "library_loud1");
    expect (initialMoveNode != nullptr, "AI worker move_node fixture has target node");
    const auto initialMoveX = initialMoveNode->position.x;
    const auto initialMoveY = initialMoveNode->position.y;

    myworld::AIWorkerCommandRequest moveRequest;
    moveRequest.commandId = "c4.2-move-node";
    moveRequest.workerId = "ai-worker-test";
    moveRequest.operation = "move_node";
    moveRequest.intent = "Move the loaded loudness compound through the shared interaction command path";
    moveRequest.nodeId = "library_loud1";
    moveRequest.deltaX = 21.0;
    moveRequest.deltaY = 9.0;

    const auto moveResult = myworld::executeAIWorkerCommand (moveSession, moveRequest);
    expect (moveResult.ok, moveResult.error);
    expect (moveResult.operation == "move_node", "AI worker move_node result operation");
    expect (moveResult.status == "ok", "AI worker move_node status");
    expect (moveResult.evidence.graphCommandLogStatus == "move_node",
            "AI worker records underlying InteractionContract command log status");
    expect (moveResult.evidence.graphMutationApplied,
            "AI worker records graph mutation evidence");
    expect (moveSession.dirty, "AI worker move_node leaves session dirty");
    expect (contains (moveSession.commandLog, "ai_worker:move_node:requested"),
            "command log records AI worker move_node intent");
    expect (contains (moveSession.commandLog, "move_node"),
            "AI worker move_node uses InteractionContract::moveNode command path");
    expect (contains (moveSession.commandLog, "ai_worker:move_node:ok"),
            "command log records AI worker move_node result");

    const auto* movedNode = myworld::findEditorNode (moveSession.graph, "library_loud1");
    expect (movedNode != nullptr, "AI worker move_node target remains in graph");
    expect (movedNode->position.x == initialMoveX + 21.0, "AI worker move_node updates x");
    expect (movedNode->position.y == initialMoveY + 9.0, "AI worker move_node updates y");
    expect (moveSession.collaborationLog.size() >= 2, "AI worker move_node records collaboration log entries");
    expect (moveSession.collaborationLog.front().operation == "move_node",
            "collaboration log records move_node operation");
    expect (moveSession.collaborationLog.back().proofEvidence.find ("graphCommandLogStatus=move_node") != std::string::npos,
            "collaboration log records move_node command proof");

    auto& session = moveSession;
    expect (session.dirty, "AI worker move_node dirties session before save_work");

    myworld::AIWorkerCommandRequest request;
    request.commandId = "c4.3-save-work";
    request.workerId = "ai-worker-test";
    request.operation = "save_work";
    request.intent = "Persist AI-mutated C2 compound work through the shared save_work command path";
    request.workManifestPath = workManifestPath.string();

    const auto result = myworld::executeAIWorkerCommand (session, request);
    expect (result.ok, result.error);
    expect (result.operation == "save_work", "AI worker result operation");
    expect (result.status == "save-ok commit-pending", "AI worker save_work status");
    expect (! session.dirty, "AI worker save_work clears dirty graph state through StorageCommand");
    expect (result.evidence.storageCommandLogStatus == "save_work:save-ok commit-pending",
            "AI worker records underlying StorageCommand command log status");
    expect (result.evidence.saveLogStatus == "save-ok commit-pending",
            "AI worker records save log status evidence");
    expect (result.evidence.patchReloaded, "AI worker records PatchDocument reload evidence");
    expect (! result.evidence.usesInteractionState, "AI worker save_work proof does not use interaction-state-v1");

    expect (contains (session.commandLog, "ai_worker:save_work:requested"),
            "command log records AI worker intent");
    expect (contains (session.commandLog, "save_work:save-ok commit-pending"),
            "AI worker save_work uses StorageCommand::saveWork command path");
    expect (contains (session.commandLog, "ai_worker:save_work:save-ok commit-pending"),
            "command log records AI worker result");

    expect (session.collaborationLog.size() >= 4, "AI worker move_node plus save_work records collaboration log entries");
    expect (session.collaborationLog.front().actor == "ai-worker-test",
            "collaboration log records actor");
    expect (session.collaborationLog.front().commandId == "c4.2-move-node",
            "collaboration log records command id");
    expect (session.collaborationLog.front().operation == "move_node",
            "collaboration log records first operation");
    expect (session.collaborationLog.back().commandId == "c4.3-save-work",
            "collaboration log records final command id");
    expect (session.collaborationLog.back().operation == "save_work",
            "collaboration log records final operation");
    expect (session.collaborationLog.back().intent == request.intent,
            "collaboration log records save intent");
    expect (session.collaborationLog.front().status == "requested",
            "collaboration log records requested status");

    const auto& finalLog = session.collaborationLog.back();
    expect (finalLog.status == "save-ok commit-pending", "collaboration log records result status");
    expect (finalLog.proofEvidence.find ("patchReloaded=true") != std::string::npos,
            "collaboration log records patch reload proof");
    expect (finalLog.proofEvidence.find ("saveLogStatus=save-ok commit-pending") != std::string::npos,
            "collaboration log records save log proof");

    const auto savedPatchPath = workRoot / "patches" / "main.patch.json";
    const auto savedText = readTextFile (savedPatchPath);
    expect (savedText.find ("interaction-state-v1") == std::string::npos,
            "AI worker save_work must not use temporary interaction serializer");
    expect (savedText.find ("library_loud1.audio.in") != std::string::npos,
            "AI worker saved PatchDocument keeps public input port edge");
    expect (savedText.find ("library_loud1.out") != std::string::npos,
            "AI worker saved PatchDocument keeps public output port edge");

    const auto reloaded = myworld::loadPatchDocument (savedPatchPath.string());
    expect (reloaded.ok, reloaded.error);
    expect (hasEdgeId (reloaded.document.graph, "edge.live_audio.channels.library_loud1.audio.in"),
            "AI worker save_work reloaded public input edge");
    expect (hasEdgeId (reloaded.document.graph, "edge.library_loud1.out.midi_loudness.value"),
            "AI worker save_work reloaded public output edge");
    const auto* savedMovedNode = myworld::findEditorNode (reloaded.document.graph, "library_loud1");
    expect (savedMovedNode != nullptr, "AI worker save_work reloads moved node");
    expect (savedMovedNode->position.x == movedNode->position.x, "AI worker save_work persists AI move_node x");
    expect (savedMovedNode->position.y == movedNode->position.y, "AI worker save_work persists AI move_node y");

    const auto reloadedExpandedGraph = myworld::makeCompoundPatchInteractionGraph (
        loadedCompound.spec,
        "library_loud1",
        reloaded.document.graph);
    const auto* monoMix = myworld::findEditorNode (reloadedExpandedGraph, "library_loud1/mono_mix");
    expect (monoMix != nullptr, "AI worker save_work reloaded expanded mono_mix");
    expect (monoMix->position.x == 358.0, "AI worker save_work preserves expanded child layout x");
    expect (monoMix->position.y == 146.0, "AI worker save_work preserves expanded child layout y");

    const auto loadedSaveLog = myworld::loadSaveLog (result.evidence.saveLogPath);
    expect (loadedSaveLog.ok, loadedSaveLog.error);
    expect (! loadedSaveLog.entries.empty(), "AI worker save_work save log can be read back");
    expect (loadedSaveLog.entries.back().command == "save_work",
            "AI worker save log still records shared save_work command");
    expect (loadedSaveLog.entries.back().status == "save-ok commit-pending",
            "AI worker save log status reads back");

    std::filesystem::remove_all (workRoot);

    const auto publishRoot = std::filesystem::temp_directory_path() / "my-world-c5-ai-worker-publish-module";
    std::filesystem::remove_all (publishRoot);

    const auto publishLoadedMain = myworld::loadMainPatchDocumentForWork (
        "fixtures/storage/c2-compound-work/myworld.work.json");
    expect (publishLoadedMain.ok, publishLoadedMain.error);

    auto publishSession = myworld::makeGraphSession (publishLoadedMain.document.graph);

    myworld::AIWorkerCommandRequest publishRequest;
    publishRequest.commandId = "c5.2-publish-module";
    publishRequest.workerId = "ai-worker-test";
    publishRequest.operation = "publish_module";
    publishRequest.intent = "Publish the loaded loudness compound through the shared publish_module command path";
    publishRequest.workManifestPath = "fixtures/storage/c2-compound-work/myworld.work.json";
    publishRequest.nodeId = "library_loud1";
    publishRequest.moduleId = "module.ai-published-loudness";
    publishRequest.moduleTitle = "AI Published Loudness";
    publishRequest.publishedNodeType = "compound.ai-published-loudness";
    publishRequest.packageDirectory = (publishRoot / "modules" / "ai-published-loudness").string();
    publishRequest.targetLibraryPath = (publishRoot / "module-libraries" / "ai-published.module-library.json").string();
    publishRequest.overwriteExisting = true;

    const auto publishResult = myworld::executeAIWorkerCommand (publishSession, publishRequest);
    expect (publishResult.ok, publishResult.error);
    expect (publishResult.operation == "publish_module", "AI worker publish_module result operation");
    expect (publishResult.status == "published", "AI worker publish_module status");
    expect (publishResult.evidence.publishCommandLogStatus == "publish_module:published",
            "AI worker records underlying publish_module command log status");
    expect (publishResult.evidence.packageReloaded, "AI worker records published module package reload evidence");
    expect (publishResult.evidence.libraryReloaded, "AI worker records published module library reload evidence");
    expect (std::filesystem::exists (publishResult.evidence.moduleManifestPath),
            "AI worker publish_module writes module manifest");
    expect (std::filesystem::exists (publishResult.evidence.compoundPatchPath),
            "AI worker publish_module writes compound patch");
    expect (std::filesystem::exists (publishResult.evidence.targetLibraryPath),
            "AI worker publish_module writes module library");
    expect (contains (publishSession.commandLog, "ai_worker:publish_module:requested"),
            "command log records AI worker publish_module intent");
    expect (contains (publishSession.commandLog, "publish_module:published"),
            "AI worker publish_module uses StorageCommand::publishModule command path");
    expect (contains (publishSession.commandLog, "ai_worker:publish_module:published"),
            "command log records AI worker publish_module result");

    expect (publishSession.collaborationLog.size() >= 2,
            "AI worker publish_module records collaboration log entries");
    expect (publishSession.collaborationLog.front().operation == "publish_module",
            "collaboration log records publish_module operation");
    expect (publishSession.collaborationLog.back().proofEvidence.find ("publishCommandLogStatus=publish_module:published")
                != std::string::npos,
            "collaboration log records publish_module command proof");
    expect (publishSession.collaborationLog.back().proofEvidence.find ("packageReloaded=true") != std::string::npos,
            "collaboration log records package reload proof");
    expect (publishSession.collaborationLog.back().proofEvidence.find ("libraryReloaded=true") != std::string::npos,
            "collaboration log records library reload proof");

    const auto aiPublishedModule = myworld::loadModulePackageManifest (publishResult.evidence.moduleManifestPath);
    expect (aiPublishedModule.ok, aiPublishedModule.error);
    expect (aiPublishedModule.manifest.nodeType == "compound.ai-published-loudness",
            "AI worker published module reloads with requested node type");

    std::filesystem::remove_all (publishRoot);

    auto repairSession = myworld::makeGraphSession (loadedMain.document.graph);
    const auto* repairInitialNode = myworld::findEditorNode (repairSession.graph, "library_loud1");
    expect (repairInitialNode != nullptr, "AI repair loop fixture has target node");
    const auto repairInitialX = repairInitialNode->position.x;
    const auto repairInitialY = repairInitialNode->position.y;

    myworld::AIWorkerRepairPlan repairPlan;
    repairPlan.repairId = "c6.2-ai-repair-loop";
    repairPlan.workerId = "ai-worker-test";
    repairPlan.intent = "Repair a failed move_node command by retrying through the shared AI command path";
    repairPlan.maxAttempts = 3;

    myworld::AIWorkerCommandRequest failedAttempt;
    failedAttempt.commandId = "c6.2-move-missing-node";
    failedAttempt.workerId = repairPlan.workerId;
    failedAttempt.operation = "move_node";
    failedAttempt.intent = "First repair attempt intentionally targets a missing node";
    failedAttempt.nodeId = "missing_loudness";
    failedAttempt.deltaX = 17.0;
    failedAttempt.deltaY = 5.0;

    myworld::AIWorkerCommandRequest repairedAttempt;
    repairedAttempt.commandId = "c6.2-move-library-loudness";
    repairedAttempt.workerId = repairPlan.workerId;
    repairedAttempt.operation = "move_node";
    repairedAttempt.intent = "Second repair attempt targets the loaded loudness node";
    repairedAttempt.nodeId = "library_loud1";
    repairedAttempt.deltaX = 17.0;
    repairedAttempt.deltaY = 5.0;

    myworld::AIWorkerCommandRequest unusedAttempt = repairedAttempt;
    unusedAttempt.commandId = "c6.2-unused-attempt";
    unusedAttempt.deltaX = 100.0;
    unusedAttempt.deltaY = 100.0;

    repairPlan.attempts = { failedAttempt, repairedAttempt, unusedAttempt };

    const auto repairResult = myworld::executeAIWorkerRepairLoop (repairSession, repairPlan);
    expect (repairResult.ok, repairResult.error);
    expect (repairResult.repairId == "c6.2-ai-repair-loop", "AI repair loop result id");
    expect (repairResult.workerId == "ai-worker-test", "AI repair loop worker id");
    expect (repairResult.status == "repaired", "AI repair loop status");
    expect (repairResult.attemptsRun == 2, "AI repair loop stops after repaired attempt");
    expect (repairResult.maxAttempts == 3, "AI repair loop records max attempts");
    expect (repairResult.successfulAttemptIndex == 2, "AI repair loop records one-based successful attempt index");
    expect (repairResult.finalOperation == "move_node", "AI repair loop final operation");
    expect (repairResult.finalCommandLogStatus == "ai_worker_repair_loop:repaired",
            "AI repair loop final command log status");
    expect (repairResult.attempts.size() == 2, "AI repair loop records only executed attempts");
    expect (! repairResult.attempts.front().commandResult.ok, "AI repair loop first attempt fails");
    expect (repairResult.attempts.front().status == "failed", "AI repair loop first attempt status");
    expect (repairResult.attempts.back().commandResult.ok, "AI repair loop second attempt succeeds");
    expect (repairResult.attempts.back().status == "repaired", "AI repair loop second attempt status");
    expect (repairResult.finalProofEvidence.find ("graphCommandLogStatus=move_node") != std::string::npos,
            "AI repair loop exposes final command proof");
    expect (repairResult.finalProofEvidence.find ("graphMutationApplied=true") != std::string::npos,
            "AI repair loop exposes final mutation proof");
    expect (contains (repairSession.commandLog, "ai_worker_repair_loop:started"),
            "AI repair loop command log records start");
    expect (contains (repairSession.commandLog, "ai_worker_repair_loop:attempt_failed"),
            "AI repair loop command log records failed attempt");
    expect (contains (repairSession.commandLog, "ai_worker_repair_loop:repaired"),
            "AI repair loop command log records repaired status");
    expect (! contains (repairSession.commandLog, "c6.2-unused-attempt"),
            "AI repair loop does not execute unused attempt");

    const auto* repairedNode = myworld::findEditorNode (repairSession.graph, "library_loud1");
    expect (repairedNode != nullptr, "AI repair loop target remains in graph");
    expect (repairedNode->position.x == repairInitialX + 17.0, "AI repair loop repaired x");
    expect (repairedNode->position.y == repairInitialY + 5.0, "AI repair loop repaired y");
    expect (repairSession.collaborationLog.size() >= 6, "AI repair loop records command and loop collaboration entries");
    expect (repairSession.collaborationLog.front().operation == "repair_loop",
            "AI repair loop records loop start in collaboration log");
    expect (repairSession.collaborationLog.back().operation == "repair_loop",
            "AI repair loop records loop result in collaboration log");
    expect (repairSession.collaborationLog.back().status == "repaired",
            "AI repair loop collaboration result status");
    expect (repairSession.collaborationLog.back().proofEvidence.find ("successfulAttemptIndex=2") != std::string::npos,
            "AI repair loop collaboration proof records successful attempt index");

    std::cout << "AI worker command contract ok\n";
    return 0;
}
