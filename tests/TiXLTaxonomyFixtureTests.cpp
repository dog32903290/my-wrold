#include "StorageContractJson.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
namespace json = myworld::storage_contract_internal;

void expect (bool condition, const std::string& message)
{
    if (! condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        std::exit (1);
    }
}

std::string readTextFile (const std::filesystem::path& path)
{
    std::ifstream input (path);
    expect (input.good(), "could not read " + path.string());

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

json::JsonValue parseJsonFile (const std::filesystem::path& path)
{
    const auto text = readTextFile (path);
    json::JsonParser parser (text);
    auto value = parser.parse();
    expect (parser.ok(), "could not parse " + path.string() + ": " + parser.error());
    return value;
}

const json::JsonValue& requiredMember (const json::JsonValue& value,
                                       const std::string& name,
                                       const std::string& context)
{
    const auto* found = json::member (value, name);
    expect (found != nullptr, context + " missing member " + name);
    return *found;
}

const std::vector<json::JsonValue>& requiredArray (const json::JsonValue& value,
                                                  const std::string& context)
{
    expect (value.kind == json::JsonValue::Kind::array, context + " should be an array");
    return value.arrayValue;
}

std::vector<std::string> stringArrayMember (const json::JsonValue& value,
                                            const std::string& name,
                                            const std::string& context)
{
    const auto strings = json::stringArrayMember (value, name);
    expect (json::member (value, name) != nullptr, context + " missing string array " + name);
    return strings;
}

std::vector<std::string> objectStringMemberList (const std::vector<json::JsonValue>& objects,
                                                 const std::string& memberName,
                                                 const std::string& context)
{
    std::vector<std::string> values;

    for (const auto& object : objects)
    {
        expect (object.kind == json::JsonValue::Kind::object, context + " item should be an object");
        values.push_back (json::stringMember (object, memberName));
    }

    return values;
}

void expectEqual (const std::vector<std::string>& actual,
                  const std::vector<std::string>& expected,
                  const std::string& message)
{
    if (actual == expected)
        return;

    std::cerr << "FAIL: " << message << "\nexpected:";
    for (const auto& value : expected)
        std::cerr << ' ' << value;

    std::cerr << "\nactual:  ";
    for (const auto& value : actual)
        std::cerr << ' ' << value;

    std::cerr << '\n';
    std::exit (1);
}

const json::JsonValue& rootEntry (const std::vector<json::JsonValue>& roots, const std::string& path)
{
    const auto found = std::find_if (roots.begin(), roots.end(), [&path] (const auto& root)
    {
        return json::stringMember (root, "path") == path;
    });

    expect (found != roots.end(), "missing default browser root " + path);
    return *found;
}

void expectNoRoot (const std::vector<std::string>& roots, const std::string& root)
{
    expect (std::find (roots.begin(), roots.end(), root) == roots.end(),
            root + " must not be a default browser root");
}

void expectContains (const std::vector<std::string>& values,
                     const std::string& expected,
                     const std::string& message)
{
    expect (std::find (values.begin(), values.end(), expected) != values.end(), message);
}
}

