#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace myworld
{
struct AnalyzerVisibleCatalogNodeProof
{
    std::string nodeType;
    std::string displayName;
    std::string category;
    std::string subcategory;
    size_t inputCount = 0;
    size_t outputCount = 0;
    size_t paramCount = 0;
    std::string runtimeCoverageStatus;
    std::string createCommandLogStatus;
    std::string createError;
    bool visible = false;
    bool runtimeReady = false;
    bool created = false;
};

struct AnalyzerVisibleCatalogProof
{
    bool ok = false;
    std::string operation = "pv_b1_analyzer_environment_promotion";
    std::string libraryPath;
    std::vector<std::string> requiredNodeTypes;
    size_t loadedModuleNodeCount = 0;
    size_t visibleNodeCount = 0;
    bool visibleCatalogContainsAllRequired = false;
    bool runtimeDiagnosticsReadyForAllRequired = false;
    size_t createdNodeCount = 0;
    std::vector<AnalyzerVisibleCatalogNodeProof> nodes;
    bool usesExistingAnalyzerRuntimeProofs = true;
    bool addsNewAnalyzerDSP = false;
    bool usesMidiMapping = false;
    bool usesShaderUniformMapping = false;
    bool usesLiveCallbackRuntime = false;
    std::string error;
};

std::vector<std::string> pvB1AnalyzerRequiredNodeTypes();
AnalyzerVisibleCatalogProof proveAnalyzerVisibleCatalog (const std::string& libraryPath);
std::string makeAnalyzerVisibleCatalogProofJson (const AnalyzerVisibleCatalogProof& proof);
}
