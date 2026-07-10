#pragma once

#include "Cubed/shader.hpp"

#include <string>
#include <unordered_map>
namespace Cubed {
class ShaderManager {
public:
    ShaderManager();
    ~ShaderManager();
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager(ShaderManager&&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;
    ShaderManager& operator=(ShaderManager&&) = delete;

    void init();

    const Shader& get_shader(const std::string& name) const;

private:
    std::unordered_map<std::string, Shader> m_shaders;

    void register_shader(const std::string& name, const std::string& v_shader,
                         const std::string& f_shader);
};

} // namespace Cubed