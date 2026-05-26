#include "WorkbenchRunSurface.h"

#include "JsonWriter.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>

namespace myworld
{
namespace
{
struct HeadlessSupport
{
    bool ready = false;
    std::string reason = "unsupported";
    const WorkbenchSessionSnapshot::GraphNodeSummary* constant = nullptr;
    const WorkbenchSessionSnapshot::GraphNodeSummary* output = nullptr;
    const WorkbenchSessionSnapshot::GraphEdgeSummary* edge = nullptr;
    std::vector<std::string> order;
};

std::string fallback (const std::string& value, const std::string& fallbackValue)
{
    return value.empty() ? fallbackValue : value;
}

std::string countValue (int nodes, int edges)
{
    return std::to_string (nodes)
           + " nodes/"
           + std::to_string (edges)
           + " edges";
}

std::string endpointNodeId (const std::string& endpoint)
{
    const auto dot = endpoint.find ('.');
    return dot == std::string::npos ? endpoint : endpoint.substr (0, dot);
}

void appendUnique (std::vector<std::string>& values, const std::string& value)
{
    if (value.empty())
        return;

    if (std::find (values.begin(), values.end(), value) == values.end())
        values.push_back (value);
}

std::string joinOrder (const std::vector<std::string>& order)
{
    if (order.empty())
        return "none";

    std::string joined;
    for (const auto& id : order)
    {
        if (! joined.empty())
            joined += " -> ";

        joined += id;
    }

    return joined;
}

const WorkbenchSessionSnapshot::GraphNodeSummary* findNode (
    const std::vector<WorkbenchSessionSnapshot::GraphNodeSummary>& nodes,
    const std::string& id)
{
    const auto found = std::find_if (nodes.begin(), nodes.end(), [&id] (const auto& node)
    {
        return node.id == id;
    });

    return found == nodes.end() ? nullptr : &*found;
}

const std::string* paramValue (const WorkbenchSessionSnapshot::GraphNodeSummary& node,
                              const std::string& id)
{
    const auto found = std::find_if (node.params.begin(), node.params.end(), [&id] (const auto& param)
    {
        return param.id == id;
    });

    return found == node.params.end() ? nullptr : &found->value;
}

std::vector<double> parseNumberList (std::string text)
{
    for (auto& character : text)
    {
        if (character == '[' || character == ']')
            character = ' ';
    }

    std::vector<double> values;
    std::stringstream stream { text };
    std::string item;
    while (std::getline (stream, item, ','))
    {
        std::stringstream itemStream { item };
        double value = 0.0;
        if (! (itemStream >> value))
            return {};

        values.push_back (value);
    }

    return values;
}

std::vector<double> numberListParam (const WorkbenchSessionSnapshot::GraphNodeSummary& node,
                                     const std::string& id,
                                     std::vector<double> fallbackValue)
{
    const auto* value = paramValue (node, id);
    if (value == nullptr)
        return fallbackValue;

    const auto parsed = parseNumberList (*value);
    return parsed.empty() ? fallbackValue : parsed;
}

HeadlessSupport headlessSupportFor (const WorkbenchSessionSnapshot& snapshot)
{
    HeadlessSupport support;

    if (! snapshot.ok)
    {
        support.reason = "blocked";
        return support;
    }

    if (snapshot.runtimeNodes.empty())
    {
        support.reason = "no runtime graph";
        return support;
    }

    for (const auto& node : snapshot.runtimeNodes)
    {
        if (node.type == "image.constant")
        {
            support.constant = &node;
            continue;
        }

        if (node.type == "output.texture_summary")
        {
            support.output = &node;
            continue;
        }

        support.reason = "unsupported node " + node.type;
        return support;
    }

    if (support.constant == nullptr)
    {
        support.reason = "missing image.constant";
        return support;
    }

    if (support.output == nullptr)
    {
        support.reason = "missing output.texture_summary";
        return support;
    }

    const auto expectedFrom = support.constant->id + ".out";
    const auto expectedTo = support.output->id + ".input";
    for (const auto& edge : snapshot.runtimeEdges)
    {
        if (edge.from == expectedFrom && edge.to == expectedTo)
        {
            support.edge = &edge;
            break;
        }
    }

    if (support.edge == nullptr)
    {
        support.reason = "missing texture edge";
        return support;
    }

    if (support.edge->dataType != "texture.rgba")
    {
        support.reason = "unsupported edge " + support.edge->dataType;
        return support;
    }

    for (const auto& edge : snapshot.runtimeEdges)
    {
        appendUnique (support.order, endpointNodeId (edge.from));
        appendUnique (support.order, endpointNodeId (edge.to));
    }

    for (const auto& node : snapshot.runtimeNodes)
        appendUnique (support.order, node.id);

    support.ready = true;
    support.reason = "ready";
    return support;
}

std::string readinessFor (const WorkbenchSessionSnapshot& snapshot, const HeadlessSupport& support)
{
    if (! snapshot.ok)
        return "blocked";

    return support.ready ? "ready" : "unsupported";
}

std::string readinessTone (const std::string& readiness)
{
    if (readiness == "blocked" || readiness == "unsupported")
        return readiness == "blocked" ? "blocked" : "idle";

    return "ready";
}

WorkbenchRunSurfaceRow makeRow (std::string id,
                                std::string label,
                                std::string value,
                                std::string tone)
{
    WorkbenchRunSurfaceRow row;
    row.id = std::move (id);
    row.label = std::move (label);
    row.value = std::move (value);
    row.text = row.label + " " + row.value;
    row.tone = std::move (tone);
    return row;
}

WorkbenchRunSurface makeWorkbenchRunSurfaceWithExecution (const WorkbenchSessionSnapshot& snapshot,
                                                          const std::string& execution)
{
    const auto support = headlessSupportFor (snapshot);
    const auto readiness = readinessFor (snapshot, support);
    const auto documentId = fallback (snapshot.documentId,
                                      snapshot.ok ? "untitled run" : "no workbench session");
    const auto target = fallback (snapshot.activeOutputNodeId, "none");

    WorkbenchRunSurface surface;
    surface.ok = snapshot.ok;
    surface.runOrder = support.ready ? support.order : std::vector<std::string> {};

    if (! snapshot.ok)
    {
        surface.headline = "run blocked: " + fallback (snapshot.message, "no current workbench session");
    }
    else if (support.ready)
    {
        surface.headline = "run ready " + documentId + " target " + target;
    }
    else
    {
        surface.headline = "run unsupported " + documentId + " headless " + support.reason;
    }

    surface.rows = {
        makeRow ("run",
                 "run",
                 documentId,
                 snapshot.ok ? readinessTone (readiness) : "blocked"),
        makeRow ("target",
                 "target",
                 target,
                 target == "none" ? "idle" : "ready"),
        makeRow ("graph",
                 "graph",
                 countValue (snapshot.runtimeNodeCount, snapshot.runtimeEdgeCount),
                 snapshot.runtimeNodeCount > 0 ? "ready" : "idle"),
        makeRow ("adapter",
                 "adapter",
                 "headless-render",
                 support.ready ? "ready" : "idle"),
        makeRow ("readiness",
                 "readiness",
                 readiness,
                 readinessTone (readiness)),
        makeRow ("execution",
                 "execution",
                 execution,
                 execution == "ran" ? "ready" : "idle")
    };

    return surface;
}

std::string formatNumberList (const std::vector<double>& values, int precision)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision (precision);
    out << "[";
    for (std::size_t index = 0; index < values.size(); ++index)
    {
        if (index != 0)
            out << ", ";

        out << values[index];
    }
    out << "]";
    return out.str();
}

std::string formatResolution (const std::vector<double>& values)
{
    const auto width = values.empty() ? 1280 : static_cast<int> (values[0]);
    const auto height = values.size() < 2 ? 720 : static_cast<int> (values[1]);
    return "[" + std::to_string (width) + ", " + std::to_string (height) + "]";
}
}

