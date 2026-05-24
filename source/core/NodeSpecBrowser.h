#pragma once

#include "NodeSpec.h"

#include <string>
#include <vector>

namespace myworld
{
enum class BrowserPortContextDirection
{
    draggedOutput,
    draggedInput
};

struct BrowserPortContext
{
    BrowserPortContextDirection direction = BrowserPortContextDirection::draggedOutput;
    std::string dataType;
};

struct BrowserNodeEntry
{
    std::string savedType;
    std::string displayName;
    std::vector<std::string> taxonomyPath;
    std::vector<std::string> aliases;
    std::string description;
    std::vector<PortSpec> inputs;
    std::vector<PortSpec> outputs;
    bool runtimeReady = false;
    bool hidden = false;
    bool visibleByDefault = true;
};

std::vector<BrowserNodeEntry> makeBrowserNodeEntries (const std::vector<NodeSpec>& specs);
std::vector<BrowserNodeEntry> searchBrowserNodeEntries (const std::vector<BrowserNodeEntry>& entries,
                                                        const std::string& query);
std::vector<BrowserNodeEntry> compatibleBrowserNodeEntries (const std::vector<BrowserNodeEntry>& entries,
                                                            const BrowserPortContext& context);
}
