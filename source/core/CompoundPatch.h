#pragma once

#include "GraphContract.h"

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

struct CompoundPatchLoadResult
{
    bool ok = false;
    CompoundPatchSpec spec;
    std::string error;
};

CompoundPatchSpec makeLoudnessCompoundPatchSpec();
const CompoundChildNode* findCompoundChild (const CompoundPatchSpec& spec, const std::string& childId);
const CompoundPublicPort* findCompoundPublicOutput (const CompoundPatchSpec& spec, const std::string& outputId);
std::vector<std::string> makeCompoundCookOrder (const CompoundPatchSpec& spec);
bool isValidCompoundPatchSpec (const CompoundPatchSpec& spec);
std::string makeCompoundPatchJson (const CompoundPatchSpec& spec);
CompoundPatchLoadResult parseCompoundPatchJson (const std::string& text);
CompoundPatchLoadResult loadCompoundPatchSpec (const std::string& path);
std::string compoundPatchChildNodeId (const std::string& parentNodeId, const std::string& childId);
GraphContract makeCompoundPatchInteractionGraph (const CompoundPatchSpec& spec, const std::string& parentNodeId);
GraphContract makeCompoundPatchInteractionGraph (const CompoundPatchSpec& spec,
                                                 const std::string& parentNodeId,
                                                 const GraphContract& rootGraph);
bool storeCompoundPatchInteractionLayout (GraphContract& rootGraph,
                                          const std::string& parentNodeId,
                                          const GraphContract& expandedGraph);
}
