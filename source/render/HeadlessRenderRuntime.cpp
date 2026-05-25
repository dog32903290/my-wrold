#include "HeadlessRenderRuntime.h"

#include "JsonWriter.h"
#include "StorageContractJson.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
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
        pathText (directory / "thumbnail.png"),
        pathText (directory / "thumbnail_stats.json"),
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

bool writeBinaryFile (const std::string& path, const std::vector<unsigned char>& bytes, std::string& error)
{
    const fs::path filePath { path };
    std::error_code createError;
    fs::create_directories (filePath.parent_path(), createError);

    if (createError)
    {
        error = "could not create output directory: " + createError.message();
        return false;
    }

    std::ofstream file { filePath, std::ios::binary };
    if (! file)
    {
        error = "could not write " + path;
        return false;
    }

    file.write (reinterpret_cast<const char*> (bytes.data()),
                static_cast<std::streamsize> (bytes.size()));
    if (! file)
    {
        error = "could not write " + path;
        return false;
    }

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

void appendU16LE (std::vector<unsigned char>& bytes, std::uint16_t value)
{
    bytes.push_back (static_cast<unsigned char> (value & 0xffu));
    bytes.push_back (static_cast<unsigned char> ((value >> 8u) & 0xffu));
}

void appendU32BE (std::vector<unsigned char>& bytes, std::uint32_t value)
{
    bytes.push_back (static_cast<unsigned char> ((value >> 24u) & 0xffu));
    bytes.push_back (static_cast<unsigned char> ((value >> 16u) & 0xffu));
    bytes.push_back (static_cast<unsigned char> ((value >> 8u) & 0xffu));
    bytes.push_back (static_cast<unsigned char> (value & 0xffu));
}

std::uint32_t updateCrc32 (std::uint32_t crc, const unsigned char* bytes, std::size_t size)
{
    for (std::size_t index = 0; index < size; ++index)
    {
        crc ^= bytes[index];

        for (int bit = 0; bit < 8; ++bit)
            crc = (crc & 1u) != 0u ? (crc >> 1u) ^ 0xedb88320u : crc >> 1u;
    }

    return crc;
}

std::uint32_t adler32 (const std::vector<unsigned char>& bytes)
{
    constexpr auto modulo = 65521u;
    std::uint32_t a = 1u;
    std::uint32_t b = 0u;

    for (const auto byte : bytes)
    {
        a = (a + byte) % modulo;
        b = (b + a) % modulo;
    }

    return (b << 16u) | a;
}

void appendPngChunk (std::vector<unsigned char>& png,
                     const char type[4],
                     const std::vector<unsigned char>& data)
{
    appendU32BE (png, static_cast<std::uint32_t> (data.size()));

    const auto typeOffset = png.size();
    png.push_back (static_cast<unsigned char> (type[0]));
    png.push_back (static_cast<unsigned char> (type[1]));
    png.push_back (static_cast<unsigned char> (type[2]));
    png.push_back (static_cast<unsigned char> (type[3]));
    png.insert (png.end(), data.begin(), data.end());

    auto crc = 0xffffffffu;
    crc = updateCrc32 (crc, png.data() + typeOffset, png.size() - typeOffset);
    appendU32BE (png, crc ^ 0xffffffffu);
}

unsigned char colorByte (double value)
{
    return static_cast<unsigned char> (std::round (std::clamp (value, 0.0, 1.0) * 255.0));
}

std::vector<unsigned char> makeConstantThumbnailPng (const ParsedFixture& fixture,
                                                     int thumbnailWidth,
                                                     int thumbnailHeight)
{
    const auto red = colorByte (fixture.constant.color[0]);
    const auto green = colorByte (fixture.constant.color[1]);
    const auto blue = colorByte (fixture.constant.color[2]);
    const auto alpha = colorByte (fixture.constant.color[3]);

    std::vector<unsigned char> raw;
    raw.reserve (static_cast<std::size_t> (thumbnailHeight) * (static_cast<std::size_t> (thumbnailWidth) * 4u + 1u));

    for (int y = 0; y < thumbnailHeight; ++y)
    {
        raw.push_back (0u);

        for (int x = 0; x < thumbnailWidth; ++x)
        {
            raw.push_back (red);
            raw.push_back (green);
            raw.push_back (blue);
            raw.push_back (alpha);
        }
    }

    std::vector<unsigned char> zlib;
    zlib.push_back (0x78u);
    zlib.push_back (0x01u);

    std::size_t offset = 0;
    while (offset < raw.size())
    {
        const auto blockSize = std::min<std::size_t> (65535u, raw.size() - offset);
        const auto finalBlock = offset + blockSize == raw.size();
        zlib.push_back (finalBlock ? 0x01u : 0x00u);
        appendU16LE (zlib, static_cast<std::uint16_t> (blockSize));
        appendU16LE (zlib, static_cast<std::uint16_t> (~blockSize));
        zlib.insert (zlib.end(), raw.begin() + static_cast<std::ptrdiff_t> (offset),
                     raw.begin() + static_cast<std::ptrdiff_t> (offset + blockSize));
        offset += blockSize;
    }

    appendU32BE (zlib, adler32 (raw));

    std::vector<unsigned char> png {
        0x89u, 0x50u, 0x4eu, 0x47u, 0x0du, 0x0au, 0x1au, 0x0au
    };

    std::vector<unsigned char> ihdr;
    appendU32BE (ihdr, static_cast<std::uint32_t> (thumbnailWidth));
    appendU32BE (ihdr, static_cast<std::uint32_t> (thumbnailHeight));
    ihdr.push_back (8u);
    ihdr.push_back (6u);
    ihdr.push_back (0u);
    ihdr.push_back (0u);
    ihdr.push_back (0u);

    appendPngChunk (png, "IHDR", ihdr);
    appendPngChunk (png, "IDAT", zlib);
    appendPngChunk (png, "IEND", {});
    return png;
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

std::string makeThumbnailStatsJson (const ParsedFixture& fixture,
                                    const std::string& thumbnailPath,
                                    int thumbnailWidth,
                                    int thumbnailHeight)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision (6);
    out << "{\n";
    out << "  \"kind\": \"renderThumbnailStats\",\n";
    out << "  \"ok\": true,\n";
    out << "  \"renderer\": \"headless\",\n";
    out << "  \"thumbnailPath\": " << jsonQuoted (thumbnailPath) << ",\n";
    out << "  \"sourceNodeId\": " << jsonQuoted (fixture.constant.id) << ",\n";
    out << "  \"outputNodeId\": " << jsonQuoted (fixture.output.id) << ",\n";
    out << "  \"sourceWidth\": " << fixture.constant.width << ",\n";
    out << "  \"sourceHeight\": " << fixture.constant.height << ",\n";
    out << "  \"thumbnailWidth\": " << thumbnailWidth << ",\n";
    out << "  \"thumbnailHeight\": " << thumbnailHeight << ",\n";
    out << "  \"format\": \"png.rgba8\",\n";
    out << "  \"color\": [" << fixture.constant.color[0] << ", "
        << fixture.constant.color[1] << ", "
        << fixture.constant.color[2] << ", "
        << fixture.constant.color[3] << "]\n";
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

    constexpr int thumbnailWidth = 96;
    constexpr int thumbnailHeight = 54;
    const auto thumbnailPng = makeConstantThumbnailPng (fixture, thumbnailWidth, thumbnailHeight);

    if (! writeTextFile (result.textureSummaryPath, makeTextureSummaryJson (fixture), error)
        || ! writeTextFile (result.cookOrderPath, makeCookOrderJson (fixture), error)
        || ! writeTextFile (result.nodeStatsPath, makeNodeStatsJson (fixture), error)
        || ! writeBinaryFile (result.thumbnailPath, thumbnailPng, error)
        || ! writeTextFile (result.thumbnailStatsPath,
                            makeThumbnailStatsJson (fixture, result.thumbnailPath, thumbnailWidth, thumbnailHeight),
                            error)
        || ! writeTextFile (result.errorsPath, makeErrorsJson (true, {}), error))
    {
        return failWithError (std::move (result), error);
    }

    result.ok = true;
    return result;
}
}
