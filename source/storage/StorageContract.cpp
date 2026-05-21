#include "StorageContract.h"

#include <algorithm>
#include <array>
#include <sstream>

namespace myworld
{
namespace
{
std::string quote (const std::string& text)
{
    std::ostringstream out;
    out << "\"";

    for (const auto character : text)
    {
        if (character == '"' || character == '\\')
            out << '\\';

        out << character;
    }

    out << "\"";
    return out.str();
}

void appendStringArray (std::ostringstream& out, const std::vector<std::string>& values)
{
    out << "[";

    for (size_t index = 0; index < values.size(); ++index)
    {
        if (index != 0)
            out << ", ";

        out << quote (values[index]);
    }

    out << "]";
}
}

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

PatchDocumentManifest makeMinimalPatchDocument (const std::string& id, const std::string& title)
{
    return { id, title, "editorGraph", "runtimeGraph", "portBindings" };
}

ModulePackageManifest makeModulePackage (const std::string& id, const std::string& title, const std::string& patchPath)
{
    return {
        id,
        title,
        patchPath,
        "docs/manual.md",
        { "in", "out" }
    };
}

std::string toJson (const WorkProjectManifest& manifest)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"workProject\",\n";
    out << "  \"id\": " << quote (manifest.id) << ",\n";
    out << "  \"title\": " << quote (manifest.title) << ",\n";
    out << "  \"mainPatchPath\": " << quote (manifest.mainPatchPath) << ",\n";
    out << "  \"savePolicy\": " << quote (manifest.savePolicy) << ",\n";
    out << "  \"moduleLibraries\": ";
    appendStringArray (out, manifest.moduleLibraries);
    out << ",\n";
    out << "  \"workLibraries\": ";
    appendStringArray (out, manifest.workLibraries);
    out << "\n";
    out << "}\n";
    return out.str();
}

std::string toJson (const PatchDocumentManifest& manifest)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"patchDocument\",\n";
    out << "  \"id\": " << quote (manifest.id) << ",\n";
    out << "  \"title\": " << quote (manifest.title) << ",\n";
    out << "  \"editorGraph\": { \"kind\": " << quote (manifest.editorGraphKind) << " },\n";
    out << "  \"runtimeGraph\": { \"kind\": " << quote (manifest.runtimeGraphKind) << " },\n";
    out << "  \"portBindings\": { \"kind\": " << quote (manifest.portBindingsKind) << " }\n";
    out << "}\n";
    return out.str();
}

std::string toJson (const ModulePackageManifest& manifest)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"modulePackage\",\n";
    out << "  \"id\": " << quote (manifest.id) << ",\n";
    out << "  \"title\": " << quote (manifest.title) << ",\n";
    out << "  \"patchPath\": " << quote (manifest.patchPath) << ",\n";
    out << "  \"humanDocPath\": " << quote (manifest.humanDocPath) << ",\n";
    out << "  \"publicPorts\": ";
    appendStringArray (out, manifest.publicPorts);
    out << "\n";
    out << "}\n";
    return out.str();
}

bool isKnownSaveStatus (const std::string& status)
{
    static constexpr std::array<const char*, 6> statuses {
        "clean",
        "saved-and-committed",
        "save-ok commit-pending",
        "save-ok commit-failed",
        "validation-failed",
        "write-failed"
    };

    return std::find (statuses.begin(), statuses.end(), status) != statuses.end();
}
}
