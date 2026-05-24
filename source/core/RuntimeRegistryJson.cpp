#include "RuntimeRegistry.h"

#include "JsonWriter.h"

#include <iomanip>
#include <sstream>

namespace myworld
{
namespace
{
void appendRuntimeOpCatalogArray (std::ostringstream& out, const std::vector<RuntimeOpCatalogEntry>& catalog)
{
    out << "[\n";

    for (size_t index = 0; index < catalog.size(); ++index)
    {
        const auto& entry = catalog[index];
        out << "    { \"nodeType\": " << jsonQuoted (entry.nodeType)
            << ", \"runtimeOp\": " << jsonQuoted (entry.runtimeOp) << " }";

        if (index + 1 < catalog.size())
            out << ",";

        out << "\n";
    }

    out << "  ]";
}

void appendValueObject (std::ostringstream& out, const std::vector<RuntimeOutputValue>& values)
{
    out << "{";

    for (size_t index = 0; index < values.size(); ++index)
    {
        if (index != 0)
            out << ",";

        out << " " << jsonQuoted (values[index].id) << ": " << values[index].value;
    }

    if (! values.empty())
        out << " ";

    out << "}";
}

void appendValueSourceObject (std::ostringstream& out, const std::vector<RuntimeOutputValue>& values)
{
    out << "{";

    bool wroteAny = false;

    for (const auto& value : values)
    {
        if (value.source.empty())
            continue;

        if (wroteAny)
            out << ",";

        out << " " << jsonQuoted (value.id) << ": " << jsonQuoted (value.source);
        wroteAny = true;
    }

    if (wroteAny)
        out << " ";

    out << "}";
}
}

std::string makeRuntimeRegistryJson (const RuntimeRegistry& registry)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"runtimeRegistry\",\n";
    out << "  \"version\": " << registry.version << ",\n";
    out << "  \"entries\": [\n";

    for (size_t index = 0; index < registry.entries.size(); ++index)
    {
        const auto& entry = registry.entries[index];
        out << "    {\n";
        out << "      \"nodeType\": " << jsonQuoted (entry.nodeType) << ",\n";
        out << "      \"displayName\": " << jsonQuoted (entry.displayName) << ",\n";
        out << "      \"runtimeDomain\": " << jsonQuoted (entry.runtimeDomain) << ",\n";
        out << "      \"executionKind\": " << jsonQuoted (entry.executionKind) << ",\n";
        out << "      \"previewPolicy\": " << jsonQuoted (entry.previewPolicy) << ",\n";
        out << "      \"childCount\": " << entry.childCount << ",\n";
        out << "      \"internalEdgeCount\": " << entry.internalEdgeCount << ",\n";
        out << "      \"children\": [\n";

        for (size_t childIndex = 0; childIndex < entry.children.size(); ++childIndex)
        {
            const auto& child = entry.children[childIndex];
            out << "        { \"id\": " << jsonQuoted (child.id)
                << ", \"nodeType\": " << jsonQuoted (child.nodeType)
                << ", \"role\": " << jsonQuoted (child.role) << " }";

            if (childIndex + 1 < entry.children.size())
                out << ",";

            out << "\n";
        }

        out << "      ],\n";
        out << "      \"internalEdges\": [\n";

        for (size_t edgeIndex = 0; edgeIndex < entry.internalEdges.size(); ++edgeIndex)
        {
            const auto& edge = entry.internalEdges[edgeIndex];
            out << "        { \"from\": " << jsonQuoted (edge.from)
                << ", \"to\": " << jsonQuoted (edge.to)
                << ", \"dataType\": " << jsonQuoted (edge.dataType) << " }";

            if (edgeIndex + 1 < entry.internalEdges.size())
                out << ",";

            out << "\n";
        }

        out << "      ],\n";
        out << "      \"publicInputs\": ";
        appendJsonStringArray (out, entry.publicInputs);
        out << ",\n";
        out << "      \"publicOutputs\": ";
        appendJsonStringArray (out, entry.publicOutputs);
        out << ",\n";
        out << "      \"publicOutputMappings\": [\n";

        for (size_t mappingIndex = 0; mappingIndex < entry.publicOutputMappings.size(); ++mappingIndex)
        {
            const auto& mapping = entry.publicOutputMappings[mappingIndex];
            out << "        { \"id\": " << jsonQuoted (mapping.id)
                << ", \"mapsTo\": " << jsonQuoted (mapping.mapsTo) << " }";

            if (mappingIndex + 1 < entry.publicOutputMappings.size())
                out << ",";

            out << "\n";
        }

        out << "      ],\n";
        out << "      \"cookOrder\": ";
        appendJsonStringArray (out, entry.cookOrder);
        out << "\n";
        out << "    }";

        if (index + 1 < registry.entries.size())
            out << ",";

        out << "\n";
    }

