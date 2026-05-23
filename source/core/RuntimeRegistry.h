#pragma once

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
    std::vector<std::string> publicInputs;
    std::vector<std::string> publicOutputs;
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
};

struct RuntimeChildExecutionStatus
{
    size_t cookIndex = 0;
    std::string childId;
    std::string nodeType;
    std::string role;
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

struct RuntimeSyntheticAudioInput
{
    std::vector<std::vector<float>> channels;
    float analysisGain = 1.0f;
};

RuntimeRegistryLoadResult loadRuntimeRegistryFromModuleLibrary (const std::string& libraryPath);
RuntimeDryRunResult dryRunRuntimeRegistry (const RuntimeRegistry& registry);
RuntimeExecutionResult executeRuntimeRegistryWithSyntheticAudio (const RuntimeRegistry& registry,
                                                                 const std::vector<float>& samples,
                                                                 float analysisGain);
RuntimeExecutionResult executeRuntimeRegistryWithSyntheticAudio (const RuntimeRegistry& registry,
                                                                 const RuntimeSyntheticAudioInput& input);
std::string makeRuntimeRegistryJson (const RuntimeRegistry& registry);
std::string makeRuntimeDryRunJson (const RuntimeDryRunSnapshot& snapshot);
std::string makeRuntimeExecutionJson (const RuntimeExecutionSnapshot& snapshot);
}