WorkbenchRunSurface makeWorkbenchRunSurface (const WorkbenchSessionSnapshot& snapshot)
{
    const auto support = headlessSupportFor (snapshot);
    const auto execution = support.ready ? "not-run" : "parked";
    return makeWorkbenchRunSurfaceWithExecution (snapshot, execution);
}

std::string makeWorkbenchHeadlessRenderFixtureText (const WorkbenchSessionSnapshot& snapshot)
{
    const auto support = headlessSupportFor (snapshot);
    if (! support.ready)
        return {};

    const auto color = numberListParam (*support.constant, "color", { 0.02, 0.02, 0.02, 1.0 });
    const auto resolution = numberListParam (*support.constant, "resolution", { 1280.0, 720.0 });

    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"runtimeProofFixture\",\n";
    out << "  \"version\": 1,\n";
    out << "  \"name\": \"workbench_headless_run\",\n";
    out << "  \"graph\": {\n";
    out << "    \"nodes\": [\n";
    out << "      {\n";
    out << "        \"id\": " << jsonQuoted (support.constant->id) << ",\n";
    out << "        \"type\": \"image.constant\",\n";
    out << "        \"params\": {\n";
    out << "          \"color\": " << formatNumberList (color, 6) << ",\n";
    out << "          \"resolution\": " << formatResolution (resolution) << "\n";
    out << "        }\n";
    out << "      },\n";
    out << "      {\n";
    out << "        \"id\": " << jsonQuoted (support.output->id) << ",\n";
    out << "        \"type\": \"output.texture_summary\",\n";
    out << "        \"params\": {\n";
    out << "          \"path\": \"texture_summary.json\"\n";
    out << "        }\n";
    out << "      }\n";
    out << "    ],\n";
    out << "    \"edges\": [\n";
    out << "      {\n";
    out << "        \"id\": " << jsonQuoted (support.edge->id) << ",\n";
    out << "        \"from\": " << jsonQuoted (support.edge->from) << ",\n";
    out << "        \"to\": " << jsonQuoted (support.edge->to) << ",\n";
    out << "        \"dataType\": " << jsonQuoted (support.edge->dataType) << ",\n";
    out << "        \"streamKind\": " << jsonQuoted (fallback (support.edge->streamKind, "continuous")) << "\n";
    out << "      }\n";
    out << "    ]\n";
    out << "  }\n";
    out << "}\n";
    return out.str();
}

