#include "RuntimeRegistry.h"

#include "CompoundPatch.h"
#include "StorageContract.h"

#include <filesystem>
#include <iomanip>
#include <sstream>

namespace myworld
{
namespace
{
std::string jsonEscaped (const std::string& text)
{
    std::ostringstream out;

    for (const auto character : text)
    {
        switch (character)
        {
            case '"':  out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\b': out << "\\b"; break;
            case '\f': out << "\\f"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:
                if (static_cast<unsigned char> (character) < 0x20)
                    out << "\\u" << std::hex << std::setw (4) << std::setfill ('0')
                        << static_cast<int> (static_cast<unsigned char> (character));
                else
                    out << character;
                break;
        }
    }

    return out.str();
}

void appendStringArray (std::ostringstream& out, const std::vector<std::string>& values)
{
    out << "[";

    for (size_t index = 0; index < values.size(); ++index)
    {
        if (index != 0)
            out << ", ";

        out << "\"" << jsonEscaped (values[index]) << "\"";
    }

    out << "]";
}

std::string fallback (const std::string& value, const std::string& fallbackValue)
{
    return value.empty() ? fallbackValue : value;
}

std::string resolvePathNear (const std::string& anchorPath, const std::string& candidatePath)
{
    namespace fs = std::filesystem;

    const fs::path candidate { candidatePath };
    if (candidate.is_absolute() || fs::exists (candidate))
        return candidate.string();

    auto directory = fs::path { anchorPath };
    directory = directory.has_parent_path() ? directory.parent_path() : fs::current_path();

    for (int depth = 0; depth < 8; ++depth)
    {
        const auto resolved = directory / candidate;
        if (fs::exists (resolved))
            return resolved.string();

        if (! directory.has_parent_path() || directory == directory.parent_path())
            break;

        directory = directory.parent_path();
    }

    return candidate.string();
}

std::vector<std::string> portIds (const std::vector<CompoundPublicPort>& ports)
{
    std::vector<std::string> ids;
    ids.reserve (ports.size());

    for (const auto& port : ports)
        ids.push_back (port.id);

    return ids;
}

RuntimeRegistryEntry makeRuntimeRegistryEntry (const ModulePackageManifest& module, const CompoundPatchSpec& compound)
{
    return {
        fallback (module.nodeType, compound.type),
        fallback (module.title, compound.displayName),
        fallback (module.runtimeDomain, "graph"),
        "compound.patch",
        fallback (module.previewPolicy, "none"),
        compound.children.size(),
        compound.internalEdges.size(),
        portIds (compound.publicInputs),
        portIds (compound.publicOutputs),
        makeCompoundCookOrder (compound)
    };
}
}

RuntimeRegistryLoadResult loadRuntimeRegistryFromModuleLibrary (const std::string& libraryPath)
{
    const auto resolvedLibraryPath = resolvePathNear ({}, libraryPath);
    const auto library = loadModuleLibraryManifest (resolvedLibraryPath);
    if (! library.ok)
        return { false, {}, library.error };

    RuntimeRegistry registry;

    for (const auto& modulePath : library.manifest.modulePackages)
    {
        const auto resolvedModulePath = resolvePathNear (resolvedLibraryPath, modulePath);
        const auto module = loadModulePackageManifest (resolvedModulePath);
        if (! module.ok)
            return { false, {}, module.error };

        const auto resolvedCompoundPath = resolvePathNear (resolvedModulePath, module.manifest.patchPath);
        const auto compound = loadCompoundPatchSpec (resolvedCompoundPath);
        if (! compound.ok)
            return { false, {}, compound.error };

        if (! isValidCompoundPatchSpec (compound.spec))
            return { false, {}, "compound patch is not valid for runtime registry: " + compound.spec.type };

        registry.entries.push_back (makeRuntimeRegistryEntry (module.manifest, compound.spec));
    }

    return { true, registry, {} };
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
        out << "      \"nodeType\": \"" << jsonEscaped (entry.nodeType) << "\",\n";
        out << "      \"displayName\": \"" << jsonEscaped (entry.displayName) << "\",\n";
        out << "      \"runtimeDomain\": \"" << jsonEscaped (entry.runtimeDomain) << "\",\n";
        out << "      \"executionKind\": \"" << jsonEscaped (entry.executionKind) << "\",\n";
        out << "      \"previewPolicy\": \"" << jsonEscaped (entry.previewPolicy) << "\",\n";
        out << "      \"childCount\": " << entry.childCount << ",\n";
        out << "      \"internalEdgeCount\": " << entry.internalEdgeCount << ",\n";
        out << "      \"publicInputs\": ";
        appendStringArray (out, entry.publicInputs);
        out << ",\n";
        out << "      \"publicOutputs\": ";
        appendStringArray (out, entry.publicOutputs);
        out << ",\n";
        out << "      \"cookOrder\": ";
        appendStringArray (out, entry.cookOrder);
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
}
