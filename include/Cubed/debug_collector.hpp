#pragma once

#include "Cubed/ui/column_layout.hpp"
#include "Cubed/ui/label.hpp"
#include "Cubed/ui/widget.hpp"

#include <format>
#include <memory>
#include <unordered_map>

namespace Cubed {

class DebugCollector {
public:
    static DebugCollector& get();
    static void distory();
    DebugCollector();

    void report(const std::string& name, std::string_view content);
    void init(int width, int height);
    Widget& get_widget();
    bool handle_event(const Event& e);

private:
    ColumnLayout m_widget;
    std::unordered_map<std::string, Label*> m_component;
    static std::unique_ptr<DebugCollector>& get_ptr();
};

template <typename... Args>
void d_rep(const std::string& key, std::format_string<Args...> fmt,
           Args&&... args) {
    std::string msg = std::vformat(fmt.get(), std::make_format_args(args...));
    DebugCollector::get().report(key, msg);
}

} // namespace Cubed