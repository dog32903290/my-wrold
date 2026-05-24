#include "ProofReports.h"

#include "JsonWriter.h"

#include <algorithm>
#include <sstream>

namespace myworld
{
namespace
{
constexpr const char* c6RmsOutputId = "rms";
constexpr const char* c6PeakOutputId = "peak";
constexpr const char* c6SampleCountOutputId = "sampleCount";
constexpr const char* c6RepairTargetNodeId = "library_loud1";

const RuntimeOutputValue* findRuntimeOutput (const std::vector<RuntimeOutputValue>& outputs, const std::string& id)
{
    for (const auto& output : outputs)
        if (output.id == id)
            return &output;

    return nullptr;
}

double runtimeOutputValueOrZero (const std::vector<RuntimeOutputValue>& outputs, const std::string& id)
{
    const auto* output = findRuntimeOutput (outputs, id);
    return output == nullptr ? 0.0 : output->value;
}

std::string runtimeOutputSourceOrEmpty (const std::vector<RuntimeOutputValue>& outputs, const std::string& id)
{
    const auto* output = findRuntimeOutput (outputs, id);
    return output == nullptr ? std::string {} : output->source;
}
}

std::string makeC2StorageReportJson (bool ok,
                                     const std::string& workManifestPath,
                                     const std::string& savedPatchPath,
                                     const std::string& saveStatus,
                                     const GraphSession& session,
                                     bool publicInputEdge,
                                     bool publicOutputEdge,
                                     bool monoMixLayout,
                                     double monoMixX,
                                     double monoMixY,
                                     const std::string& error)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"c2StorageProof\",\n";
    out << "  \"ok\": " << (ok ? "true" : "false") << ",\n";
    out << "  \"source\": \"PatchDocument\",\n";
    out << "  \"usesInteractionState\": false,\n";
    out << "  \"workManifestPath\": " << jsonQuoted (workManifestPath) << ",\n";
    out << "  \"savedPatchPath\": " << jsonQuoted (savedPatchPath) << ",\n";
    out << "  \"saveStatus\": " << jsonQuoted (saveStatus) << ",\n";
    out << "  \"editorNodeCount\": " << session.graph.editorGraph.nodes.size() << ",\n";
    out << "  \"editorEdgeCount\": " << session.graph.editorGraph.edges.size() << ",\n";
    out << "  \"runtimeNodeCount\": " << session.graph.runtimeGraph.nodes.size() << ",\n";
    out << "  \"runtimeEdgeCount\": " << session.graph.runtimeGraph.edges.size() << ",\n";
    out << "  \"publicInputEdge\": " << (publicInputEdge ? "true" : "false") << ",\n";
    out << "  \"publicOutputEdge\": " << (publicOutputEdge ? "true" : "false") << ",\n";
    out << "  \"expandedLayout\": {\n";
    out << "    \"nodeId\": \"library_loud1/mono_mix\",\n";
    out << "    \"matches\": " << (monoMixLayout ? "true" : "false") << ",\n";
    out << "    \"x\": " << monoMixX << ",\n";
    out << "    \"y\": " << monoMixY << "\n";
    out << "  },\n";
    out << "  \"error\": " << jsonQuoted (error) << "\n";
    out << "}\n";
    return out.str();
}

