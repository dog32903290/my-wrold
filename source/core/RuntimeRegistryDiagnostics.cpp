#include "RuntimeRegistryInternals.h"

#include <algorithm>
#include <sstream>

namespace myworld
{
using namespace runtime_registry_internal;

namespace
{
std::vector<std::string> missingNodeTypesFor (const RuntimeEntryCoverageStatus& entry)
{
    std::vector<std::string> missing;

    for (const auto& child : entry.children)
    {
        if (child.status != "missing-runtime-op")
            continue;

        if (std::find (missing.begin(), missing.end(), child.nodeType) == missing.end())
            missing.push_back (child.nodeType);
    }

    return missing;
}

std::string joinedNodeTypes (const std::vector<std::string>& nodeTypes)
{
    std::ostringstream text;

    for (size_t index = 0; index < nodeTypes.size(); ++index)
    {
        if (index != 0)
            text << ", ";

        text << nodeTypes[index];
    }

    return text.str();
}
}

std::vector<RuntimeOpCatalogEntry> makeRuntimeOpCatalog()
{
    std::vector<RuntimeOpCatalogEntry> catalog;
    catalog.reserve (syntheticRuntimeOps().size());

    for (const auto& op : syntheticRuntimeOps())
        catalog.push_back ({ op.nodeType, op.id });

    return catalog;
}

RuntimeOpCoverageResult inspectRuntimeOpCoverage (const RuntimeRegistry& registry)
{
    RuntimeOpCoverageSnapshot snapshot;
    snapshot.version = registry.version;
    snapshot.catalog = makeRuntimeOpCatalog();

    bool ok = true;
    std::string firstError;

    for (const auto& entry : registry.entries)
    {
        RuntimeEntryCoverageStatus entryStatus;
        entryStatus.nodeType = entry.nodeType;
        entryStatus.executionKind = entry.executionKind;
        entryStatus.status = "runtime-op-covered";
        entryStatus.children.reserve (entry.cookOrder.size());

        for (size_t cookIndex = 0; cookIndex < entry.cookOrder.size(); ++cookIndex)
        {
            const auto& childId = entry.cookOrder[cookIndex];
            const auto* child = findRuntimeChild (entry, childId);

            if (child == nullptr)
                return { false, snapshot, "coverage missing child metadata for " + entry.nodeType + ":" + childId };

            const auto* runtimeOp = findSyntheticRuntimeOp (child->nodeType);

            if (runtimeOp == nullptr)
            {
                ok = false;
                entryStatus.status = "missing-runtime-op";
                ++entryStatus.missingChildCount;
                ++snapshot.missingChildCount;

                if (firstError.empty())
                    firstError = makeMissingRuntimeOpError ("coverage", entry, *child);

                entryStatus.children.push_back ({ cookIndex,
                                                  child->id,
                                                  child->nodeType,
                                                  child->role,
                                                  {},
                                                  "missing-runtime-op",
                                                  makeMissingRuntimeOpReason (child->nodeType) });
                continue;
            }

            ++entryStatus.supportedChildCount;
            ++snapshot.supportedChildCount;
            entryStatus.children.push_back ({ cookIndex,
                                              child->id,
                                              child->nodeType,
                                              child->role,
                                              runtimeOp->id,
                                              "supported-runtime-op",
                                              "RuntimeOp is registered; execution not run" });
        }

        if (entryStatus.children.size() != entry.childCount)
            return { false, snapshot, "coverage child count mismatch for " + entry.nodeType };

        snapshot.entries.push_back (entryStatus);
    }

    return { ok, snapshot, ok ? std::string {} : firstError };
}

std::vector<RuntimeOpModuleDiagnostic> makeRuntimeOpModuleDiagnostics (const RuntimeOpCoverageSnapshot& snapshot)
{
    std::vector<RuntimeOpModuleDiagnostic> diagnostics;
    diagnostics.reserve (snapshot.entries.size());

    for (const auto& entry : snapshot.entries)
    {
        RuntimeOpModuleDiagnostic diagnostic;
        diagnostic.nodeType = entry.nodeType;
        diagnostic.supportedChildCount = entry.supportedChildCount;
        diagnostic.missingChildCount = entry.missingChildCount;
        diagnostic.missingNodeTypes = missingNodeTypesFor (entry);

        if (entry.missingChildCount == 0)
        {
            diagnostic.status = "runtime-op-ready";
            diagnostic.browserLabel = "runtime ready";
            diagnostic.inspectorDetail = std::to_string (entry.supportedChildCount)
                                       + " RuntimeOps registered; execution not run";
            diagnostic.creationStatus = "create-enabled";
            diagnostic.creationLabel = "create";
        }
        else
        {
            diagnostic.status = "missing-runtime-op";
            diagnostic.browserLabel = "missing RuntimeOp";
            diagnostic.inspectorDetail = "missing RuntimeOp: " + joinedNodeTypes (diagnostic.missingNodeTypes);
            diagnostic.creationStatus = "create-blocked";
            diagnostic.creationLabel = "blocked";
            diagnostic.creationBlockReason = diagnostic.inspectorDetail;
        }

        diagnostics.push_back (std::move (diagnostic));
    }

    return diagnostics;
}

bool runtimeOpDiagnosticAllowsCreation (const RuntimeOpModuleDiagnostic& diagnostic)
{
    return diagnostic.creationStatus != "create-blocked";
}

RuntimeDryRunResult dryRunRuntimeRegistry (const RuntimeRegistry& registry)
{
    RuntimeDryRunSnapshot snapshot;
    snapshot.version = registry.version;

    for (const auto& entry : registry.entries)
    {
        RuntimeEntryDryRunStatus entryStatus;
        entryStatus.nodeType = entry.nodeType;
        entryStatus.executionKind = entry.executionKind;
        entryStatus.status = "dry-run-ready";
        entryStatus.children.reserve (entry.cookOrder.size());

        for (size_t cookIndex = 0; cookIndex < entry.cookOrder.size(); ++cookIndex)
        {
            const auto& childId = entry.cookOrder[cookIndex];
            const auto* child = findRuntimeChild (entry, childId);

            if (child == nullptr)
                return { false, {}, "dry-run missing child metadata for " + entry.nodeType + ":" + childId };

            const auto* runtimeOp = findSyntheticRuntimeOp (child->nodeType);

            if (runtimeOp == nullptr)
            {
                entryStatus.status = "missing-runtime-op";
                entryStatus.children.push_back ({ cookIndex,
                                                 child->id,
                                                 child->nodeType,
                                                 child->role,
                                                 {},
                                                 "missing-runtime-op",
                                                 makeMissingRuntimeOpReason (child->nodeType) });
                snapshot.entries.push_back (entryStatus);
                return { false, snapshot, makeMissingRuntimeOpError ("dry-run", entry, *child) };
            }

            entryStatus.children.push_back ({ cookIndex,
                                             child->id,
                                             child->nodeType,
                                             child->role,
                                             runtimeOp->id,
                                             "dry-run-ready",
                                             "validated child order and RuntimeOp coverage; RuntimeOp not executed" });
        }

        if (entryStatus.children.size() != entry.childCount)
            return { false, {}, "dry-run child count mismatch for " + entry.nodeType };

        snapshot.entries.push_back (entryStatus);
    }

    return { true, snapshot, {} };
}
}
