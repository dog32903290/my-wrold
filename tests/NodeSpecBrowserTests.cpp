#include "CompoundModule.h"
#include "NodeSpecBrowser.h"
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

void expectEqual (const std::string& actual, const std::string& expected, const std::string& message)
{
    expect (actual == expected, message + " expected " + expected + " got " + actual);
}

void expectContains (const std::vector<std::string>& values,
                     const std::string& expected,
                     const std::string& message)
{
    expect (std::find (values.begin(), values.end(), expected) != values.end(), message);
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

std::vector<std::string> savedTypes (const std::vector<myworld::BrowserNodeEntry>& entries)
{
    std::vector<std::string> result;
    for (const auto& entry : entries)
        result.push_back (entry.savedType);

    return result;
}

const myworld::BrowserNodeEntry& requiredEntry (const std::vector<myworld::BrowserNodeEntry>& entries,
                                               const std::string& savedType)
{
    const auto found = std::find_if (entries.begin(), entries.end(), [&savedType] (const auto& entry)
    {
        return entry.savedType == savedType;
    });

    expect (found != entries.end(), "missing browser entry " + savedType);
    return *found;
}

myworld::BrowserNodeEntry firstSearchResult (const std::vector<myworld::BrowserNodeEntry>& entries,
                                             const std::string& query)
{
    const auto results = myworld::searchBrowserNodeEntries (entries, query);
    expect (! results.empty(), "search returned no results for " + query);
    return results.front();
}

bool traceNamed (const std::vector<json::JsonValue>& traces, const std::string& name)
{
    return std::find_if (traces.begin(), traces.end(), [&name] (const auto& trace)
    {
        return json::stringMember (trace, "name") == name;
    }) != traces.end();
}
}

int main()
{
    const auto loadedSpecs = myworld::loadCompoundModuleNodeSpecsFromLibrary (
        "fixtures/module-libraries/analyzer-family.module-library.json");
    expect (loadedSpecs.ok, loadedSpecs.error.empty() ? "module specs load" : loadedSpecs.error);
    const auto specs = myworld::mergeNodeSpecs (myworld::makeSeedNodeSpecs(), loadedSpecs.specs);
    const auto entries = myworld::makeBrowserNodeEntries (specs);

    const auto& texture = requiredEntry (entries, "image.texture");
    expectEqual (texture.displayName, "Texture", "display name copied from NodeSpec");
    expectEqual (texture.taxonomyPath.size() == 2 ? texture.taxonomyPath.front() : "", "image", "taxonomy root");
    expectEqual (texture.taxonomyPath.size() == 2 ? texture.taxonomyPath.back() : "", "use", "taxonomy child");
    expectContains (texture.aliases, "top.texture", "image texture has top alias");
    expect (texture.visibleByDefault, "seed texture is visible by default");
    expect (! texture.hidden, "seed texture is not hidden");
    expect (texture.runtimeReady, "seed texture has runtime readiness metadata");
    expect (texture.outputs.size() == 1 && texture.outputs.front().dataType == "texture.rgba",
            "entry keeps output port metadata");

    expectEqual (firstSearchResult (entries, "loudness").savedType,
                 "analyzer.loudness",
                 "exact display-name search ranks first");
    expectEqual (firstSearchResult (entries, "tex").savedType,
                 "image.texture",
                 "starts-with search ranks first");
    expectEqual (firstSearchResult (entries, "xture").savedType,
                 "image.texture",
                 "contains search ranks first");
    expectEqual (firstSearchResult (entries, "mco").savedType,
                 "io.midi.cc_out",
                 "PascalCase/token acronym search works");
    expectEqual (firstSearchResult (entries, "io midi").savedType,
                 "io.midi.cc_out",
                 "namespace search works");

    const auto descriptionResults = savedTypes (myworld::searchBrowserNodeEntries (entries, "audio mono"));
    expectContains (descriptionResults, "analyzer.loudness", "description search includes port/data-type words");

    const auto topResults = myworld::searchBrowserNodeEntries (entries, "top.texture");
    expect (! topResults.empty(), "top alias search returns results");
    expectEqual (topResults.front().savedType,
                 "image.texture",
                 "alias search preserves native saved type");

    const auto fromOutput = savedTypes (myworld::compatibleBrowserNodeEntries (
        entries,
        { myworld::BrowserPortContextDirection::draggedOutput, "audio.channels" }));
    expectContains (fromOutput, "audio.mono_mix", "output drag suggests seed node with matching input");
    expectContains (fromOutput, "compound.raw-energy", "output drag includes loaded module candidate");

    const auto fromInput = savedTypes (myworld::compatibleBrowserNodeEntries (
        entries,
        { myworld::BrowserPortContextDirection::draggedInput, "signal.float" }));
    expectContains (fromInput, "analyzer.loudness", "input drag suggests seed node with matching output");
    expectContains (fromInput, "compound.raw-energy", "input drag includes loaded module output candidate");

    const auto fixture = parseJsonFile ("fixtures/interaction/tixl-search-compatible-create.behavior.json");
    expect (json::intMember (fixture, "schemaVersion", 0) == 1, "browser behavior fixture schema version");
    expectEqual (json::stringMember (fixture, "kind"),
                 "tixl-search-compatible-create-behavior",
                 "browser behavior fixture kind");
    const auto traces = requiredArray (requiredMember (fixture, "traces", "browser behavior fixture"),
                                       "browser behavior traces");
    expect (traceNamed (traces, "compatible_create_from_output_drag"), "output-drag behavior row");
    expect (traceNamed (traces, "compatible_create_from_input_drag"), "input-drag behavior row");
    expect (traceNamed (traces, "browser_keyboard_return_escape_no_mutation_on_cancel"),
            "cancel-without-mutation behavior row");

    std::cout << "node spec browser tests ok\n";
    return 0;
}