    out << "  ]\n";
    out << "}\n";
    return out.str();
}

std::string makeRuntimeOpCatalogJson (const std::vector<RuntimeOpCatalogEntry>& catalog)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"runtimeOpCatalog\",\n";
    out << "  \"version\": 1,\n";
    out << "  \"entries\": ";
    appendRuntimeOpCatalogArray (out, catalog);
    out << "\n";
    out << "}\n";
    return out.str();
}

std::string makeRuntimeOpCoverageJson (const RuntimeOpCoverageSnapshot& snapshot)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"runtimeOpCoverage\",\n";
    out << "  \"version\": " << snapshot.version << ",\n";
    out << "  \"mode\": " << jsonQuoted (snapshot.mode) << ",\n";
    out << "  \"supportedChildCount\": " << snapshot.supportedChildCount << ",\n";
    out << "  \"missingChildCount\": " << snapshot.missingChildCount << ",\n";
    out << "  \"catalog\": ";
    appendRuntimeOpCatalogArray (out, snapshot.catalog);
    out << ",\n";
    out << "  \"entries\": [\n";

    for (size_t entryIndex = 0; entryIndex < snapshot.entries.size(); ++entryIndex)
    {
        const auto& entry = snapshot.entries[entryIndex];
        out << "    {\n";
        out << "      \"nodeType\": " << jsonQuoted (entry.nodeType) << ",\n";
        out << "      \"executionKind\": " << jsonQuoted (entry.executionKind) << ",\n";
        out << "      \"status\": " << jsonQuoted (entry.status) << ",\n";
        out << "      \"supportedChildCount\": " << entry.supportedChildCount << ",\n";
        out << "      \"missingChildCount\": " << entry.missingChildCount << ",\n";
        out << "      \"children\": [\n";

        for (size_t childIndex = 0; childIndex < entry.children.size(); ++childIndex)
        {
            const auto& child = entry.children[childIndex];
            out << "        {\n";
            out << "          \"cookIndex\": " << child.cookIndex << ",\n";
            out << "          \"childId\": " << jsonQuoted (child.childId) << ",\n";
            out << "          \"nodeType\": " << jsonQuoted (child.nodeType) << ",\n";
            out << "          \"role\": " << jsonQuoted (child.role) << ",\n";
            out << "          \"runtimeOp\": " << jsonQuoted (child.runtimeOp) << ",\n";
            out << "          \"status\": " << jsonQuoted (child.status) << ",\n";
            out << "          \"reason\": " << jsonQuoted (child.reason) << "\n";
            out << "        }";

            if (childIndex + 1 < entry.children.size())
                out << ",";

            out << "\n";
        }

        out << "      ]\n";
        out << "    }";

        if (entryIndex + 1 < snapshot.entries.size())
            out << ",";

        out << "\n";
    }

    out << "  ]\n";
    out << "}\n";
    return out.str();
}

