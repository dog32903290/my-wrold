#pragma once

#include "RuntimeRegistry.h"

#include <map>
#include <string>
#include <vector>

namespace myworld
{
namespace runtime_registry_internal
{
using RuntimeValueBus = std::map<std::string, RuntimeOutputValue>;
using RuntimeSampleBus = std::map<std::string, std::vector<float>>;

struct SyntheticRuntimeOpContext
{
    const RuntimeRegistryEntry& entry;
    const RuntimeRegistryChild& child;
    const RuntimeSyntheticAudioInput& input;
    size_t sampleCount = 0;
    RuntimeValueBus& valueBus;
    RuntimeSampleBus& sampleBus;
};

using SyntheticRuntimeOpFunction = void (*) (SyntheticRuntimeOpContext&, RuntimeChildExecutionStatus&);

struct SyntheticRuntimeOpDefinition
{
    const char* nodeType = "";
    const char* id = "";
    SyntheticRuntimeOpFunction execute = nullptr;
};

const std::vector<SyntheticRuntimeOpDefinition>& syntheticRuntimeOps();
const SyntheticRuntimeOpDefinition* findSyntheticRuntimeOp (const std::string& nodeType);
const RuntimeRegistryChild* findRuntimeChild (const RuntimeRegistryEntry& entry, const std::string& childId);
const RuntimeOutputValue* findOutputValue (const std::vector<RuntimeOutputValue>& values, const std::string& id);
std::string makeMissingRuntimeOpReason (const std::string& nodeType);
std::string makeMissingRuntimeOpError (const std::string& phase,
                                       const RuntimeRegistryEntry& entry,
                                       const RuntimeRegistryChild& child);
}
}