std::string makeC3SaveWorkReportJson (bool ok,
                                      const std::string& workManifestPath,
                                      const std::string& savedPatchPath,
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
    const auto saveLogStatus = saveLog.entries.empty() ? std::string {}
                                                       : saveLog.entries.back().status;
    const auto commitStatus = saveLog.entries.empty() ? std::string {}
                                                      : saveLog.entries.back().commitStatus;

    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"c3SaveWorkProof\",\n";
    out << "  \"ok\": " << (ok ? "true" : "false") << ",\n";
    out << "  \"source\": \"PatchDocument\",\n";
    out << "  \"usesInteractionState\": false,\n";
    out << "  \"workManifestPath\": " << jsonQuoted (workManifestPath) << ",\n";
    out << "  \"savedPatchPath\": " << jsonQuoted (savedPatchPath) << ",\n";
    out << "  \"saveLogPath\": " << jsonQuoted (saveLogPath) << ",\n";
    out << "  \"saveStatus\": " << jsonQuoted (saveStatus) << ",\n";
    out << "  \"commandLogStatus\": " << jsonQuoted (commandLogStatus) << ",\n";
    out << "  \"saveLogOk\": " << (saveLog.ok ? "true" : "false") << ",\n";
    out << "  \"saveLogEntries\": " << saveLog.entries.size() << ",\n";
    out << "  \"saveLogStatus\": " << jsonQuoted (saveLogStatus) << ",\n";
    out << "  \"commitStatus\": " << jsonQuoted (commitStatus) << ",\n";
    out << "  \"editorNodeCount\": " << session.graph.editorGraph.nodes.size() << ",\n";
    out << "  \"editorEdgeCount\": " << session.graph.editorGraph.edges.size() << ",\n";
    out << "  \"runtimeNodeCount\": " << session.graph.runtimeGraph.nodes.size() << ",\n";
    out << "  \"runtimeEdgeCount\": " << session.graph.runtimeGraph.edges.size() << ",\n";
    out << "  \"publicInputEdge\": " << (publicInputEdge ? "true" : "false") << ",\n";
    out << "  \"publicOutputEdge\": " << (publicOutputEdge ? "true" : "false") << ",\n";
    out << "  \"expandedLayout\": {\n";
    out << "    \"nodeId\": \"library_loud1/mono_mix\",\n";
    out << "    \"matches\": " << (monoMixLayout ? "true" : "false") << ",\n";
    out << "    \"x\": " << monoMixX << ",\n";
    out << "    \"y\": " << monoMixY << "\n";
    out << "  },\n";
    out << "  \"error\": " << jsonQuoted (error) << "\n";
    out << "}\n";
    return out.str();
}