int main()
{
    const auto fixture = parseJsonFile ("fixtures/tixl-witness/operator-browser-taxonomy.json");
    const auto catalog = parseJsonFile ("fixtures/tixl-witness/operator-catalog.json");

    expect (json::intMember (fixture, "schemaVersion", 0) == 1, "taxonomy fixture schema version");
    expect (json::stringMember (fixture, "kind") == "tixl-operator-browser-taxonomy",
            "taxonomy fixture kind");

    const auto& source = requiredMember (fixture, "source", "taxonomy fixture");
    const auto& catalogSource = requiredMember (catalog, "source", "operator catalog");
    expect (json::stringMember (source, "commit") == json::stringMember (catalogSource, "commit"),
            "taxonomy fixture keeps source commit aligned with operator catalog");
    expect (json::stringMember (source, "scope") == "Operators/Lib", "taxonomy fixture source scope");

    const auto& counts = requiredMember (fixture, "counts", "taxonomy fixture");
    const auto& catalogCounts = requiredMember (catalog, "counts", "operator catalog");
    expect (json::intMember (counts, "catalogEntries", 0) == json::intMember (catalogCounts, "entries", -1),
            "taxonomy fixture keeps operator catalog entry count");

    const auto& defaultBrowser = requiredMember (fixture, "defaultBrowser", "taxonomy fixture");
    expect (json::stringMember (defaultBrowser, "sourceRoot") == "Operators/Lib",
            "default browser source root");

    const auto& roots = requiredArray (requiredMember (defaultBrowser, "roots", "default browser"),
                                       "default browser roots");
    const auto rootPaths = objectStringMemberList (roots, "path", "default browser roots");

    expectEqual (rootPaths,
                 { "data",
                   "field",
                   "flow",
                   "image",
                   "io",
                   "mesh",
                   "numbers",
                   "particle",
                   "point",
                   "render",
                   "string" },
                 "default browser root order mirrors TiXL Operators/Lib");

    expectEqual (stringArrayMember (rootEntry (roots, "field"), "children", "field root"),
                 { "adjust", "analyze", "combine", "generate", "render", "space", "use" },
                 "field children mirror TiXL");
    expectEqual (stringArrayMember (rootEntry (roots, "image"), "children", "image root"),
                 { "analyze", "color", "fx", "generate", "transform", "use" },
                 "image children mirror TiXL");
    expectEqual (stringArrayMember (rootEntry (roots, "io"), "children", "io root"),
                 { "audio",
                   "dmx",
                   "file",
                   "freed",
                   "http",
                   "input",
                   "json",
                   "midi",
                   "osc",
                   "posistage",
                   "ptz",
                   "serial",
                   "tcp",
                   "udp",
                   "video",
                   "websocket" },
                 "io children mirror TiXL");
    expectEqual (stringArrayMember (rootEntry (roots, "numbers"), "children", "numbers root"),
                 { "anim",
                   "bool",
                   "color",
                   "curve",
                   "data",
                   "float",
                   "floats",
                   "int",
                   "int2",
                   "ints",
                   "vec2",
                   "vec3",
                   "vec4" },
                 "numbers children mirror TiXL");
    expect (json::boolMember (rootEntry (roots, "flow"), "rootOperators", false),
            "flow preserves root operators marker");

    const auto thirdLevelPaths = stringArrayMember (defaultBrowser,
                                                   "importantThirdLevelPaths",
                                                   "default browser");
    expectContains (thirdLevelPaths, "image/fx/blur", "third-level path preserves image/fx/blur");
    expectContains (thirdLevelPaths, "field/generate/sdf", "third-level path preserves field/generate/sdf");
    expectContains (thirdLevelPaths, "numbers/float/trigonometry", "third-level path preserves float trigonometry");
    expectContains (thirdLevelPaths, "render/camera/analyze", "third-level path preserves render camera analyze");
    expectContains (thirdLevelPaths, "string/buffers/transform", "third-level path preserves string buffers transform");

    const auto excluded = objectStringMemberList (
        requiredArray (requiredMember (defaultBrowser, "excludedFromDefault", "default browser"),
                       "default browser excluded paths"),
        "pattern",
        "default browser excluded paths");
    expectContains (excluded, "Operators/Lib/.meta", "hidden .meta path excluded");
    expectContains (excluded, "Operators/Lib/Assets", "resource Assets path excluded");
    expectContains (excluded, "Operators/Lib/Utils", "Utils path excluded");
    expectContains (excluded, "*/_", "internal underscore path excluded");
    expectContains (excluded, "*/_obsolete", "obsolete underscore path excluded");
    expectContains (excluded, "*/_internal", "internal path excluded");
    expectContains (excluded, "*/_experimental", "experimental path excluded");
    expectContains (excluded, "*/obsolete", "obsolete path excluded");
    expectContains (excluded, "*/legacy", "legacy path excluded");
    expectContains (excluded, "*/experimental", "experimental path excluded");
    expectContains (excluded, "operator-file-prefix:_", "underscore-prefixed operators excluded");
    expectContains (excluded, "flow/skillQuest", "flow skillQuest path excluded");

    const auto aliases = requiredArray (requiredMember (fixture, "aliasRules", "taxonomy fixture"),
                                        "alias rules");
    const auto aliasNames = objectStringMemberList (aliases, "alias", "alias rules");
    expectContains (aliasNames, "top", "top alias recorded");
    expectContains (aliasNames, "sop", "sop alias recorded");
    expectContains (aliasNames, "mat", "mat alias recorded");
    expectNoRoot (rootPaths, "top");
    expectNoRoot (rootPaths, "sop");
    expectNoRoot (rootPaths, "mat");

    const auto traces = stringArrayMember (fixture, "acceptanceTraces", "taxonomy fixture");
    expectContains (traces,
                    "category_browser_root_exact_tixl_paths",
                    "root path acceptance trace recorded");
    expectContains (traces,
                    "category_browser_drilldown_exact_namespace",
                    "drilldown acceptance trace recorded");
    expectContains (traces,
                    "hidden_paths_excluded_from_default_browser",
                    "hidden path acceptance trace recorded");

    return 0;
}
