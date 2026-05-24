#include "C5ModulePublishProofRunner.h"

#include "AIWorkerCommand.h"
#include "CompoundModule.h"
#include "GraphContract.h"
#include "InteractionContract.h"
#include "ProofReports.h"
#include "ProofRunSupport.h"
#include "RuntimeRegistry.h"
#include "StorageCommand.h"
#include "StorageContract.h"

#include <algorithm>
#include <cctype>
#include <filesystem>

namespace myworld
{
namespace
{
constexpr const char* workFixturePath = "fixtures/storage/c2-compound-work/myworld.work.json";
constexpr const char* proofWorkerId = "ai-worker-proof";
constexpr const char* loudnessCompoundNodeId = "library_loud1";

struct C5ModulePublishProofDefinition
{
    C5ModulePublishProofKind kind;
    const char* displayName;
    const char* directoryName;
    const char* reportFileName;
};

constexpr C5ModulePublishProofDefinition modulePublishDefinition {
    C5ModulePublishProofKind::modulePublish,
    "C5 module publish",
    "c5-module-publish-proof",
    "module_publish_report.json"
};

constexpr C5ModulePublishProofDefinition aiWorkerModulePublishDefinition {
    C5ModulePublishProofKind::aiWorkerModulePublish,
    "C5 AI worker module publish",
    "c5-ai-worker-module-publish-proof",
    "ai_worker_module_publish_report.json"
};

constexpr C5ModulePublishProofDefinition visibleModulePublishDefinition {
    C5ModulePublishProofKind::visibleModulePublish,
    "C5 visible module publish",
    "c5-visible-module-publish-proof",
    "visible_module_publish_report.json"
};

const C5ModulePublishProofDefinition& definitionFor (C5ModulePublishProofKind kind)
{
    switch (kind)
    {
        case C5ModulePublishProofKind::modulePublish:
            return modulePublishDefinition;
        case C5ModulePublishProofKind::aiWorkerModulePublish:
            return aiWorkerModulePublishDefinition;
        case C5ModulePublishProofKind::visibleModulePublish:
            return visibleModulePublishDefinition;
    }

    return modulePublishDefinition;
}

std::string firstLoadableWorkManifestPath (const C5ModulePublishProofRunRequest& request)
{
    for (const auto& candidate : proofCandidatePaths (request.candidateRoots, workFixturePath))
    {
        if (loadMainPatchDocumentForWork (candidate.string()).ok)
            return candidate.string();
    }

    return {};
}

std::string safeIdentifier (const std::string& text)
{
    std::string result;
    result.reserve (text.size());

    for (const auto character : text)
    {
        const auto byte = static_cast<unsigned char> (character);
        if (std::isalnum (byte) != 0 || character == '-' || character == '_')
            result.push_back (static_cast<char> (std::tolower (byte)));
        else
            result.push_back ('-');
    }

    return result.empty() ? "module" : result;
}

PublishModuleRequest makeModulePublishProofRequest (std::string workManifestPath,
                                                    const std::filesystem::path& packageDirectory,
                                                    const std::filesystem::path& targetLibraryPath)
{
    PublishModuleRequest request;
    request.workManifestPath = std::move (workManifestPath);
    request.sourceNodeId = loudnessCompoundNodeId;
    request.moduleId = "module.published-loudness";
    request.moduleTitle = "Published Loudness";
    request.nodeType = "compound.published-loudness";
    request.packageDirectory = packageDirectory.string();
    request.targetLibraryPath = targetLibraryPath.string();
    request.overwriteExisting = true;
    return request;
}

AIWorkerCommandRequest makeAIWorkerModulePublishProofRequest (const std::filesystem::path& packageDirectory,
                                                              const std::filesystem::path& targetLibraryPath)
{
    AIWorkerCommandRequest request;
    request.commandId = "c5.2-publish-module";
    request.workerId = proofWorkerId;
    request.operation = "publish_module";
    request.intent = "Publish the loaded loudness compound through the shared publish_module command path";
    request.nodeId = loudnessCompoundNodeId;
    request.moduleId = "module.ai-published-loudness";
    request.moduleTitle = "AI Published Loudness";
    request.publishedNodeType = "compound.ai-published-loudness";
    request.packageDirectory = packageDirectory.string();
    request.targetLibraryPath = targetLibraryPath.string();
    request.overwriteExisting = true;
    return request;
}

PublishModuleRequest makeVisibleModulePublishProofRequest (std::string workManifestPath,
                                                           const std::string& sourceNodeId,
                                                           const std::filesystem::path& publishDirectory)
{
    const auto safeNodeId = safeIdentifier (sourceNodeId);

    PublishModuleRequest request;
    request.workManifestPath = std::move (workManifestPath);
    request.sourceNodeId = sourceNodeId;
    request.moduleId = "module.visible-" + safeNodeId;
    request.moduleTitle = "Visible " + sourceNodeId;
    request.nodeType = "compound.visible-" + safeNodeId;
    request.packageDirectory = (publishDirectory / "modules" / safeNodeId).string();
    request.targetLibraryPath = (publishDirectory / "module-libraries" / "visible.module-library.json").string();
    request.overwriteExisting = true;
    return request;
}

C5ModulePublishProofRunResult makeInitialResult (const C5ModulePublishProofRunRequest& request,
                                                 const C5ModulePublishProofDefinition& definition)
{
    C5ModulePublishProofRunResult result;
    result.outputDirectory = request.outputDirectory;
    result.reportPath = request.outputDirectory / definition.reportFileName;
    result.artifactPaths = { result.reportPath };
    return result;
}

bool containsPublishedNodeSpec (const CompoundModuleNodeSpecsResult& loadedSpecs,
                                const std::string& publishedNodeType)
{
    return loadedSpecs.ok
        && std::any_of (loadedSpecs.specs.begin(),
                        loadedSpecs.specs.end(),
                        [&publishedNodeType] (const auto& spec) {
                            return spec.type == publishedNodeType;
                        });
}

bool runtimeRegistryContainsPublishedNode (const RuntimeRegistryLoadResult& runtime,
                                           const std::string& publishedNodeType)
{
    return runtime.ok
        && std::any_of (runtime.registry.entries.begin(),
                        runtime.registry.entries.end(),
                        [&publishedNodeType] (const auto& entry) {
                            return entry.nodeType == publishedNodeType;
                        });
}

std::string runtimeCoverageStatusForPublishedNode (const RuntimeOpCoverageResult& coverage,
                                                   const std::string& publishedNodeType)
{
    if (! coverage.ok)
        return {};

    const auto diagnostics = makeRuntimeOpModuleDiagnostics (coverage.snapshot);
    for (const auto& diagnostic : diagnostics)
    {
        if (diagnostic.nodeType != publishedNodeType)
            continue;

        return diagnostic.status == "runtime-op-ready" ? std::string { "ready" } : diagnostic.status;
    }

    return {};
}

CommandResult createPublishedNode (const CompoundModuleNodeSpecsResult& loadedSpecs,
                                   const std::string& publishedNodeType,
                                   const std::string& nodeId,
                                   GraphSession& session)
{
    if (! loadedSpecs.ok)
        return { false, "published node spec not loaded" };

    const auto visibleRegistry = mergeNodeSpecs (makeSeedNodeSpecs(), loadedSpecs.specs);
    return createNode (session, visibleRegistry, publishedNodeType, nodeId, { 320.0, 260.0 });
}

C5ModulePublishProofRunResult runModulePublishProof (const C5ModulePublishProofRunRequest& request,
                                                     const C5ModulePublishProofDefinition& definition)
{
    auto result = makeInitialResult (request, definition);
    const PublishModuleResult emptyPublish;

    const auto writeReport = [&] (bool ok,
                                  const PublishModuleResult& publish,
                                  bool packageReloaded,
                                  bool libraryReloaded,
                                  bool visibleRegistryContainsPublishedNode,
                                  bool runtimeRegistryContainsPublishedNodeResult,
                                  const std::string& runtimeCoverageStatus,
                                  bool createdPublishedNode,
                                  const std::string& graphCommandLogStatus,
                                  const std::string& error)
    {
        return writeProofTextFile (result.reportPath,
                                   makeC5ModulePublishReportJson (ok,
                                                                  publish,
                                                                  packageReloaded,
                                                                  libraryReloaded,
                                                                  visibleRegistryContainsPublishedNode,
                                                                  runtimeRegistryContainsPublishedNodeResult,
                                                                  runtimeCoverageStatus,
                                                                  createdPublishedNode,
                                                                  graphCommandLogStatus,
                                                                  error));
    };

    const auto fail = [&] (const PublishModuleResult& publish,
                           const std::string& message,
                           const std::string& runtimeCoverageStatus = {})
    {
        const auto writeError = writeReport (false, publish, false, false, false, false, runtimeCoverageStatus,
                                             false, {}, message);
        result.ok = false;
        result.status = "failed";
        result.error = writeError.empty() ? message : writeError;
        return result;
    };

    if (const auto error = clearProofDirectoryIfExists (request.outputDirectory); ! error.empty())
        return fail (emptyPublish, error);

    if (const auto error = createProofDirectoryIfMissing (request.outputDirectory); ! error.empty())
        return fail (emptyPublish, error);

    const auto workManifestPath = firstLoadableWorkManifestPath (request);
    if (workManifestPath.empty())
        return fail (emptyPublish, "could not load C2 work fixture");

    const auto loadedPatch = loadMainPatchDocumentForWork (workManifestPath);
    if (! loadedPatch.ok)
        return fail (emptyPublish, loadedPatch.error);

    auto sourceSession = makeGraphSession (loadedPatch.document.graph);
    const auto publish = publishModule (
        sourceSession,
        makeModulePublishProofRequest (workManifestPath,
                                       request.outputDirectory / "modules" / "published-loudness",
                                       request.outputDirectory / "module-libraries" / "published.module-library.json"));
    if (! publish.ok)
        return fail (publish, publish.error);

    const auto package = loadModulePackageManifest (publish.moduleManifestPath);
    const auto library = loadModuleLibraryManifest (publish.targetLibraryPath);
    const auto loadedSpecs = loadCompoundModuleNodeSpecsFromLibrary (publish.targetLibraryPath);
    const auto runtime = loadRuntimeRegistryFromModuleLibrary (publish.targetLibraryPath);
    const auto coverage = runtime.ok ? inspectRuntimeOpCoverage (runtime.registry) : RuntimeOpCoverageResult {};
    const auto visibleRegistryContainsPublishedNode = containsPublishedNodeSpec (loadedSpecs, publish.publishedNodeType);
    const auto runtimeRegistryContainsPublishedNodeResult = runtimeRegistryContainsPublishedNode (
        runtime, publish.publishedNodeType);
    const auto runtimeCoverageStatus = runtimeCoverageStatusForPublishedNode (coverage, publish.publishedNodeType);

    auto reuseSession = makeGraphSession (makeDefaultShaderOutputGraph());
    const auto createResult = createPublishedNode (loadedSpecs, publish.publishedNodeType, "published_loud1",
                                                   reuseSession);
    const auto graphCommandLogStatus = reuseSession.commandLog.empty() ? std::string {}
                                                                       : reuseSession.commandLog.back();
    const auto createdPublishedNode = createResult.ok && graphCommandLogStatus == "create_node";
    const auto ok = publish.ok
                    && package.ok
                    && library.ok
                    && visibleRegistryContainsPublishedNode
                    && runtimeRegistryContainsPublishedNodeResult
                    && runtimeCoverageStatus == "ready"
                    && createdPublishedNode;
    const auto error = ok ? std::string {}
                          : ! package.ok ? package.error
                          : ! library.ok ? library.error
                          : ! loadedSpecs.ok ? loadedSpecs.error
                          : ! runtime.ok ? runtime.error
                          : ! coverage.ok ? coverage.error
                          : ! createResult.ok ? createResult.message
                          : "C5 module publish proof did not match expected publish/reuse evidence";

    if (const auto writeError = writeReport (ok,
                                             publish,
                                             package.ok,
                                             library.ok,
                                             visibleRegistryContainsPublishedNode,
                                             runtimeRegistryContainsPublishedNodeResult,
                                             runtimeCoverageStatus,
                                             createdPublishedNode,
                                             graphCommandLogStatus,
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

C5ModulePublishProofRunResult runAIWorkerModulePublishProof (const C5ModulePublishProofRunRequest& request,
                                                             const C5ModulePublishProofDefinition& definition)
{
    auto result = makeInitialResult (request, definition);
    const auto allowedOperations = allowedAIWorkerOperations();
    const auto allowedPublishModule = std::find (allowedOperations.begin(),
                                                 allowedOperations.end(),
                                                 "publish_module") != allowedOperations.end();
    auto aiRequest = makeAIWorkerModulePublishProofRequest (
        request.outputDirectory / "modules" / "ai-published-loudness",
        request.outputDirectory / "module-libraries" / "ai-published.module-library.json");
    const AIWorkerCommandResult emptyResult;

    const auto writeReport = [&] (bool ok,
                                  const AIWorkerCommandRequest& commandRequest,
                                  const AIWorkerCommandResult& commandResult,
                                  size_t collaborationLogEntries,
                                  const std::string& collaborationProofEvidence,
                                  const std::string& aiCommandLogStatus,
                                  const std::string& error)
    {
        return writeProofTextFile (result.reportPath,
                                   makeC5AIWorkerModulePublishReportJson (ok,
                                                                           allowedPublishModule,
                                                                           commandRequest,
                                                                           commandResult,
                                                                           collaborationLogEntries,
                                                                           collaborationProofEvidence,
                                                                           aiCommandLogStatus,
                                                                           error));
    };

    const auto fail = [&] (const AIWorkerCommandResult& commandResult, const std::string& message)
    {
        const auto writeError = writeReport (false, aiRequest, commandResult, 0, {}, {}, message);
        result.ok = false;
        result.status = "failed";
        result.error = writeError.empty() ? message : writeError;
        return result;
    };

    if (const auto error = clearProofDirectoryIfExists (request.outputDirectory); ! error.empty())
        return fail (emptyResult, error);

    if (const auto error = createProofDirectoryIfMissing (request.outputDirectory); ! error.empty())
        return fail (emptyResult, error);

    const auto workManifestPath = firstLoadableWorkManifestPath (request);
    if (workManifestPath.empty())
        return fail (emptyResult, "could not load C2 work fixture");

    const auto loadedPatch = loadMainPatchDocumentForWork (workManifestPath);
    if (! loadedPatch.ok)
        return fail (emptyResult, loadedPatch.error);

    aiRequest.workManifestPath = workManifestPath;
    auto session = makeGraphSession (loadedPatch.document.graph);
    const auto commandResult = executeAIWorkerCommand (session, aiRequest);
    const auto aiCommandLogStatus = session.commandLog.empty() ? std::string {} : session.commandLog.back();
    const auto collaborationProofEvidence = session.collaborationLog.empty() ? std::string {}
                                                                             : session.collaborationLog.back().proofEvidence;
    const auto collaborationLogOk = session.collaborationLog.size() >= 2
                                    && session.collaborationLog.front().operation == "publish_module"
                                    && session.collaborationLog.front().status == "requested"
                                    && session.collaborationLog.back().operation == "publish_module"
                                    && session.collaborationLog.back().status == "published"
                                    && collaborationProofEvidence.find ("publishCommandLogStatus=publish_module:published")
                                        != std::string::npos
                                    && collaborationProofEvidence.find ("packageReloaded=true") != std::string::npos
                                    && collaborationProofEvidence.find ("libraryReloaded=true") != std::string::npos;
    const auto ok = allowedPublishModule
                    && commandResult.ok
                    && commandResult.operation == "publish_module"
                    && commandResult.status == "published"
                    && commandResult.evidence.publishCommandLogStatus == "publish_module:published"
                    && commandResult.evidence.packageReloaded
                    && commandResult.evidence.libraryReloaded
                    && aiCommandLogStatus == "ai_worker:publish_module:published"
                    && collaborationLogOk;
    const auto error = ok ? std::string {}
                          : ! commandResult.ok ? commandResult.error
                          : "C5 AI worker publish_module proof did not match expected command/collaboration evidence";

    if (const auto writeError = writeReport (ok,
                                             aiRequest,
                                             commandResult,
                                             session.collaborationLog.size(),
                                             collaborationProofEvidence,
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

C5ModulePublishProofRunResult runVisibleModulePublishProof (const C5ModulePublishProofRunRequest& request,
                                                            const C5ModulePublishProofDefinition& definition)
{
    auto result = makeInitialResult (request, definition);
    const PublishModuleResult emptyPublish;
    const auto publishDirectory = request.outputDirectory.parent_path() / "c5-visible-module-publish";

    const auto writeReport = [&] (bool ok,
                                  const PublishModuleResult& publish,
                                  bool packageReloaded,
                                  bool libraryReloaded,
                                  bool visibleRegistryContainsPublishedNode,
                                  bool createdPublishedNode,
                                  const std::string& graphCommandLogStatus,
                                  const std::string& error)
    {
        return writeProofTextFile (result.reportPath,
                                   makeC5VisibleModulePublishReportJson (ok,
                                                                         publish,
                                                                         packageReloaded,
                                                                         libraryReloaded,
                                                                         visibleRegistryContainsPublishedNode,
                                                                         createdPublishedNode,
                                                                         graphCommandLogStatus,
                                                                         error));
    };

    const auto fail = [&] (const PublishModuleResult& publish, const std::string& message)
    {
        const auto writeError = writeReport (false, publish, false, false, false, false, {}, message);
        result.ok = false;
        result.status = "failed";
        result.error = writeError.empty() ? message : writeError;
        return result;
    };

    if (const auto error = clearProofDirectoryIfExists (request.outputDirectory); ! error.empty())
        return fail (emptyPublish, error);

    if (const auto error = clearProofDirectoryIfExists (publishDirectory); ! error.empty())
        return fail (emptyPublish, error);

    if (const auto error = createProofDirectoryIfMissing (request.outputDirectory); ! error.empty())
        return fail (emptyPublish, error);

    const auto workManifestPath = firstLoadableWorkManifestPath (request);
    if (workManifestPath.empty())
        return fail (emptyPublish, "could not load C2 work fixture");

    auto session = makeGraphSession (makeDefaultShaderOutputGraph());
    const auto createSource = createNode (session, "compound.loudness", "loud1", { 220.0, 260.0 });
    if (! createSource.ok)
        return fail (emptyPublish, createSource.message);

    const auto publish = publishModule (
        session,
        makeVisibleModulePublishProofRequest (workManifestPath, "loud1", publishDirectory));
    const auto package = publish.ok ? loadModulePackageManifest (publish.moduleManifestPath) : ModulePackageLoadResult {};
    const auto library = publish.ok ? loadModuleLibraryManifest (publish.targetLibraryPath) : ModuleLibraryLoadResult {};
    const auto loadedSpecs = publish.ok ? loadCompoundModuleNodeSpecsFromLibrary (publish.targetLibraryPath)
                                        : CompoundModuleNodeSpecsResult {};
    const auto packageReloaded = package.ok && std::filesystem::exists (publish.compoundPatchPath);
    const auto libraryReloaded = library.ok;
    const auto visibleRegistryContainsPublishedNode = containsPublishedNodeSpec (loadedSpecs, publish.publishedNodeType);

    auto reuseSession = makeGraphSession (makeDefaultShaderOutputGraph());
    const auto createResult = createPublishedNode (loadedSpecs, publish.publishedNodeType, "visible_published_loud1",
                                                   reuseSession);
    const auto graphCommandLogStatus = reuseSession.commandLog.empty() ? std::string {}
                                                                       : reuseSession.commandLog.back();
    const auto createdPublishedNode = createResult.ok && graphCommandLogStatus == "create_node";
    const auto ok = publish.ok
                    && publish.status == "published"
                    && packageReloaded
                    && libraryReloaded
                    && visibleRegistryContainsPublishedNode
                    && createdPublishedNode;
    const auto error = ok ? std::string {}
                          : ! publish.ok ? publish.error
                          : ! package.ok ? package.error
                          : ! library.ok ? library.error
                          : ! loadedSpecs.ok ? loadedSpecs.error
                          : ! createResult.ok ? createResult.message
                          : "C5 visible publish proof did not match expected publish/reuse evidence";

    if (const auto writeError = writeReport (ok,
                                             publish,
                                             packageReloaded,
                                             libraryReloaded,
                                             visibleRegistryContainsPublishedNode,
                                             createdPublishedNode,
                                             graphCommandLogStatus,
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

const char* c5ModulePublishProofDisplayName (C5ModulePublishProofKind kind)
{
    return definitionFor (kind).displayName;
}

const char* c5ModulePublishProofDirectoryName (C5ModulePublishProofKind kind)
{
    return definitionFor (kind).directoryName;
}

C5ModulePublishProofRunResult runC5ModulePublishProof (const C5ModulePublishProofRunRequest& request)
{
    const auto& definition = definitionFor (request.kind);

    switch (request.kind)
    {
        case C5ModulePublishProofKind::modulePublish:
            return runModulePublishProof (request, definition);
        case C5ModulePublishProofKind::aiWorkerModulePublish:
            return runAIWorkerModulePublishProof (request, definition);
        case C5ModulePublishProofKind::visibleModulePublish:
            return runVisibleModulePublishProof (request, definition);
    }

    return runModulePublishProof (request, definition);
}
}