std::string makeC4AIWorkerSaveWorkReportJson (bool ok,
                                              const AIWorkerCommandRequest& moveRequest,
                                              const AIWorkerCommandResult& moveResult,
                                              const AIWorkerCommandRequest& saveRequest,
                                              const AIWorkerCommandResult& saveResult,
                                              const std::vector<std::string>& allowedOperations,
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
    const auto saveLogStatus = saveLog.entries.empty() ? std::string {}
                                                       : saveLog.entries.back().status;
    const auto collaborationIntentStatus = session.collaborationLog.empty() ? std::string {}
                                                                            : session.collaborationLog.front().status;
    const auto collaborationResultStatus = session.collaborationLog.empty() ? std::string {}
                                                                            : session.collaborationLog.back().status;
    const auto collaborationProofEvidence = session.collaborationLog.empty() ? std::string {}
                                                                             : session.collaborationLog.back().proofEvidence;
    const auto moveCollaborationProof = [&session]
    {
        for (const auto& item : session.collaborationLog)
            if (item.operation == "move_node" && ! item.proofEvidence.empty())
                return item.proofEvidence;

        return std::string {};
    }();
    const auto saveWorkAllowed = std::find (allowedOperations.begin(), allowedOperations.end(), "save_work")
                                 != allowedOperations.end();
    const auto moveNodeAllowed = std::find (allowedOperations.begin(), allowedOperations.end(), "move_node")
                                 != allowedOperations.end();

    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"c4AIWorkerSaveWorkProof\",\n";
    out << "  \"ok\": " << (ok ? "true" : "false") << ",\n";
    out << "  \"source\": \"PatchDocument\",\n";
    out << "  \"usesInteractionState\": " << (saveResult.evidence.usesInteractionState ? "true" : "false") << ",\n";
    out << "  \"allowedSaveWork\": " << (saveWorkAllowed ? "true" : "false") << ",\n";
    out << "  \"allowedMoveNode\": " << (moveNodeAllowed ? "true" : "false") << ",\n";
    out << "  \"operation\": " << jsonQuoted (saveResult.operation) << ",\n";
    out << "  \"commandId\": " << jsonQuoted (saveResult.commandId) << ",\n";
    out << "  \"workerId\": " << jsonQuoted (saveResult.workerId) << ",\n";
    out << "  \"intent\": " << jsonQuoted (saveRequest.intent) << ",\n";
    out << "  \"workManifestPath\": " << jsonQuoted (saveRequest.workManifestPath) << ",\n";
    out << "  \"savedPatchPath\": " << jsonQuoted (saveResult.evidence.patchPath) << ",\n";
    out << "  \"saveLogPath\": " << jsonQuoted (saveResult.evidence.saveLogPath) << ",\n";
    out << "  \"status\": " << jsonQuoted (saveResult.status) << ",\n";
    out << "  \"moveOperation\": " << jsonQuoted (moveResult.operation) << ",\n";
    out << "  \"moveCommandId\": " << jsonQuoted (moveResult.commandId) << ",\n";
    out << "  \"moveIntent\": " << jsonQuoted (moveRequest.intent) << ",\n";
    out << "  \"moveStatus\": " << jsonQuoted (moveResult.status) << ",\n";
    out << "  \"graphCommandLogStatus\": " << jsonQuoted (moveResult.evidence.graphCommandLogStatus) << ",\n";
    out << "  \"graphMutationApplied\": " << (moveResult.evidence.graphMutationApplied ? "true" : "false") << ",\n";
    out << "  \"storageCommandLogStatus\": " << jsonQuoted (saveResult.evidence.storageCommandLogStatus) << ",\n";
    out << "  \"aiCommandLogStatus\": " << jsonQuoted (aiCommandLogStatus) << ",\n";
    out << "  \"patchReloaded\": " << (saveResult.evidence.patchReloaded ? "true" : "false") << ",\n";
    out << "  \"saveLogOk\": " << (saveLog.ok ? "true" : "false") << ",\n";
    out << "  \"saveLogEntries\": " << saveLog.entries.size() << ",\n";
    out << "  \"saveLogStatus\": " << jsonQuoted (saveLogStatus) << ",\n";
    out << "  \"collaborationLogEntries\": " << session.collaborationLog.size() << ",\n";
    out << "  \"collaborationIntentStatus\": " << jsonQuoted (collaborationIntentStatus) << ",\n";
    out << "  \"collaborationResultStatus\": " << jsonQuoted (collaborationResultStatus) << ",\n";
    out << "  \"moveCollaborationProofEvidence\": " << jsonQuoted (moveCollaborationProof) << ",\n";
    out << "  \"collaborationProofEvidence\": " << jsonQuoted (collaborationProofEvidence) << ",\n";
    out << "  \"editorNodeCount\": " << session.graph.editorGraph.nodes.size() << ",\n";
    out << "  \"editorEdgeCount\": " << session.graph.editorGraph.edges.size() << ",\n";
    out << "  \"runtimeNodeCount\": " << session.graph.runtimeGraph.nodes.size() << ",\n";
    out << "  \"runtimeEdgeCount\": " << session.graph.runtimeGraph.edges.size() << ",\n";
    out << "  \"publicInputEdge\": " << (publicInputEdge ? "true" : "false") << ",\n";
    out << "  \"publicOutputEdge\": " << (publicOutputEdge ? "true" : "false") << ",\n";
    out << "  \"savedMove\": {\n";
    out << "    \"nodeId\": \"library_loud1\",\n";
    out << "    \"matches\": " << (savedMovePersisted ? "true" : "false") << ",\n";
    out << "    \"x\": " << savedMoveX << ",\n";
    out << "    \"y\": " << savedMoveY << "\n";
    out << "  },\n";
    out << "  \"expandedLayout\": {\n";
    out << "    \"nodeId\": \"library_loud1/mono_mix\",\n";
    out << "    \"matches\": " << (monoMixLayout ? "true" : "false") << ",\n";
    out << "    \"x\": " << monoMixX << ",\n";
    out << "    \"y\": " << monoMixY << "\n";
    out << "  },\n";
    out << "  \"error\": " << jsonQuoted (error) << "\n";
    out << "}\n";
    return out.str();
}

