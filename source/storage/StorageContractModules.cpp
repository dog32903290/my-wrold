#include "StorageContract.h"

#include "JsonWriter.h"

#include <cctype>
#include <fstream>
#include <sstream>

namespace myworld
{
namespace
{
size_t findMemberValue (const std::string& text, const std::string& key)
{
    const auto keyToken = "\"" + key + "\"";
    const auto keyPosition = text.find (keyToken);
    if (keyPosition == std::string::npos)
        return std::string::npos;

    const auto colon = text.find (':', keyPosition + keyToken.size());
    if (colon == std::string::npos)
        return std::string::npos;

    auto valuePosition = colon + 1;
    while (valuePosition < text.size() && std::isspace (static_cast<unsigned char> (text[valuePosition])) != 0)
        ++valuePosition;

    return valuePosition;
}

std::string parseJsonStringAt (const std::string& text, size_t position)
{
    if (position >= text.size() || text[position] != '"')
        return {};

    std::string result;
    ++position;

    while (position < text.size())
    {
        const auto current = text[position++];

        if (current == '"')
            return result;

        if (current == '\\')
        {
            if (position >= text.size())
                return {};

            const auto escaped = text[position++];
            switch (escaped)
            {
                case '"':  result.push_back ('"'); break;
                case '\\': result.push_back ('\\'); break;
                case 'n':  result.push_back ('\n'); break;
                case 'r':  result.push_back ('\r'); break;
                case 't':  result.push_back ('\t'); break;
                default:   result.push_back (escaped); break;
            }
        }
        else
        {
            result.push_back (current);
        }
    }

    return {};
}

std::string stringMember (const std::string& text, const std::string& key)
{
    const auto position = findMemberValue (text, key);
    return position == std::string::npos ? std::string {} : parseJsonStringAt (text, position);
}

std::vector<std::string> stringArrayMember (const std::string& text, const std::string& key)
{
    std::vector<std::string> result;
    auto position = findMemberValue (text, key);

    if (position == std::string::npos || position >= text.size() || text[position] != '[')
        return result;

    ++position;

    while (position < text.size())
    {
        while (position < text.size() && std::isspace (static_cast<unsigned char> (text[position])) != 0)
            ++position;

        if (position < text.size() && text[position] == ']')
            return result;

        const auto value = parseJsonStringAt (text, position);
        if (value.empty())
            return {};

        result.push_back (value);
        position = text.find_first_of (",]", position + 1);

        if (position == std::string::npos)
            return {};

        if (text[position] == ']')
            return result;

        ++position;
    }

    return {};
}
}

ModulePackageManifest makeModulePackage (const std::string& id, const std::string& title, const std::string& patchPath)
{
    return {
        id,
        title,
        id,
        patchPath,
        "docs/manual.md",
        "compound",
        "feature",
        "graph",
        "none",
        { "in", "out" }
    };
}

ModuleLibraryManifest makeModuleLibrary (const std::string& id,
                                         const std::string& title,
                                         const std::vector<std::string>& modulePackages)
{
    return { id, title, modulePackages };
}

std::string toJson (const ModulePackageManifest& manifest)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"modulePackage\",\n";
    out << "  \"id\": " << jsonQuoted (manifest.id) << ",\n";
    out << "  \"title\": " << jsonQuoted (manifest.title) << ",\n";
    out << "  \"nodeType\": " << jsonQuoted (manifest.nodeType) << ",\n";
    out << "  \"patchPath\": " << jsonQuoted (manifest.patchPath) << ",\n";
    out << "  \"humanDocPath\": " << jsonQuoted (manifest.humanDocPath) << ",\n";
    out << "  \"category\": " << jsonQuoted (manifest.category) << ",\n";
    out << "  \"subcategory\": " << jsonQuoted (manifest.subcategory) << ",\n";
    out << "  \"runtimeDomain\": " << jsonQuoted (manifest.runtimeDomain) << ",\n";
    out << "  \"previewPolicy\": " << jsonQuoted (manifest.previewPolicy) << ",\n";
    out << "  \"publicPorts\": ";
    appendJsonStringArray (out, manifest.publicPorts);
    out << "\n";
    out << "}\n";
    return out.str();
}

std::string toJson (const ModuleLibraryManifest& manifest)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"moduleLibrary\",\n";
    out << "  \"id\": " << jsonQuoted (manifest.id) << ",\n";
    out << "  \"title\": " << jsonQuoted (manifest.title) << ",\n";
    out << "  \"modulePackages\": ";
    appendJsonStringArray (out, manifest.modulePackages);
    out << "\n";
    out << "}\n";
    return out.str();
}

ModulePackageLoadResult parseModulePackageManifest (const std::string& text)
{
    ModulePackageManifest manifest {
        stringMember (text, "id"),
        stringMember (text, "title"),
        stringMember (text, "nodeType"),
        stringMember (text, "patchPath"),
        stringMember (text, "humanDocPath"),
        stringMember (text, "category"),
        stringMember (text, "subcategory"),
        stringMember (text, "runtimeDomain"),
        stringMember (text, "previewPolicy"),
        stringArrayMember (text, "publicPorts")
    };

    if (stringMember (text, "kind") != "modulePackage")
        return { false, {}, "module manifest kind must be modulePackage" };

    if (manifest.id.empty() || manifest.title.empty() || manifest.nodeType.empty() || manifest.patchPath.empty())
        return { false, {}, "module manifest is missing required identity fields" };

    if (manifest.humanDocPath.empty() || manifest.category.empty() || manifest.subcategory.empty()
        || manifest.runtimeDomain.empty() || manifest.previewPolicy.empty())
    {
        return { false, {}, "module manifest is missing node spec fields" };
    }

    if (manifest.publicPorts.empty())
        return { false, {}, "module manifest must expose public ports" };

    return { true, manifest, {} };
}

ModuleLibraryLoadResult parseModuleLibraryManifest (const std::string& text)
{
    ModuleLibraryManifest manifest {
        stringMember (text, "id"),
        stringMember (text, "title"),
        stringArrayMember (text, "modulePackages")
    };

    if (stringMember (text, "kind") != "moduleLibrary")
        return { false, {}, "module library kind must be moduleLibrary" };

    if (manifest.id.empty() || manifest.title.empty())
        return { false, {}, "module library is missing required identity fields" };

    if (manifest.modulePackages.empty())
        return { false, {}, "module library must list module packages" };

    return { true, manifest, {} };
}

ModulePackageLoadResult loadModulePackageManifest (const std::string& path)
{
    std::ifstream input (path);
    if (! input)
        return { false, {}, "could not open module manifest: " + path };

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return parseModulePackageManifest (buffer.str());
}

ModuleLibraryLoadResult loadModuleLibraryManifest (const std::string& path)
{
    std::ifstream input (path);
    if (! input)
        return { false, {}, "could not open module library: " + path };

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return parseModuleLibraryManifest (buffer.str());
}
}
