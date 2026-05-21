#include "PatchInteraction.h"

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
    expect (myworld::isKnownPatchGesture ("zoom"), "zoom gesture");
    expect (myworld::isKnownPatchGesture ("pan"), "pan gesture");
    expect (myworld::isKnownPatchGesture ("select"), "select gesture");
    expect (myworld::isKnownPatchGesture ("frame_selection"), "frame selection gesture");
    expect (myworld::isKnownPatchGesture ("drag_pin_to_empty_canvas"), "pin to empty canvas gesture");
    expect (myworld::isKnownPatchGesture ("drag_pin_to_pin"), "pin to pin gesture");
    expect (myworld::isKnownPatchGesture ("override_parameter_with_connection"), "parameter override gesture");
    expect (myworld::isKnownPatchGesture ("enter_compound"), "enter compound gesture");
    expect (myworld::isKnownPatchGesture ("collapse_compound"), "collapse compound gesture");
    expect (myworld::isKnownPatchGesture ("undo"), "undo gesture");
    expect (myworld::isKnownPatchGesture ("redo"), "redo gesture");
    expect (! myworld::isKnownPatchGesture ("edit_json_directly"), "direct json edit forbidden");

    expect (myworld::commandForGesture ("zoom") == "set_view", "zoom command");
    expect (myworld::commandForGesture ("pan") == "set_view", "pan command");
    expect (myworld::commandForGesture ("select") == "select", "select command");
    expect (myworld::commandForGesture ("frame_selection") == "set_view", "frame selection command");
    expect (myworld::commandForGesture ("drag_pin_to_empty_canvas") == "create_node+connect", "node search lowers to create and connect");
    expect (myworld::commandForGesture ("drag_pin_to_pin") == "connect", "pin to pin command");
    expect (myworld::commandForGesture ("override_parameter_with_connection") == "set_port_binding", "parameter override command");
    expect (myworld::commandForGesture ("enter_compound") == "enter_patch", "enter compound command");
    expect (myworld::commandForGesture ("collapse_compound") == "exit_patch", "collapse compound command");
    expect (myworld::commandForGesture ("undo") == "undo", "undo command");
    expect (myworld::commandForGesture ("redo") == "redo", "redo command");

    std::cout << "patch interaction grammar ok\n";
    return 0;
}
