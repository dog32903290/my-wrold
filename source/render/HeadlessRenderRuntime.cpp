#include "HeadlessRenderRuntime.h"

#include "JsonWriter.h"
#include "StorageContractJson.h"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

namespace myworld
{
namespace
{
namespace fs = std::filesystem;
namespace json = storage_contract_internal;

struct ConstantNode
{
    std::string id;
    std::vector<double> color { 0.02, 0.02, 0.02, 1.0 };
    int width = 1280;
    int height = 720;
};

struct OutputNode
{
    std::string id;
};

struct Edge
{
    std::string from;
    std::string to;
    std::string dataType;
};

struct ParsedFixture
{
    ConstantNode constant;
    OutputNode output;
    Edge edge;
};

std::string pathText (const fs::path& path)
{
    return path.generic_string();
}

HeadlessRenderRuntimeResult makeResultFor (const std::string& outputDirectory)
{
    const fs::path directory { outputDirectory };
    return {
        false,
        {},
        pathText (directory / "texture_summary.json"),
        pathText (directory / "cook_order.json"),
        pathText (directory / "node_stats.json"),
        pathText (directory / "errors.json")
    };
}

bool writeTextFile (const std::string& path, const std::string& text, std::string& error)
{
    const fs::path filePath { path };
    std::error_code createError;
    fs::create_directories (filePath.parent_path(), createError);

    if (createError)
    {
        error = "could not create output directory: " + createError.message();
        return false;
    }

    std::ofstream file { filePath };
    if (! file)
    {
        error = "could not write " + path;
        return false;
    }

    file << text;
    return true;
}

std::string readTextFile (const std::string& path, std::string& error)
{
    std::ifstream file { path };
    if (! file)
    {
        error = "could not read " + path;
        return {};
    }

    std::ostringstream text;
    text << file.rdbuf();
    return text.str();
}

std::string makeErrorsJson (bool ok, const std::vector<std::string>& errors)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"headlessRenderRuntimeErrors\",\n";
    out << "  \"ok\": " << (ok ? "true" : "false") << ",\n";
    out << "  \"errors\": [";

    for (size_t index = 0; index < errors.size(); ++index)
    {
        if (index != 0)
            out << ", ";

        out << jsonQuoted (errors[index]);
    }

    out << "]\n";
    out << "}\n";
    return out.str();
}

HeadlessRenderRuntimeResult failWithError (HeadlessRenderRuntimeResult result, const std::string& error)
{
    result.ok = false;
    result.error = error;

    std::string writeError;
    if (! writeTextFile (result.errorsPath, makeErrorsJson (false, { error }), writeError))
        result.error = error + "; " + writeError;

    return result;
}

std::vector<double> numberArrayMember (const json::JsonValue& value, const std::string& name)
{
    const auto* array = json::member (value, name);
    if (array == nullptr || array->kind != json::JsonValue::Kind::array)
        return {};

    std::vector<double> numbers;
    for (const auto& item : array->arrayValue)
    {
        if (item.kind != json::JsonValue::Kind::number)
            return {};

        numbers.push_back (item.numberValue);
    }

    return numbers;
}

const json::JsonValue* requiredObjectMember (const json::JsonValue& value,
                                             const std::string& name,
                                             std::string& error)
{
    const auto* found = json::member (value, name);
    if (found == nullptr || found->kind != json::JsonValue::Kind::object)
    {
        error = "missing object: " + name;
        return nullptr;
    }

    return found;
}

const json::JsonValue* requiredArrayMember (const json::JsonValue& value,
                                            const std::string& name,
                                            std::string& error)
{
    const auto* found = json::member (value, name);
    if (found == nullptr || found->kind != json::JsonValue::Kind::array)
    {
        error = "missing array: " + name;
        return nullptr;
    }

    return found;
}

bool parseFixture (const std::string& fixtureText, ParsedFixture& fixture, std::string& error)
{
    json::JsonParser parser { fixtureText };
    const auto root = parser.parse();
    if (! parser.ok())
    {
        error = "fixture json parse failed: " + parser.error();
        return false;
    }

    const auto* graph = requiredObjectMember (root, "graph", error);
    if (graph == nullptr)
        return false;

    const auto* nodes = requiredArrayMember (*graph, "nodes", error);
    if (nodes == nullptr)
        return false;

    bool hasConstant = false;
    bool hasOutput = false;

    for (const auto& node : nodes->arrayValue)
    {
        if (node.kind != json::JsonValue::Kind::object)
        {
            error = "node entry must be object";
            return false;
        }

        const auto nodeId = json::stringMember (node, "id");
        const auto nodeType = json::stringMember (node, "type");
        if (nodeId.empty() || nodeType.empty())
        {
            error = "node requires id and type";
            return false;
        }

        if (nodeType == "image.constant")
        {
            fixture.constant.id = nodeId;

            if (const auto* params = json::member (node, "params");
                params != nullptr && params->kind == json::JsonValue::Kind::object)
            {
                auto color = numberArrayMember (*params, "color");
                if (! color.empty())
                    fixture.constant.color = color;

                auto resolution = numberArrayMember (*params, "resolution");
                if (! resolution.empty())
                {
                    if (resolution.size() != 2)
                    {
                        error = "invalid resolution";
                        return false;
                    }

                    fixture.constant.width = static_cast<int> (resolution[0]);
                    fixture.constant.height = static_cast<int> (resolution[1]);
                }
            }

            hasConstant = true;
            continue;
        }

        if (nodeType == "output.texture_summary")
        {
            fixture.output.id = nodeId;
            hasOutput = true;
            continue;
        }

        error = "unsupported node type: " + nodeType;
        return false;
    }

    if (! hasConstant)
    {
        error = "missing image.constant node";
        return false;
    }

    if (! hasOutput)
    {
        error = "missing output.texture_summary node";
        return false;
    }

    if (fixture.constant.color.size() != 4)
    {
        error = "invalid color";
        return false;
    }

    if (fixture.constant.width <= 0 || fixture.constant.height <= 0)
    {
        error = "invalid resolution";
        return false;
    }

    const auto* edges = requiredArrayMember (*graph, "edges", error);
    if (edges == nullptr)
        return false;

    for (const auto& edge : edges->arrayValue)
    {
        if (edge.kind != json::JsonValue::Kind::object)
        {
            error = "edge entry must be object";
            return false;
        }

        const auto from = json::stringMember (edge, "from");
        const auto to = json::stringMember (edge, "to");
        const auto dataType = json::stringMember (edge, "dataType");

        if (from == fixture.constant.id + ".out" && to == fixture.output.id + ".input")
        {
            fixture.edge = { from, to, dataType };
            break;
        }
    }

    if (fixture.edge.from.empty())
    {
        error = "missing texture edge";
        return false;
    }

    if (fixture.edge.dataType != "texture.rgba")
    {
        error = "unsupported edge data type: " + fixture.edge.dataType;
        return false;
    }

    return true;
}

std::string makeTextureSummaryJson (const ParsedFixture& fixture)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision (6);
    out << "{\n";
    out << "  \"kind\": \"textureSummary\",\n";
    out << "  \"ok\": true,\n";
    out << "  \"sourceNodeId\": " << jsonQuoted (fixture.constant.id) << ",\n";
    out << "  \"outputNodeId\": " << jsonQuoted (fixture.output.id) << ",\n";
    out << "  \"width\": " << fixture.constant.width << ",\n";
    out << "  \"height\": " << fixture.constant.height << ",\n";
    out << "  \"format\": \"rgba8\",\n";
    out << "  \"color\": [" << fixture.constant.color[0] << ", "
        << fixture.constant.color[1] << ", "
        << fixture.constant.color[2] << ", "
        << fixture.constant.color[3] << "],\n";
    out << "  \"status\": \"ok\"\n";
    out << "}\n";
    return out.str();
}