std::string makeC5ModulePublishReportJson (bool ok,
                                           const PublishModuleResult& publish,
                                           bool packageReloaded,
                                           bool libraryReloaded,
                                           bool visibleRegistryContainsPublishedNode,
                                           bool runtimeRegistryContainsPublishedNode,
                                           const std::string& runtimeCoverageStatus,
                                           bool createdPublishedNode,
                                           const std::string& graphCommandLogStatus,
                                           const std::string& error)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"c5ModulePublishProof\",\n";
    out << "  \"ok\": " << (ok ? "true" : "false") << ",\n";
    out << "  \"operation\": " << jsonQuoted (publish.operation) << ",\n";
    out << "  \"source\": \"PatchDocument\",\n";
    out << "  \"sourceNodeId\": " << jsonQuoted (publish.sourceNodeId) << ",\n";
    out << "  \"sourceNodeType\": " << jsonQuoted (publish.sourceNodeType) << ",\n";
    out << "  \"publishedModuleId\": " << jsonQuoted (publish.moduleId) << ",\n";
    out << "  \"publishedNodeType\": " << jsonQuoted (publish.publishedNodeType) << ",\n";
    out << "  \"moduleManifestPath\": " << jsonQuoted (publish.moduleManifestPath) << ",\n";
    out << "  \"compoundPatchPath\": " << jsonQuoted (publish.compoundPatchPath) << ",\n";
    out << "  \"targetLibraryPath\": " << jsonQuoted (publish.targetLibraryPath) << ",\n";
    out << "  \"packageReloaded\": " << (packageReloaded ? "true" : "false") << ",\n";
    out << "  \"libraryReloaded\": " << (libraryReloaded ? "true" : "false") << ",\n";
    out << "  \"visibleRegistryContainsPublishedNode\": " << (visibleRegistryContainsPublishedNode ? "true" : "false") << ",\n";
    out << "  \"runtimeRegistryContainsPublishedNode\": " << (runtimeRegistryContainsPublishedNode ? "true" : "false") << ",\n";
    out << "  \"runtimeCoverageStatus\": " << jsonQuoted (runtimeCoverageStatus) << ",\n";
    out << "  \"createdPublishedNode\": " << (createdPublishedNode ? "true" : "false") << ",\n";
    out << "  \"graphCommandLogStatus\": " << jsonQuoted (graphCommandLogStatus) << ",\n";
    out << "  \"usesInteractionState\": false,\n";
    out << "  \"error\": " << jsonQuoted (error) << "\n";
    out << "}\n";
    return out.str();
}

std::string makeC5AIWorkerModulePublishReportJson (bool ok,
                                                   bool allowedPublishModule,
                                                   const AIWorkerCommandRequest& request,
                                                   const AIWorkerCommandResult& result,
                                                   size_t collaborationLogEntries,
                                                   const std::string& collaborationProofEvidence,
                                                   const std::string& aiCommandLogStatus,
                                                   const std::string& error)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"c5AIWorkerModulePublishProof\",\n";
    out << "  \"ok\": " << (ok ? "true" : "false") << ",\n";
    out << "  \"source\": \"PatchDocument\",\n";
    out << "  \"allowedPublishModule\": " << (allowedPublishModule ? "true" : "false") << ",\n";
    out << "  \"operation\": " << jsonQuoted (result.operation) << ",\n";
    out << "  \"commandId\": " << jsonQuoted (result.commandId) << ",\n";
    out << "  \"workerId\": " << jsonQuoted (result.workerId) << ",\n";
    out << "  \"intent\": " << jsonQuoted (request.intent) << ",\n";
    out << "  \"sourceNodeId\": " << jsonQuoted (request.nodeId) << ",\n";
    out << "  \"publishedModuleId\": " << jsonQuoted (request.moduleId) << ",\n";
    out << "  \"publishedNodeType\": " << jsonQuoted (request.publishedNodeType) << ",\n";
    out << "  \"moduleManifestPath\": " << jsonQuoted (result.evidence.moduleManifestPath) << ",\n";
    out << "  \"compoundPatchPath\": " << jsonQuoted (result.evidence.compoundPatchPath) << ",\n";
    out << "  \"targetLibraryPath\": " << jsonQuoted (result.evidence.targetLibraryPath) << ",\n";
    out << "  \"status\": " << jsonQuoted (result.status) << ",\n";
    out << "  \"publishCommandLogStatus\": " << jsonQuoted (result.evidence.publishCommandLogStatus) << ",\n";
    out << "  \"aiCommandLogStatus\": " << jsonQuoted (aiCommandLogStatus) << ",\n";
    out << "  \"packageReloaded\": " << (result.evidence.packageReloaded ? "true" : "false") << ",\n";
    out << "  \"libraryReloaded\": " << (result.evidence.libraryReloaded ? "true" : "false") << ",\n";
    out << "  \"collaborationLogEntries\": " << collaborationLogEntries << ",\n";
    out << "  \"collaborationProofEvidence\": " << jsonQuoted (collaborationProofEvidence) << ",\n";
    out << "  \"usesInteractionState\": false,\n";
    out << "  \"error\": " << jsonQuoted (error) << "\n";
    out << "}\n";
    return out.str();
}

