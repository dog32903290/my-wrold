#include "AnalyzerVisibleCatalog.h"

#include "CompoundModule.h"
#include "InteractionContract.h"
#include "JsonWriter.h"
#include "NodeSpecQueries.h"
#include "RuntimeRegistry.h"

#include <algorithm>
#include <ostream>
#include <sstream>

namespace myworld
{
namespace
{
const RuntimeOpModuleDiagnostic* findDiagnostic (const std::vector<RuntimeOpModuleDiagnostic>& diagnostics,
                                                 const std::string& nodeType)
{
    for (const auto& diagnostic : diagnostics)
        if (diagnostic.nodeType == nodeType)
            return &diagnostic;

    return nullptr;
}

std::string firstProofError (const AnalyzerVisibleCatalogProof& proof)
{
    for (const auto& node : proof.nodes)
    {
        if (! node.visible)
            return "missing visible node spec: " + node.nodeType;

        if (! node.runtimeReady)
            return "runtime diagnostics not ready: " + node.nodeType;

        if (! node.created)
            return node.createError.empty() ? "createNode failed: " + node.nodeType
                                            : node.createError;
    }

    return "PV-B1 analyzer environment proof did not match expected visible catalog evidence";
}

void appendNodeProofJson (std::ostream& out, const AnalyzerVisibleCatalogNodeProof& node)
{
    out << "    {\n";
    out << "      \"nodeType\": " << jsonQuoted (node.nodeType) << ",\n";
    out << "      \"displayName\": " << jsonQuoted (node.displayName) << ",\n";
    out << "      \"category\": " << jsonQuoted (node.category) << ",\n";
    out << "      \"subcategory\": " << jsonQuoted (node.subcategory) << ",\n";
    out << "      \"inputCount\": " << node.inputCount << ",\n";
    out << "      \"outputCount\": " << node.outputCount << ",\n";
    out << "      \"paramCount\": " << node.paramCount << ",\n";
    out << "      \"runtimeCoverageStatus\": " << jsonQuoted (node.runtimeCoverageStatus) << ",\n";
    out << "      \"visible\": " << (node.visible ? "true" : "false") << ",\n";
    out << "      \"runtimeReady\": " << (node.runtimeReady ? "true" : "false") << ",\n";
    out << "      \"created\": " << (node.created ? "true" : "false") << ",\n";
    out << "      \"createCommandLogStatus\": " << jsonQuoted (node.createCommandLogStatus) << ",\n";
    out << "      \"createError\": " << jsonQuoted (node.createError) << "\n";
    out << "    }";
}
}

std::vector<std::string> pvB1AnalyzerRequiredNodeTypes()
{
    return {
        "compound.loudness",
        "compound.raw-energy",
        "compound.attack",
        "compound.density",
        "compound.silence",
        "compound.sustain",
        "compound.residue",
        "compound.aggregate-pressure"
    };
}

AnalyzerVisibleCatalogProof proveAnalyzerVisibleCatalog (const std::string& libraryPath)
{
    AnalyzerVisibleCatalogProof proof;
    proof.libraryPath = libraryPath;
    proof.requiredNodeTypes = pvB1AnalyzerRequiredNodeTypes();

    const auto loadedSpecs = loadCompoundModuleNodeSpecsFromLibrary (libraryPath);
    if (! loadedSpecs.ok)
    {
        proof.error = loadedSpecs.error;
        return proof;
    }

    proof.loadedModuleNodeCount = loadedSpecs.specs.size();
    const auto visibleRegistry = mergeNodeSpecs (makeSeedNodeSpecs(), loadedSpecs.specs);
    proof.visibleNodeCount = visibleRegistry.size();

    const auto runtime = loadRuntimeRegistryFromModuleLibrary (libraryPath);
    if (! runtime.ok)
    {
        proof.error = runtime.error;
        return proof;
    }

    const auto coverage = inspectRuntimeOpCoverage (runtime.registry);
    if (! coverage.ok)
    {
        proof.error = coverage.error;
        return proof;
    }

    const auto diagnostics = makeRuntimeOpModuleDiagnostics (coverage.snapshot);
    auto session = makeGraphSession (makeDefaultShaderOutputGraph());

    for (size_t index = 0; index < proof.requiredNodeTypes.size(); ++index)
    {
        const auto& nodeType = proof.requiredNodeTypes[index];
        AnalyzerVisibleCatalogNodeProof node;
        node.nodeType = nodeType;

        if (const auto* spec = findNodeSpec (visibleRegistry, nodeType))
        {
            node.visible = true;
            node.displayName = spec->displayName;
            node.category = spec->category;
            node.subcategory = spec->subcategory;
            node.inputCount = spec->inputs.size();
            node.outputCount = spec->outputs.size();
            node.paramCount = spec->params.size();
        }

        if (const auto* diagnostic = findDiagnostic (diagnostics, nodeType))
        {
            node.runtimeCoverageStatus = diagnostic->status;
            node.runtimeReady = diagnostic->status == "runtime-op-ready"
                                && runtimeOpDiagnosticAllowsCreation (*diagnostic);
        }

        if (node.visible && node.runtimeReady)
        {
            const auto nodeId = makeNodeIdStem (nodeType) + "_pv_b1_" + std::to_string (index + 1);
            const auto createResult = createNode (session,
                                                  visibleRegistry,
                                                  nodeType,
                                                  nodeId,
                                                  { 220.0 + static_cast<double> (index * 36),
                                                    280.0 + static_cast<double> (index * 8) });
            node.createCommandLogStatus = session.commandLog.empty() ? std::string {}
                                                                     : session.commandLog.back();
            node.created = createResult.ok && node.createCommandLogStatus == "create_node";
            node.createError = createResult.ok ? std::string {} : createResult.message;
        }

        if (node.visible)
            proof.visibleCatalogContainsAllRequired = true;
        else
            proof.visibleCatalogContainsAllRequired = false;

        proof.nodes.push_back (std::move (node));
    }

    proof.visibleCatalogContainsAllRequired = std::all_of (proof.nodes.begin(), proof.nodes.end(), [] (const auto& node) {
        return node.visible;
    });
    proof.runtimeDiagnosticsReadyForAllRequired = std::all_of (proof.nodes.begin(), proof.nodes.end(), [] (const auto& node) {
        return node.runtimeReady;
    });
    proof.createdNodeCount = static_cast<size_t> (std::count_if (proof.nodes.begin(), proof.nodes.end(), [] (const auto& node) {
        return node.created;
    }));

    proof.ok = proof.loadedModuleNodeCount == proof.requiredNodeTypes.size()
               && proof.visibleCatalogContainsAllRequired
               && proof.runtimeDiagnosticsReadyForAllRequired
               && proof.createdNodeCount == proof.requiredNodeTypes.size()
               && proof.usesExistingAnalyzerRuntimeProofs
               && ! proof.addsNewAnalyzerDSP
               && ! proof.usesMidiMapping
               && ! proof.usesShaderUniformMapping
               && ! proof.usesLiveCallbackRuntime;
    proof.error = proof.ok ? std::string {} : firstProofError (proof);
    return proof;
}

std::string makeAnalyzerVisibleCatalogProofJson (const AnalyzerVisibleCatalogProof& proof)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"pvB1AnalyzerEnvironmentProof\",\n";
    out << "  \"ok\": " << (proof.ok ? "true" : "false") << ",\n";
    out << "  \"operation\": " << jsonQuoted (proof.operation) << ",\n";
    out << "  \"libraryPath\": " << jsonQuoted (proof.libraryPath) << ",\n";
    out << "  \"requiredNodeTypes\": ";
    appendJsonStringArray (out, proof.requiredNodeTypes);
    out << ",\n";
    out << "  \"loadedModuleNodeCount\": " << proof.loadedModuleNodeCount << ",\n";
    out << "  \"visibleNodeCount\": " << proof.visibleNodeCount << ",\n";
    out << "  \"visibleCatalogContainsAllRequired\": "
        << (proof.visibleCatalogContainsAllRequired ? "true" : "false") << ",\n";
    out << "  \"runtimeDiagnosticsReadyForAllRequired\": "
        << (proof.runtimeDiagnosticsReadyForAllRequired ? "true" : "false") << ",\n";
    out << "  \"createdNodeCount\": " << proof.createdNodeCount << ",\n";
    out << "  \"usesExistingAnalyzerRuntimeProofs\": "
        << (proof.usesExistingAnalyzerRuntimeProofs ? "true" : "false") << ",\n";
    out << "  \"addsNewAnalyzerDSP\": " << (proof.addsNewAnalyzerDSP ? "true" : "false") << ",\n";
    out << "  \"usesMidiMapping\": " << (proof.usesMidiMapping ? "true" : "false") << ",\n";
    out << "  \"usesShaderUniformMapping\": " << (proof.usesShaderUniformMapping ? "true" : "false") << ",\n";
    out << "  \"usesLiveCallbackRuntime\": " << (proof.usesLiveCallbackRuntime ? "true" : "false") << ",\n";
    out << "  \"nodes\": [\n";
    for (size_t index = 0; index < proof.nodes.size(); ++index)
    {
        appendNodeProofJson (out, proof.nodes[index]);
        if (index + 1 < proof.nodes.size())
            out << ",";
        out << "\n";
    }
    out << "  ],\n";
    out << "  \"error\": " << jsonQuoted (proof.error) << "\n";
    out << "}\n";
    return out.str();
}
}