std::string makeRuntimeOpModuleDiagnosticsJson (const std::vector<RuntimeOpModuleDiagnostic>& diagnostics)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"runtimeOpModuleDiagnostics\",\n";
    out << "  \"version\": 1,\n";
    out << "  \"visibleIn\": [\"browser\", \"inspector\", \"leftRail\"],\n";
    out << "  \"entries\": [\n";

    for (size_t index = 0; index < diagnostics.size(); ++index)
    {
        const auto& diagnostic = diagnostics[index];
        out << "    {\n";
        out << "      \"nodeType\": " << jsonQuoted (diagnostic.nodeType) << ",\n";
        out << "      \"status\": " << jsonQuoted (diagnostic.status) << ",\n";
        out << "      \"browserLabel\": " << jsonQuoted (diagnostic.browserLabel) << ",\n";
        out << "      \"inspectorDetail\": " << jsonQuoted (diagnostic.inspectorDetail) << ",\n";
        out << "      \"creationStatus\": " << jsonQuoted (diagnostic.creationStatus) << ",\n";
        out << "      \"creationLabel\": " << jsonQuoted (diagnostic.creationLabel) << ",\n";
        out << "      \"creationBlockReason\": " << jsonQuoted (diagnostic.creationBlockReason) << ",\n";
        out << "      \"supportedChildCount\": " << diagnostic.supportedChildCount << ",\n";
        out << "      \"missingChildCount\": " << diagnostic.missingChildCount << ",\n";
        out << "      \"missingNodeTypes\": ";
        appendJsonStringArray (out, diagnostic.missingNodeTypes);
        out << "\n";
        out << "    }";

        if (index + 1 < diagnostics.size())
            out << ",";

        out << "\n";
    }

    out << "  ]\n";
    out << "}\n";
    return out.str();
}

std::string makeRuntimeDryRunJson (const RuntimeDryRunSnapshot& snapshot)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"runtimeDryRun\",\n";
    out << "  \"version\": " << snapshot.version << ",\n";
    out << "  \"mode\": " << jsonQuoted (snapshot.mode) << ",\n";
    out << "  \"entries\": [\n";

    for (size_t entryIndex = 0; entryIndex < snapshot.entries.size(); ++entryIndex)
    {
        const auto& entry = snapshot.entries[entryIndex];
        out << "    {\n";
        out << "      \"nodeType\": " << jsonQuoted (entry.nodeType) << ",\n";
        out << "      \"executionKind\": " << jsonQuoted (entry.executionKind) << ",\n";
        out << "      \"status\": " << jsonQuoted (entry.status) << ",\n";
        out << "      \"children\": [\n";

        for (size_t childIndex = 0; childIndex < entry.children.size(); ++childIndex)
        {
            const auto& child = entry.children[childIndex];
            out << "        {\n";
            out << "          \"cookIndex\": " << child.cookIndex << ",\n";
            out << "          \"childId\": " << jsonQuoted (child.childId) << ",\n";
            out << "          \"nodeType\": " << jsonQuoted (child.nodeType) << ",\n";
            out << "          \"role\": " << jsonQuoted (child.role) << ",\n";
            out << "          \"runtimeOp\": " << jsonQuoted (child.runtimeOp) << ",\n";
            out << "          \"status\": " << jsonQuoted (child.status) << ",\n";
            out << "          \"reason\": " << jsonQuoted (child.reason) << "\n";
            out << "        }";

            if (childIndex + 1 < entry.children.size())
                out << ",";

            out << "\n";
        }

        out << "      ]\n";
        out << "    }";

        if (entryIndex + 1 < snapshot.entries.size())
            out << ",";

        out << "\n";
    }

    out << "  ]\n";
    out << "}\n";
    return out.str();
}

