#pragma once

#include "AudioAnalyzerState.h"

#include <cstddef>
#include <string>
#include <vector>

namespace myworld
{
struct RuntimeRegistryChild
{
    std::string id;
    std::string nodeType;
    std::string role;
};

struct RuntimeRegistryEdge
{
    std::string from;
    std::string to;
    std::string dataType;
};

struct RuntimeRegistryPublicOutputMapping
{
    std::string id;
    std::string mapsTo;
};

struct RuntimeRegistryEntry
{
    std::string nodeType;
    std::string displayName;
    std::string runtimeDomain;
    std::string executionKind;
    std::string previewPolicy;
    std::vector<RuntimeRegistryChild> children;
    size_t childCount = 0;
    size_t internalEdgeCount = 0;
    std::vector<RuntimeRegistryEdge> internalEdges;
    std::vector<std::string> publicInputs;
    std::vector<std::string> publicOutputs;
    std::vector<RuntimeRegistryPublicOutputMapping> publicOutputMappings;
    std::vector<std::string> cookOrder;
};

struct RuntimeRegistry
{
    int version = 1;
    std::vector<RuntimeRegistryEntry> entries;
};

struct RuntimeRegistryLoadResult
{
    bool ok = false;
    RuntimeRegistry registry;
    std::string error;
};

struct RuntimeChildDryRunStatus
{
    size_t cookIndex = 0;
    std::string childId;
    std::string nodeType;
    std::string role;
    std::string runtimeOp;
    std::string status;
    std::string reason;
};

struct RuntimeEntryDryRunStatus
{
    std::string nodeType;
    std::string executionKind;
    std::string status;
    std::vector<RuntimeChildDryRunStatus> children;
};

struct RuntimeDryRunSnapshot
{
    int version = 1;
    std::string mode = "dry-run";
    std::vector<RuntimeEntryDryRunStatus> entries;
};

struct RuntimeDryRunResult
{
    bool ok = false;
    RuntimeDryRunSnapshot snapshot;
    std::string error;
};

struct RuntimeOutputValue
{
    std::string id;
    double value = 0.0;
    std::string source;
};

struct RuntimeChildExecutionStatus
{
    size_t cookIndex = 0;
    std::string childId;
    std::string nodeType;
    std::string role;
    std::string runtimeOp;
    std::string status;
    std::string reason;
    std::vector<RuntimeOutputValue> inputs;
    std::vector<RuntimeOutputValue> outputs;
};

struct RuntimeEntryExecutionStatus
{
    std::string nodeType;
    std::string executionKind;
    std::string status;
    std::vector<RuntimeOutputValue> publicOutputs;
    std::vector<RuntimeChildExecutionStatus> children;
};

struct RuntimeExecutionSnapshot
{
    int version = 1;
    std::string mode = "synthetic-audio";
    std::vector<RuntimeEntryExecutionStatus> entries;
};

struct RuntimeExecutionResult
{
    bool ok = false;
    RuntimeExecutionSnapshot snapshot;
    std::string error;
};

struct LoudnessRuntimeBridgeSnapshot
{
    bool usesLoadedRuntimeOutputs = false;
    std::string sourceMode;
    std::vector<RuntimeOutputValue> publicOutputs;
    AudioAnalyzerSnapshot analyzer;
};

struct RuntimeSyntheticAudioInput
{
    std::vector<std::vector<float>> channels;
    float analysisGain = 1.0f;
};

struct RuntimeOpCatalogEntry
{
    std::string nodeType;
    std::string runtimeOp;
};

struct RuntimeChildCoverageStatus
{
    size_t cookIndex = 0;
    std::string childId;
    std::string nodeType;
    std::string role;
    std::string runtimeOp;
    std::string status;
    std::string reason;
};

struct RuntimeEntryCoverageStatus
{
    std::string nodeType;
    std::string executionKind;
    std::string status;
    size_t supportedChildCount = 0;
    size_t missingChildCount = 0;
    std::vector<RuntimeChildCoverageStatus> children;
};

struct RuntimeOpCoverageSnapshot
{
    int version = 1;
    std::string mode = "runtime-op-coverage";
    std::vector<RuntimeOpCatalogEntry> catalog;
    size_t supportedChildCount = 0;
    size_t missingChildCount = 0;
    std::vector<RuntimeEntryCoverageStatus> entries;
};

struct RuntimeOpCoverageResult
{
    bool ok = false;
    RuntimeOpCoverageSnapshot snapshot;
    std::string error;
};

struct RuntimeOpModuleDiagnostic
{
    std::string nodeType;
    std::string status;
    std::string browserLabel;
    std::string inspectorDetail;
    std::string creationStatus;
    std::string creationLabel;
    std::string creationBlockReason;
    size_t supportedChildCount = 0;
    size_t missingChildCount = 0;
    std::vector<std::string> missingNodeTypes;
};

RuntimeRegistryLoadResult loadRuntimeRegistryFromModuleLibrary (const std::string& libraryPath);
std::vector<RuntimeOpCatalogEntry> makeRuntimeOpCatalog();
RuntimeOpCoverageResult inspectRuntimeOpCoverage (const RuntimeRegistry& registry);
std::vector<RuntimeOpModuleDiagnostic> makeRuntimeOpModuleDiagnostics (const RuntimeOpCoverageSnapshot& snapshot);
bool runtimeOpDiagnosticAllowsCreation (const RuntimeOpModuleDiagnostic& diagnostic);
RuntimeDryRunResult dryRunRuntimeRegistry (const RuntimeRegistry& registry);
RuntimeExecutionResult executeRuntimeRegistryWithSyntheticAudio (const RuntimeRegistry& registry,
                                                                 const std::vector<float>& samples,
                                                                 float analysisGain);
RuntimeExecutionResult executeRuntimeRegistryWithSyntheticAudio (const RuntimeRegistry& registry,
                                                                 const RuntimeSyntheticAudioInput& input);
LoudnessRuntimeBridgeSnapshot makeLoudnessRuntimeBridgeSnapshot (const RuntimeExecutionSnapshot& runtimeSnapshot,
                                                                 const AudioAnalyzerSnapshot& fallbackSnapshot);
std::string makeRuntimeRegistryJson (const RuntimeRegistry& registry);
std::string makeRuntimeOpCatalogJson (const std::vector<RuntimeOpCatalogEntry>& catalog);
std::string makeRuntimeOpCoverageJson (const RuntimeOpCoverageSnapshot& snapshot);
std::string makeRuntimeOpModuleDiagnosticsJson (const std::vector<RuntimeOpModuleDiagnostic>& diagnostics);
std::string makeRuntimeDryRunJson (const RuntimeDryRunSnapshot& snapshot);
std::string makeRuntimeExecutionJson (const RuntimeExecutionSnapshot& snapshot);
std::string makeLoudnessRuntimeBridgeJson (const LoudnessRuntimeBridgeSnapshot& snapshot);
}
