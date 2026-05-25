#include "GraphLanguage.h"

#include <algorithm>
#include <array>

namespace myworld
{
namespace
{
template <size_t size>
bool contains (const std::array<const char*, size>& values, const std::string& value)
{
    return std::find (values.begin(), values.end(), value) != values.end();
}
}

bool isKnownRegionType (const std::string& type)
{
    static constexpr std::array<const char*, 4> values { "if", "for_each", "repeat", "while" };
    return contains (values, type);
}

bool isKnownTypeSpec (const std::string& type)
{
    static constexpr std::array<const char*, 15> values {
        "audio.channels",
        "audio.mono",
        "signal.float",
        "texture.rgba",
        "geometry.mesh",
        "point.cloud",
        "field.scalar",
        "material.shader",
        "event.midi",
        "event.osc",
        "command.graph",
        "text.string",
        "data.object",
        "resource.file",
        "generic<T>"
    };
    return contains (values, type);
}

bool isKnownStreamKind (const std::string& kind)
{
    static constexpr std::array<const char*, 4> values { "continuous", "event", "command", "resource" };
    return contains (values, kind);
}

bool isKnownPortBindingMode (const std::string& mode)
{
    static constexpr std::array<const char*, 4> values { "default", "manual", "connected", "animated" };
    return contains (values, mode);
}

bool isKnownCommandType (const std::string& type)
{
    static constexpr std::array<const char*, 24> values {
        "create_node",
        "delete_node",
        "create_region",
        "connect",
        "disconnect",
        "reconnect",
        "split_edge_create_node",
        "connect_hidden_input",
        "multi_input_insert",
        "insert_node_on_edge",
        "snap_connect",
        "unsnap_disconnect",
        "shake_disconnect",
        "move_node",
        "set_param",
        "set_port_binding",
        "set_view",
        "select",
        "enter_patch",
        "exit_patch",
        "publish_module",
        "save_work",
        "undo",
        "redo"
    };
    return contains (values, type);
}

bool isValidTypedEdge (const TypedEdge& edge)
{
    if (! isKnownTypeSpec (edge.dataType) || ! isKnownStreamKind (edge.streamKind))
        return false;

    if (edge.dataType.rfind ("event.", 0) == 0)
        return edge.streamKind == "event";

    if (edge.dataType.rfind ("command.", 0) == 0)
        return edge.streamKind == "command";

    if (edge.dataType.rfind ("resource.", 0) == 0)
        return edge.streamKind == "resource";

    return edge.streamKind == "continuous";
}

bool isValidPortBinding (const PortBinding& binding)
{
    return isKnownTypeSpec (binding.dataType) && isKnownPortBindingMode (binding.bindingMode);
}

bool isAllowedCompilerWorkerLanguage (const std::string& language)
{
    static constexpr std::array<const char*, 2> values { "c++", "c#" };
    return contains (values, language);
}

bool isAllowedRealtimeRuntimeLanguage (const std::string& language)
{
    static constexpr std::array<const char*, 1> values { "c++" };
    return contains (values, language);
}
}
