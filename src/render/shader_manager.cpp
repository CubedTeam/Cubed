#include "Cubed/render/shader_manager.hpp"

#include "Cubed/tools/cubed_assert.hpp"

namespace Cubed {
ShaderManager::ShaderManager() {}
ShaderManager::~ShaderManager() {}

void ShaderManager::init() {
    register_shader("normal_block", "shaders/block_v_shader.glsl",
                    "shaders/block_f_shader.glsl");
    register_shader("outline", "shaders/outline_v_shader.glsl",
                    "shaders/outline_f_shader.glsl");
    register_shader("sky", "shaders/sky_v_shader.glsl",
                    "shaders/sky_f_shader.glsl");
    register_shader("image", "shaders/image_v_shader.glsl",
                    "shaders/image_f_shader.glsl");
    register_shader("text", "shaders/text_v_shader.glsl",
                    "shaders/text_f_shader.glsl");
    register_shader("under_water", "shaders/under_water_v_shader.glsl",
                    "shaders/under_water_f_shader.glsl");
    register_shader("accum", "shaders/block_accumulation_v_shader.glsl",
                    "shaders/block_accumulation_f_shader.glsl");
    register_shader("composite", "shaders/block_composite_v_shader.glsl",
                    "shaders/block_composite_f_shader.glsl");
    register_shader("depth_shader", "shaders/depth_shader.glsl",
                    "shaders/depth_fragment_shader.glsl");
    register_shader("billboard", "shaders/billboard_v_shader.glsl",
                    "shaders/billboard_f_shader.glsl");
    register_shader("water", "shaders/water_v_shader.glsl",
                    "shaders/water_f_shader.glsl");
    register_shader("player", "shaders/player_v_shader.glsl",
                    "shaders/player_f_shader.glsl");
    register_shader("player_depth", "shaders/depth_player_shader.glsl",
                    "shaders/depth_player_fragment_shader.glsl");
    register_shader("rect", "shaders/rect_v_shader.glsl",
                    "shaders/rect_f_shader.glsl");
    register_shader("model_shader", "shaders/model_vert.glsl",
                    "shaders/model_frag.glsl");
}

void ShaderManager::register_shader(const std::string& name,
                                    const std::string& v_shader,
                                    const std::string& f_shader) {

    auto [_, inserted] = m_shaders.try_emplace(name, name, v_shader, f_shader);

    if (!inserted) {
        std::string msg = std::format("Shader name {} already esist!", name);
        ASSERT_MSG(false, msg);
        throw std::runtime_error(msg);
    }
}

const Shader& ShaderManager::get_shader(const std::string& name) const {
    auto it = m_shaders.find(name);
    if (it == m_shaders.end()) {
        std::string msg = std::format("Shader name {} not find", name);
        ASSERT_MSG(false, msg);
        throw std::runtime_error(msg);
    }
    return it->second;
}

} // namespace Cubed