std::string makeC5VisibleModulePublishReportJson (bool ok,
                                                  const PublishModuleResult& publish,
                                                  bool packageReloaded,
                                                  bool libraryReloaded,
                                                  bool visibleRegistryContainsPublishedNode,
                                                  bool createdPublishedNode,
                                                  const std::string& graphCommandLogStatus,
                                                  const std::string& error)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"c5VisibleModulePublishProof\",\n";
    out << "  \"ok\": " << (ok ? "true" : "false") << ",\n";
    out << "  \"operation\": " << jsonQuoted (publish.operation) << ",\n";
    out << "  \"source\": \"PatchDocument\",\n";
    out << "  \"sourceNodeId\": " << jsonQuoted (publish.sourceNodeId) << ",\n";
    out << "  \"sourceNodeType\": " << jsonQuoted (publish.sourceNodeType) << ",\n";
    out << "  \"publishedModuleId\": " << jsonQuoted (publish.moduleId) << ",\n";
    out << "  \"publishedNodeType\": " << jsonQuoted (publish.publishedNodeType) << ",\n";
    out << "  \"moduleManifestPath\": " << jsonQuoted (publish.moduleManifestPath) << ",\n";
    out << "  \"compoundPatchPath\": " << jsonQuoted (publish.compoundPatchPath) << ",\n";
    out << "  \"targetLibraryPath\": " << jsonQuoted (publish.targetLibraryPath) << ",\n";
    out << "  \"status\": " << jsonQuoted (publish.status) << ",\n";
    out << "  \"packageReloaded\": " << (packageReloaded ? "true" : "false") << ",\n";
    out << "  \"libraryReloaded\": " << (libraryReloaded ? "true" : "false") << ",\n";
    out << "  \"visibleRegistryContainsPublishedNode\": " << (visibleRegistryContainsPublishedNode ? "true" : "false") << ",\n";
    out << "  \"createdPublishedNode\": " << (createdPublishedNode ? "true" : "false") << ",\n";
    out << "  \"graphCommandLogStatus\": " << jsonQuoted (graphCommandLogStatus) << ",\n";
    out << "  \"usesInteractionState\": false,\n";
    out << "  \"error\": " << jsonQuoted (error) << "\n";
    out << "}\n";
    return out.str();
}