std::string makeCookOrderJson (const ParsedFixture& fixture)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"headlessRenderCookOrder\",\n";
    out << "  \"version\": 1,\n";
    out << "  \"cookOrder\": ["
        << jsonQuoted (fixture.constant.id) << ", "
        << jsonQuoted (fixture.output.id) << "],\n";
    out << "  \"edges\": [\n";
    out << "    { \"from\": " << jsonQuoted (fixture.edge.from)
        << ", \"to\": " << jsonQuoted (fixture.edge.to)
        << ", \"dataType\": " << jsonQuoted (fixture.edge.dataType) << " }\n";
    out << "  ]\n";
    out << "}\n";
    return out.str();
}

std::string makeNodeStatsJson (const ParsedFixture& fixture)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"headlessRenderNodeStats\",\n";
    out << "  \"version\": 1,\n";
    out << "  \"renderer\": \"headless\",\n";
    out << "  \"nodes\": [\n";
    out << "    {\n";
    out << "      \"id\": " << jsonQuoted (fixture.constant.id) << ",\n";
    out << "      \"type\": \"image.constant\",\n";
    out << "      \"cookDomain\": \"render\",\n";
    out << "      \"status\": \"ok\",\n";
    out << "      \"outputSummary\": {\n";
    out << "        \"format\": \"rgba8\",\n";
    out << "        \"width\": " << fixture.constant.width << ",\n";
    out << "        \"height\": " << fixture.constant.height << "\n";
    out << "      }\n";
    out << "    },\n";
    out << "    {\n";
    out << "      \"id\": " << jsonQuoted (fixture.output.id) << ",\n";
    out << "      \"type\": \"output.texture_summary\",\n";
    out << "      \"cookDomain\": \"render\",\n";
    out << "      \"status\": \"ok\"\n";
    out << "    }\n";
    out << "  ]\n";
    out << "}\n";
    return out.str();
}
}

HeadlessRenderRuntimeResult runHeadlessRenderRuntimeProof (const std::string& fixturePath,
                                                           const std::string& outputDirectory)
{
    auto result = makeResultFor (outputDirectory);
    std::string error;
    const auto fixtureText = readTextFile (fixturePath, error);
    if (! error.empty())
        return failWithError (std::move (result), error);

    return runHeadlessRenderRuntimeProofText (fixtureText, outputDirectory);
}

HeadlessRenderRuntimeResult runHeadlessRenderRuntimeProofText (const std::string& fixtureText,
                                                               const std::string& outputDirectory)
{
    auto result = makeResultFor (outputDirectory);

    ParsedFixture fixture;
    std::string error;
    if (! parseFixture (fixtureText, fixture, error))
        return failWithError (std::move (result), error);

    if (! writeTextFile (result.textureSummaryPath, makeTextureSummaryJson (fixture), error)
        || ! writeTextFile (result.cookOrderPath, makeCookOrderJson (fixture), error)
        || ! writeTextFile (result.nodeStatsPath, makeNodeStatsJson (fixture), error)
        || ! writeTextFile (result.errorsPath, makeErrorsJson (true, {}), error))
    {
        return failWithError (std::move (result), error);
    }

    result.ok = true;
    return result;
}
}
