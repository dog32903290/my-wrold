#include "StorageContract.h"

#include "JsonWriter.h"
#include "StorageContractJson.h"

#include <sstream>

namespace myworld
{
using namespace storage_contract_internal;

WorkProjectManifest makeMinimalWorkProject (const std::string& id, const std::string& title)
{
    return {
        id,
        title,
        "patches/main.patch.json",
        "commandS.localCommit",
        { "modules" },
        { "works" }
    };
}

std::string toJson (const WorkProjectManifest& manifest)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"workProject\",\n";
    out << "  \"id\": " << jsonQuoted (manifest.id) << ",\n";
    out << "  \"title\": " << jsonQuoted (manifest.title) << ",\n";
    out << "  \"mainPatchPath\": " << jsonQuoted (manifest.mainPatchPath) << ",\n";
    out << "  \"savePolicy\": " << jsonQuoted (manifest.savePolicy) << ",\n";
    out << "  \"moduleLibraries\": ";
    appendJsonStringArray (out, manifest.moduleLibraries);
    out << ",\n";
    out << "  \"workLibraries\": ";
    appendJsonStringArray (out, manifest.workLibraries);
    out << "\n";
    out << "}\n";
    return out.str();
}

WorkProjectLoadResult parseWorkProjectManifest (const std::string& text)
{
    JsonParser parser (text);
    const auto root = parser.parse();

    if (! parser.ok())
        return { false, {}, parser.error() };

    if (root.kind != JsonValue::Kind::object)
        return { false, {}, "work project root must be an object" };

    if (stringMember (root, "kind") != "workProject")
        return { false, {}, "work project kind must be workProject" };

    WorkProjectManifest manifest;
    manifest.id = stringMember (root, "id");
    manifest.title = stringMember (root, "title");
    manifest.mainPatchPath = stringMember (root, "mainPatchPath");
    manifest.savePolicy = stringMember (root, "savePolicy");
    manifest.moduleLibraries = stringArrayMember (root, "moduleLibraries");
    manifest.workLibraries = stringArrayMember (root, "workLibraries");

    if (manifest.id.empty() || manifest.title.empty() || manifest.mainPatchPath.empty())
        return { false, {}, "work project is missing required identity or main patch fields" };

    return { true, manifest, {} };
}

WorkProjectLoadResult loadWorkProjectManifest (const std::string& path)
{
    std::string error;
    const auto text = readTextFile (path, error);
    if (! error.empty())
        return { false, {}, error };

    return parseWorkProjectManifest (text);
}
}