std::string makeC6AnalyzerFamilyReportJson (bool ok,
                                            const std::string& libraryPath,
                                            size_t familyEntryCount,
                                            bool visibleRegistryContainsRawEnergy,
                                            bool runtimeRegistryContainsRawEnergy,
                                            const std::string& runtimeCoverageStatus,
                                            bool createdRawEnergyNode,
                                            const std::string& graphCommandLogStatus,
                                            bool loudnessStillPresent,
                                            const std::vector<RuntimeOutputValue>& rawEnergyPublicOutputs,
                                            const std::string& error)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"c6AnalyzerFamilyProof\",\n";
    out << "  \"ok\": " << (ok ? "true" : "false") << ",\n";
    out << "  \"operation\": \"analyzer_compound_family_seed\",\n";
    out << "  \"libraryPath\": " << jsonQuoted (libraryPath) << ",\n";
    out << "  \"familyEntryCount\": " << familyEntryCount << ",\n";
    out << "  \"visibleRegistryContainsRawEnergy\": " << (visibleRegistryContainsRawEnergy ? "true" : "false") << ",\n";
    out << "  \"runtimeRegistryContainsRawEnergy\": " << (runtimeRegistryContainsRawEnergy ? "true" : "false") << ",\n";
    out << "  \"runtimeCoverageStatus\": " << jsonQuoted (runtimeCoverageStatus) << ",\n";
    out << "  \"createdRawEnergyNode\": " << (createdRawEnergyNode ? "true" : "false") << ",\n";
    out << "  \"graphCommandLogStatus\": " << jsonQuoted (graphCommandLogStatus) << ",\n";
    out << "  \"rawEnergyPublicOutputs\": {\n";
    out << "    \"rms\": " << runtimeOutputValueOrZero (rawEnergyPublicOutputs, c6RmsOutputId) << ",\n";
    out << "    \"peak\": " << runtimeOutputValueOrZero (rawEnergyPublicOutputs, c6PeakOutputId) << ",\n";
    out << "    \"sampleCount\": " << runtimeOutputValueOrZero (rawEnergyPublicOutputs, c6SampleCountOutputId) << "\n";
    out << "  },\n";
    out << "  \"rawEnergyPublicOutputSources\": {\n";
    out << "    \"rms\": " << jsonQuoted (runtimeOutputSourceOrEmpty (rawEnergyPublicOutputs, c6RmsOutputId)) << ",\n";
    out << "    \"peak\": " << jsonQuoted (runtimeOutputSourceOrEmpty (rawEnergyPublicOutputs, c6PeakOutputId)) << ",\n";
    out << "    \"sampleCount\": "
        << jsonQuoted (runtimeOutputSourceOrEmpty (rawEnergyPublicOutputs, c6SampleCountOutputId)) << "\n";
    out << "  },\n";
    out << "  \"loudnessStillPresent\": " << (loudnessStillPresent ? "true" : "false") << ",\n";
    out << "  \"usesInteractionState\": false,\n";
    out << "  \"error\": " << jsonQuoted (error) << "\n";
    out << "}\n";
    return out.str();
}

std::string makeC6AIRepairLoopReportJson (bool ok,
                                          const AIWorkerRepairLoopResult& repairResult,
                                          bool graphMutationApplied,
                                          size_t collaborationLogEntries,
                                          bool usesInteractionState,
                                          double finalNodeX,
                                          double finalNodeY,
                                          const std::string& error)
{
    const auto firstAttemptStatus = repairResult.attempts.empty() ? std::string {}
                                                                  : repairResult.attempts.front().status;

    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"c6AIRepairLoopProof\",\n";
    out << "  \"ok\": " << (ok ? "true" : "false") << ",\n";
    out << "  \"operation\": \"ai_repair_loop\",\n";
    out << "  \"repairId\": " << jsonQuoted (repairResult.repairId) << ",\n";
    out << "  \"workerId\": " << jsonQuoted (repairResult.workerId) << ",\n";
    out << "  \"status\": " << jsonQuoted (repairResult.status) << ",\n";
    out << "  \"attemptsRun\": " << repairResult.attemptsRun << ",\n";
    out << "  \"maxAttempts\": " << repairResult.maxAttempts << ",\n";
    out << "  \"firstAttemptStatus\": " << jsonQuoted (firstAttemptStatus) << ",\n";
    out << "  \"successfulAttemptIndex\": " << repairResult.successfulAttemptIndex << ",\n";
    out << "  \"finalOperation\": " << jsonQuoted (repairResult.finalOperation) << ",\n";
    out << "  \"finalCommandLogStatus\": " << jsonQuoted (repairResult.finalCommandLogStatus) << ",\n";
    out << "  \"finalProofEvidence\": " << jsonQuoted (repairResult.finalProofEvidence) << ",\n";
    out << "  \"graphMutationApplied\": " << (graphMutationApplied ? "true" : "false") << ",\n";
    out << "  \"collaborationLogEntries\": " << collaborationLogEntries << ",\n";
    out << "  \"usesInteractionState\": " << (usesInteractionState ? "true" : "false") << ",\n";
    out << "  \"finalNode\": {\n";
    out << "    \"nodeId\": " << jsonQuoted (c6RepairTargetNodeId) << ",\n";
    out << "    \"x\": " << finalNodeX << ",\n";
    out << "    \"y\": " << finalNodeY << "\n";
    out << "  },\n";
    out << "  \"error\": " << jsonQuoted (error) << "\n";
    out << "}\n";
    return out.str();
}
}
