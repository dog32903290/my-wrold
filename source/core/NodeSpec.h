#pragma once

#include <string>
#include <vector>

namespace myworld
{
struct PortSpec
{
    std::string id;
    std::string label;
    std::string dataType;
    std::string direction;
};

struct ParamSpec
{
    std::string id;
    std::string label;
    std::string dataType;
    std::string defaultValue;
    std::string range;
    std::string group;
    std::string description;
    bool excludeFromPresets = false;
    std::string visibleWhenParamId;
    std::string visibleWhenValue;
};

struct NodeSpec
{
    std::string type;
    std::string displayName;
    std::string category;
    std::string subcategory;
    std::string runtimeDomain;
    std::string previewPolicy;
    std::string humanDocPath;
    int machineSpecVersion = 1;
    std::vector<PortSpec> inputs;
    std::vector<PortSpec> outputs;
    std::vector<ParamSpec> params;
};

std::vector<NodeSpec> makeSeedNodeSpecs();
const NodeSpec* findNodeSpec (const std::vector<NodeSpec>& specs, const std::string& type);
bool isKnownNodeCategory (const std::string& category);
bool isKnownNodeSubcategory (const std::string& subcategory);
bool isKnownNodeCategoryAlias (const std::string& alias);
bool isKnownPreviewPolicy (const std::string& policy);
}
