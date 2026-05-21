#include "PatchInteraction.h"

#include <array>
#include <utility>

namespace myworld
{
namespace
{
using GestureCommand = std::pair<const char*, const char*>;

static constexpr std::array<GestureCommand, 11> gestures {
    GestureCommand { "zoom", "set_view" },
    GestureCommand { "pan", "set_view" },
    GestureCommand { "select", "select" },
    GestureCommand { "frame_selection", "set_view" },
    GestureCommand { "drag_pin_to_empty_canvas", "create_node+connect" },
    GestureCommand { "drag_pin_to_pin", "connect" },
    GestureCommand { "override_parameter_with_connection", "set_port_binding" },
    GestureCommand { "enter_compound", "enter_patch" },
    GestureCommand { "collapse_compound", "exit_patch" },
    GestureCommand { "undo", "undo" },
    GestureCommand { "redo", "redo" }
};
}

bool isKnownPatchGesture (const std::string& gesture)
{
    for (const auto& entry : gestures)
        if (gesture == entry.first)
            return true;

    return false;
}

std::string commandForGesture (const std::string& gesture)
{
    for (const auto& entry : gestures)
        if (gesture == entry.first)
            return entry.second;

    return {};
}
}