WorkbenchHeadlessRunResult runWorkbenchHeadlessRender (const WorkbenchSessionSnapshot& snapshot,
                                                       const std::filesystem::path& outputDirectory)
{
    WorkbenchHeadlessRunResult result;
    result.surface = makeWorkbenchRunSurface (snapshot);

    const auto support = headlessSupportFor (snapshot);
    if (! snapshot.ok)
    {
        result.status = "failed";
        result.error = fallback (snapshot.message, "no current workbench session");
        result.statusText = "workbench run failed: " + result.error;
        return result;
    }

    if (! support.ready)
    {
        result.status = "failed";
        result.error = support.reason;
        result.statusText = "workbench run failed: headless " + result.error;
        return result;
    }

    result.fixtureText = makeWorkbenchHeadlessRenderFixtureText (snapshot);
    result.headless = runHeadlessRenderRuntimeProofText (result.fixtureText, outputDirectory.string());
    result.artifactPaths = {
        result.headless.textureSummaryPath,
        result.headless.cookOrderPath,
        result.headless.nodeStatsPath,
        result.headless.thumbnailPath,
        result.headless.thumbnailStatsPath,
        result.headless.errorsPath
    };

    result.ok = result.headless.ok;
    result.status = result.ok ? "ran" : "failed";
    result.error = result.ok ? std::string {} : result.headless.error;
    result.statusText = result.ok
                            ? "workbench run ready: "
                                  + fallback (snapshot.documentId, "untitled run")
                                  + " headless "
                                  + joinOrder (support.order)
                            : "workbench run failed: " + result.error;
    result.surface = makeWorkbenchRunSurfaceWithExecution (snapshot, result.ok ? "ran" : "failed");
    return result;
}
}
