#include "NodeSpecBrowser.h"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace myworld
{
namespace
{
std::string normalizedText (const std::string& text)
{
    std::string result;
    bool previousWasSpace = true;

    for (const auto raw : text)
    {
        const auto c = static_cast<unsigned char> (raw);
        if (std::isalnum (c))
        {
            result.push_back (static_cast<char> (std::tolower (c)));
            previousWasSpace = false;
        }
        else if (! previousWasSpace)
        {
            result.push_back (' ');
            previousWasSpace = true;
        }
    }

    while (! result.empty() && result.back() == ' ')
        result.pop_back();

    return result;
}

std::vector<std::string> splitWords (const std::string& text)
{
    std::vector<std::string> words;
    std::istringstream stream (normalizedText (text));
    std::string word;

    while (stream >> word)
        words.push_back (word);

    return words;
}

std::string joinWords (const std::vector<std::string>& words)
{
    std::string result;

    for (const auto& word : words)
    {
        if (! result.empty())
            result.push_back (' ');

        result += word;
    }

    return result;
}

bool allQueryWordsInOrder (const std::string& field, const std::vector<std::string>& queryWords)
{
    auto searchFrom = std::string::size_type { 0 };

    for (const auto& word : queryWords)
    {
        const auto found = field.find (word, searchFrom);
        if (found == std::string::npos)
            return false;

        searchFrom = found + word.size();
    }

    return true;
}

std::string acronymFor (const std::string& field)
{
    std::string acronym;

    for (const auto& word : splitWords (field))
        if (! word.empty())
            acronym.push_back (word.front());

    return acronym;
}

bool startsWith (const std::string& text, const std::string& prefix)
{
    return text.size() >= prefix.size() && text.compare (0, prefix.size(), prefix) == 0;
}

bool wordEquals (const std::vector<std::string>& words, const std::string& query)
{
    return std::find (words.begin(), words.end(), query) != words.end();
}

bool wordStartsWith (const std::vector<std::string>& words, const std::string& query)
{
    return std::any_of (words.begin(), words.end(), [&query] (const auto& word)
    {
        return startsWith (word, query);
    });
}

int scoreField (const std::string& rawField,
                const std::string& query,
                const std::vector<std::string>& queryWords,
                int baseScore)
{
    const auto field = normalizedText (rawField);
    if (field.empty())
        return 1000000;

    const auto words = splitWords (field);

    if (field == query)
        return baseScore;

    if (wordEquals (words, query))
        return baseScore + 1;

    if (startsWith (field, query))
        return baseScore + 10;

    if (wordStartsWith (words, query))
        return baseScore + 12;

    if (field.find (query) != std::string::npos)
        return baseScore + 20;

    if (queryWords.size() > 1 && allQueryWordsInOrder (field, queryWords))
        return baseScore + 28;

    const auto acronym = acronymFor (field);
    if (! acronym.empty() && startsWith (acronym, query))
        return baseScore + 30;

    return 1000000;
}

void appendUnique (std::vector<std::string>& values, const std::string& value)
{
    if (! value.empty() && std::find (values.begin(), values.end(), value) == values.end())
        values.push_back (value);
}

std::string categoryAliasRoot (const std::string& category)
{
    if (category == "image")
        return "top";

    if (category == "mesh")
        return "sop";

    if (category == "material")
        return "mat";

    return {};
}

std::string typeSuffixAfterCategory (const NodeSpec& spec)
{
    const auto prefix = spec.category + ".";
    if (startsWith (spec.type, prefix))
        return spec.type.substr (prefix.size());

    return {};
}

std::vector<std::string> aliasesForSpec (const NodeSpec& spec)
{
    std::vector<std::string> aliases;
    const auto aliasRoot = categoryAliasRoot (spec.category);

    appendUnique (aliases, aliasRoot);
    if (! aliasRoot.empty())
    {
        appendUnique (aliases, aliasRoot + "." + spec.subcategory);
        appendUnique (aliases, aliasRoot + "." + typeSuffixAfterCategory (spec));
    }

    return aliases;
}

bool isHiddenPathToken (const std::string& token)
{
    return startsWith (token, "_")
        || token == "internal"
        || token == "obsolete"
        || token == "legacy"
        || token == "experimental";
}

bool isHiddenSpec (const NodeSpec& spec)
{
    return isHiddenPathToken (spec.category)
        || isHiddenPathToken (spec.subcategory)
        || startsWith (spec.type, "_")
        || spec.type.find ("._") != std::string::npos;
}

std::string descriptionForSpec (const NodeSpec& spec)
{
    std::ostringstream out;
    out << spec.displayName << ' '
        << spec.category << ' ' << spec.subcategory << ' '
        << spec.runtimeDomain << ' ' << spec.previewPolicy << ' '
        << spec.humanDocPath;

    for (const auto& port : spec.inputs)
        out << ' ' << port.label << ' ' << port.dataType;

    out << " to";

    for (const auto& port : spec.outputs)
        out << ' ' << port.label << ' ' << port.dataType;

    return out.str();
}

std::vector<std::string> primarySearchFields (const BrowserNodeEntry& entry)
{
    std::vector<std::string> fields {
        entry.displayName,
        entry.savedType,
        joinWords (entry.taxonomyPath)
    };

    for (const auto& alias : entry.aliases)
        fields.push_back (alias);

    return fields;
}

int scoreEntry (const BrowserNodeEntry& entry,
                const std::string& query,
                const std::vector<std::string>& queryWords)
{
    auto score = 1000000;

    for (const auto& field : primarySearchFields (entry))
        score = std::min (score, scoreField (field, query, queryWords, 0));

    score = std::min (score, scoreField (entry.description, query, queryWords, 40));
    return score;
}

bool hasMatchingPort (const std::vector<PortSpec>& ports, const std::string& dataType)
{
    return std::any_of (ports.begin(), ports.end(), [&dataType] (const auto& port)
    {
        return port.dataType == dataType;
    });
}
}

std::vector<BrowserNodeEntry> makeBrowserNodeEntries (const std::vector<NodeSpec>& specs)
{
    std::vector<BrowserNodeEntry> entries;
    entries.reserve (specs.size());

    for (const auto& spec : specs)
    {
        BrowserNodeEntry entry;
        entry.savedType = spec.type;
        entry.displayName = spec.displayName;
        entry.taxonomyPath = { spec.category, spec.subcategory };
        entry.aliases = aliasesForSpec (spec);
        entry.description = descriptionForSpec (spec);
        entry.inputs = spec.inputs;
        entry.outputs = spec.outputs;
        entry.runtimeReady = ! spec.runtimeDomain.empty();
        entry.hidden = isHiddenSpec (spec);
        entry.visibleByDefault = ! entry.hidden;
        entries.push_back (entry);
    }

    return entries;
}

std::vector<BrowserNodeEntry> searchBrowserNodeEntries (const std::vector<BrowserNodeEntry>& entries,
                                                        const std::string& query)
{
    const auto normalizedQuery = normalizedText (query);
    if (normalizedQuery.empty())
    {
        std::vector<BrowserNodeEntry> visibleEntries;
        for (const auto& entry : entries)
            if (entry.visibleByDefault)
                visibleEntries.push_back (entry);

        return visibleEntries;
    }

    const auto queryWords = splitWords (query);

    struct ScoredEntry
    {
        BrowserNodeEntry entry;
        int score = 0;
        size_t index = 0;
    };

    std::vector<ScoredEntry> scoredEntries;

    for (size_t index = 0; index < entries.size(); ++index)
    {
        const auto& entry = entries[index];
        if (! entry.visibleByDefault)
            continue;

        const auto score = scoreEntry (entry, normalizedQuery, queryWords);
        if (score < 1000000)
            scoredEntries.push_back ({ entry, score, index });
    }

    std::stable_sort (scoredEntries.begin(), scoredEntries.end(), [] (const auto& left, const auto& right)
    {
        if (left.score != right.score)
            return left.score < right.score;

        return left.index < right.index;
    });

    std::vector<BrowserNodeEntry> result;
    result.reserve (scoredEntries.size());

    for (const auto& scored : scoredEntries)
        result.push_back (scored.entry);

    return result;
}

std::vector<BrowserNodeEntry> compatibleBrowserNodeEntries (const std::vector<BrowserNodeEntry>& entries,
                                                            const BrowserPortContext& context)
{
    std::vector<BrowserNodeEntry> result;

    if (context.dataType.empty())
        return result;

    for (const auto& entry : entries)
    {
        if (! entry.visibleByDefault)
            continue;

        const auto& ports = context.direction == BrowserPortContextDirection::draggedOutput
            ? entry.inputs
            : entry.outputs;

        if (hasMatchingPort (ports, context.dataType))
            result.push_back (entry);
    }

    return result;
}
}