std::string makeRuntimeExecutionJson (const RuntimeExecutionSnapshot& snapshot)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision (6);
    out << "{\n";
    out << "  \"kind\": \"runtimeExecution\",\n";
    out << "  \"version\": " << snapshot.version << ",\n";
    out << "  \"mode\": " << jsonQuoted (snapshot.mode) << ",\n";
    out << "  \"entries\": [\n";

    for (size_t entryIndex = 0; entryIndex < snapshot.entries.size(); ++entryIndex)
    {
        const auto& entry = snapshot.entries[entryIndex];
        out << "    {\n";
        out << "      \"nodeType\": " << jsonQuoted (entry.nodeType) << ",\n";
        out << "      \"executionKind\": " << jsonQuoted (entry.executionKind) << ",\n";
        out << "      \"status\": " << jsonQuoted (entry.status) << ",\n";
        out << "      \"publicOutputs\": ";
        appendValueObject (out, entry.publicOutputs);
        out << ",\n";
        out << "      \"publicOutputSources\": ";
        appendValueSourceObject (out, entry.publicOutputs);
        out << ",\n";
        out << "      \"children\": [\n";

        for (size_t childIndex = 0; childIndex < entry.children.size(); ++childIndex)
        {
            const auto& child = entry.children[childIndex];
            out << "        {\n";
            out << "          \"cookIndex\": " << child.cookIndex << ",\n";
            out << "          \"childId\": " << jsonQuoted (child.childId) << ",\n";
            out << "          \"nodeType\": " << jsonQuoted (child.nodeType) << ",\n";
            out << "          \"role\": " << jsonQuoted (child.role) << ",\n";
            out << "          \"runtimeOp\": " << jsonQuoted (child.runtimeOp) << ",\n";
            out << "          \"status\": " << jsonQuoted (child.status) << ",\n";
            out << "          \"reason\": " << jsonQuoted (child.reason) << ",\n";
            out << "          \"inputs\": ";
            appendValueObject (out, child.inputs);
            out << ",\n";
            out << "          \"inputSources\": ";
            appendValueSourceObject (out, child.inputs);
            out << ",\n";
            out << "          \"outputs\": ";
            appendValueObject (out, child.outputs);
            out << "\n";
            out << "        }";

            if (childIndex + 1 < entry.children.size())
                out << ",";

            out << "\n";
        }

        out << "      ]\n";
        out << "    }";

        if (entryIndex + 1 < snapshot.entries.size())
            out << ",";

        out << "\n";
    }

    out << "  ]\n";
    out << "}\n";
    return out.str();
}

std::string makeLoudnessRuntimeBridgeJson (const LoudnessRuntimeBridgeSnapshot& snapshot)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision (6);
    out << "{\n";
    out << "  \"kind\": \"loudnessRuntimeBridge\",\n";
    out << "  \"sourceMode\": " << jsonQuoted (snapshot.sourceMode) << ",\n";
    out << "  \"usesLoadedRuntimeOutputs\": " << (snapshot.usesLoadedRuntimeOutputs ? "true" : "false") << ",\n";
    out << "  \"publicOutputs\": ";
    appendValueObject (out, snapshot.publicOutputs);
    out << ",\n";
    out << "  \"publicOutputSources\": ";
    appendValueSourceObject (out, snapshot.publicOutputs);
    out << ",\n";
    out << "  \"analyzer\": {\n";
    out << "    \"rms\": " << snapshot.analyzer.rms << ",\n";
    out << "    \"peak\": " << snapshot.analyzer.peak << ",\n";
    out << "    \"loudness\": " << snapshot.analyzer.loudness << ",\n";
    out << "    \"gate\": " << snapshot.analyzer.gate << ",\n";
    out << "    \"confidence\": " << snapshot.analyzer.confidence << ",\n";
    out << "    \"active\": " << (snapshot.analyzer.active ? "true" : "false") << ",\n";
    out << "    \"sampleCounter\": " << snapshot.analyzer.sampleCounter << "\n";
    out << "  }\n";
    out << "}\n";
    return out.str();
}
}
