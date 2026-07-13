#pragma once

#include "Cubed/ui/column_layout.hpp"
#include "Cubed/ui/label.hpp"
#include "Cubed/ui/widget.hpp"

#include <unordered_map>

namespace Cubed {

class DebugCollector {
public:
    static DebugCollector& get();
    DebugCollector();

    void report(const std::string& name, std::string_view content);
    void init(int width, int height);
    Widget& get_widget();
    bool handle_event(const Event& e);

private:
    ColumnLayout m_widget;
    std::unordered_map<std::string, Label*> m_component;
};

} // namespace Cubed