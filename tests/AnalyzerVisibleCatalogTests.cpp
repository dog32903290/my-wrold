#include "AnalyzerVisibleCatalog.h"

#include <cstdlib>
#include <iostream>
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

void expectEqual (const std::string& actual, const std::string& expected, const std::string& message)
{
    expect (actual == expected, message + " expected " + expected + " got " + actual);
}

const myworld::AnalyzerVisibleCatalogNodeProof* findNode (
    const myworld::AnalyzerVisibleCatalogProof& proof,
    const std::string& nodeType)
{
    for (const auto& node : proof.nodes)
        if (node.nodeType == nodeType)
            return &node;

    return nullptr;
}
}

int main()
{
    const std::string libraryPath = "fixtures/module-libraries/pv-analyzer-visible.module-library.json";
    const auto proof = myworld::proveAnalyzerVisibleCatalog (libraryPath);

    expect (proof.ok, proof.error);
    expectEqual (proof.operation, "pv_b1_analyzer_environment_promotion", "operation");
    expectEqual (proof.libraryPath, libraryPath, "library path");
    expect (proof.requiredNodeTypes.size() == 8, "required node type count");
    expect (proof.loadedModuleNodeCount == 8, "loaded module node count");
    expect (proof.visibleCatalogContainsAllRequired, "visible catalog contains all required nodes");
    expect (proof.runtimeDiagnosticsReadyForAllRequired, "runtime diagnostics ready for all required nodes");
    expect (proof.createdNodeCount == proof.requiredNodeTypes.size(), "created node count");
    expect (proof.usesExistingAnalyzerRuntimeProofs, "uses existing analyzer runtime proofs");
    expect (! proof.addsNewAnalyzerDSP, "does not add analyzer DSP");
    expect (! proof.usesMidiMapping, "does not use MIDI mapping");
    expect (! proof.usesShaderUniformMapping, "does not use shader uniform mapping");
    expect (! proof.usesLiveCallbackRuntime, "does not use live callback runtime");

    for (const auto& nodeType : proof.requiredNodeTypes)
    {
        const auto* node = findNode (proof, nodeType);
        expect (node != nullptr, "node proof exists for " + nodeType);
        expect (node->visible, "node visible " + nodeType);
        expect (node->runtimeReady, "node runtime ready " + nodeType);
        expect (node->created, "node created " + nodeType);
        expectEqual (node->createCommandLogStatus, "create_node", "create command for " + nodeType);
        expect (node->inputCount + node->outputCount > 0, "node has visible ports " + nodeType);
    }

    const auto json = myworld::makeAnalyzerVisibleCatalogProofJson (proof);
    expect (json.find ("\"kind\": \"pvB1AnalyzerEnvironmentProof\"") != std::string::npos, "json kind");
    expect (json.find ("\"visibleCatalogContainsAllRequired\": true") != std::string::npos, "json visible proof");
    expect (json.find ("\"usesShaderUniformMapping\": false") != std::string::npos, "json shader flag");

    std::cout << "analyzer visible catalog ok\n";
    return 0;
}
