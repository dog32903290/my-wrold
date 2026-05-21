#include "NodeSpec.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
void expect (bool condition, const std::string& message)
{
    if (! condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        std::exit (1);
    }
}
}

int main()
{
    const auto specs = myworld::makeSeedNodeSpecs();

    const auto* shader = myworld::findNodeSpec (specs, "shader.fragment");
    expect (shader != nullptr, "shader.fragment seed spec exists");
    expect (shader->category == "shader", "shader category");
    expect (shader->subcategory == "use", "shader subcategory");
    expect (shader->runtimeDomain == "render", "shader runtime domain");
    expect (shader->previewPolicy == "tiny_preview", "shader preview policy");
    expect (shader->outputs.size() == 1, "shader output count");
    expect (shader->outputs[0].dataType == "texture.rgba", "shader output data type");

    const auto* loudness = myworld::findNodeSpec (specs, "analyzer.loudness");
    expect (loudness != nullptr, "analyzer.loudness seed spec exists");
    expect (loudness->category == "analyzer", "loudness category");
    expect (loudness->subcategory == "feature", "loudness subcategory");
    expect (loudness->runtimeDomain == "audioAnalysis", "loudness runtime domain");
    expect (loudness->previewPolicy == "meter_scope", "loudness preview policy");
    expect (loudness->inputs[0].dataType == "audio.mono", "loudness input data type");
    expect (loudness->outputs[0].dataType == "signal.float", "loudness output data type");
    expect (loudness->humanDocPath == "docs/nodes/analyzer.loudness.md", "loudness human manual path");
    expect (loudness->machineSpecVersion == 1, "machine spec version");

    const auto* midi = myworld::findNodeSpec (specs, "io.midi.cc_out");
    expect (midi != nullptr, "io.midi.cc_out seed spec exists");
    expect (midi->category == "io", "midi category is io");
    expect (midi->subcategory == "midi", "midi subcategory");
    expect (midi->runtimeDomain == "control", "midi runtime domain");

    const auto* texture = myworld::findNodeSpec (specs, "image.texture");
    expect (texture != nullptr, "image.texture seed spec exists");
    expect (texture->category == "image", "texture category");
    expect (texture->subcategory == "use", "texture subcategory");

    const auto* mesh = myworld::findNodeSpec (specs, "mesh.plane");
    expect (mesh != nullptr, "mesh.plane seed spec exists");
    expect (mesh->category == "mesh", "mesh category");
    expect (mesh->subcategory == "generate", "mesh subcategory");

    expect (myworld::isKnownNodeCategory ("audio"), "audio category is known");
    expect (myworld::isKnownNodeCategory ("image"), "image category is known");
    expect (myworld::isKnownNodeCategory ("mesh"), "mesh category is known");
    expect (myworld::isKnownNodeCategory ("io"), "io category is known");
    expect (myworld::isKnownNodeSubcategory ("midi"), "midi subcategory is known");
    expect (myworld::isKnownNodeSubcategory ("use"), "use subcategory is known");
    expect (myworld::isKnownNodeSubcategory ("measurement"), "measurement subcategory is known");
    expect (myworld::isKnownPreviewPolicy ("none"), "none preview policy is known");
    expect (myworld::isKnownPreviewPolicy ("tiny_preview"), "tiny preview policy is known");
    expect (myworld::isKnownPreviewPolicy ("meter_scope"), "meter scope preview policy is known");
    expect (myworld::isKnownNodeCategoryAlias ("top"), "top is accepted as a browser alias");
    expect (! myworld::isKnownNodeCategory ("top"), "top is not first-level category law");
    expect (! myworld::isKnownNodeCategory ("weather"), "unknown category stays unknown until registry extension");

    std::cout << "node spec contract ok\n";
    return 0;
}
