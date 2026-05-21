#include "GraphLanguage.h"

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
    expect (myworld::isKnownRegionType ("if"), "if region");
    expect (myworld::isKnownRegionType ("for_each"), "for_each region");
    expect (! myworld::isKnownRegionType ("maze"), "unknown region");

    expect (myworld::isKnownTypeSpec ("audio.mono"), "audio.mono type");
    expect (myworld::isKnownTypeSpec ("signal.float"), "signal.float type");
    expect (myworld::isKnownTypeSpec ("point.cloud"), "point.cloud type");
    expect (myworld::isKnownTypeSpec ("material.shader"), "material.shader type");
    expect (myworld::isKnownTypeSpec ("generic<T>"), "generic type slot");
    expect (! myworld::isKnownTypeSpec ("mystery.blob"), "unknown type");

    expect (myworld::isKnownStreamKind ("continuous"), "continuous stream");
    expect (myworld::isKnownStreamKind ("event"), "event stream");
    expect (myworld::isKnownStreamKind ("command"), "command stream");
    expect (myworld::isKnownStreamKind ("resource"), "resource stream");
    expect (myworld::isKnownPortBindingMode ("default"), "default binding mode");
    expect (myworld::isKnownPortBindingMode ("manual"), "manual binding mode");
    expect (myworld::isKnownPortBindingMode ("connected"), "connected binding mode");
    expect (myworld::isKnownPortBindingMode ("animated"), "animated binding mode");
    expect (! myworld::isKnownPortBindingMode ("forgotten"), "unknown binding mode");

    const myworld::TypedEdge edge { "edge1", "audio1.mono", "loudness1.input", "audio.mono", "continuous" };
    expect (myworld::isValidTypedEdge (edge), "typed edge validates");

    const myworld::TypedEdge badEdge { "edge2", "midi1.note", "shader1.input", "event.midi", "continuous" };
    expect (! myworld::isValidTypedEdge (badEdge), "event data cannot use continuous stream kind");

    const myworld::PortBinding manualBinding { "gain", "signal.float", "manual" };
    expect (myworld::isValidPortBinding (manualBinding), "manual port binding validates");

    const myworld::PortBinding connectedBinding { "brightness", "signal.float", "connected" };
    expect (myworld::isValidPortBinding (connectedBinding), "connected port binding validates");

    const myworld::PortBinding badBinding { "mystery", "mystery.blob", "manual" };
    expect (! myworld::isValidPortBinding (badBinding), "unknown binding type rejected");

    expect (myworld::isKnownCommandType ("create_node"), "create_node command");
    expect (myworld::isKnownCommandType ("create_region"), "create_region command");
    expect (myworld::isKnownCommandType ("set_port_binding"), "set_port_binding command");
    expect (myworld::isKnownCommandType ("set_view"), "set_view command");
    expect (myworld::isKnownCommandType ("select"), "select command");
    expect (myworld::isKnownCommandType ("enter_patch"), "enter_patch command");
    expect (myworld::isKnownCommandType ("undo"), "undo command");
    expect (myworld::isKnownCommandType ("redo"), "redo command");
    expect (myworld::isKnownCommandType ("save_work"), "save_work command");
    expect (! myworld::isKnownCommandType ("edit_json_directly"), "direct json mutation forbidden");

    expect (myworld::isAllowedCompilerWorkerLanguage ("c++"), "c++ compiler worker language");
    expect (myworld::isAllowedCompilerWorkerLanguage ("c#"), "c# external compiler worker language");
    expect (! myworld::isAllowedRealtimeRuntimeLanguage ("c#"), "c# not allowed in realtime runtime");

    std::cout << "graph language contract ok\n";
    return 0;
}
