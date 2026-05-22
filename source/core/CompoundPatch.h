#pragma once

#include <string>
#include <vector>

namespace myworld
{
struct CompoundChildNode
{
    std::string id;
    std::string nodeType;
    std::string role;
};

struct CompoundInternalEdge
{
    std::string from;
    std::string to;
    std::string dataType;
};

struct CompoundPublicPort
{
    std::string id;
    std::string label;
    std::string dataType;
    std::string direction;
    std::string mapsTo;
};

struct CompoundPatchSpec
{
    std::string type;
    std::string displayName;
    bool collapsedByDefault = true;
    std::vector<CompoundChildNode> children;
    std::vector<CompoundInternalEdge> internalEdges;
    std::vector<CompoundPublicPort> publicInputs;
    std::vector<CompoundPublicPort> publicOutputs;
};

CompoundPatchSpec makeLoudnessCompoundPatchSpec();
const CompoundChildNode* findCompoundChild (const CompoundPatchSpec& spec, const std::string& childId);
const CompoundPublicPort* findCompoundPublicOutput (const CompoundPatchSpec& spec, const std::string& outputId);
std::vector<std::string> makeCompoundCookOrder (const CompoundPatchSpec& spec);
bool isValidCompoundPatchSpec (const CompoundPatchSpec& spec);
std::string makeCompoundPatchJson (const CompoundPatchSpec& spec);
}
