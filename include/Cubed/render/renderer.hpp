#pragma once

#include "Cubed/config.hpp"
#include "Cubed/constants.hpp"
#include "Cubed/input/event.hpp"
#include "Cubed/primitive_data.hpp"
#include "Cubed/render/player_renderer.hpp"
#include "Cubed/render/shader_manager.hpp"
#include "Cubed/render/vertex_array.hpp"
#include "Cubed/render/vertex_buffer.hpp"
#include "Cubed/render/world_renderer.hpp"
#include "Cubed/shader.hpp"
#include "Cubed/ui/image.hpp"
#include "Cubed/ui/label.hpp"
#include "Cubed/ui/rect.hpp"

#include <glm/glm.hpp>
#include <vector>
namespace Cubed {
class TextureManager;
class ClientWorld;
class DevPanel;
class Renderer {
public:
    constexpr static int NUM_VAO = 7;

    Renderer(TextureManager& texture_manager, Config& config);
    ~Renderer();
    void hot_reload();
    void init(bool debug_on);
    const Shader& get_shader(const std::string& name) const;
    void begin_frame();
    void end_frame();
    void render_world(ClientWorld& world);
    void render_lable(const Label& label);
    void render_image(const Image& image);
    void render_rect(const Rect& rect);
    void begin_render_ui();
    void end_render_ui();

    void update(float delta_time);
    void update_fov(float fov);

    float& ambient_strength();

    bool& discard_transparent();
    bool& shader_on();
    bool& water_perturb();
    bool& water_depth_fade();
    bool& pbr();
    bool& flip_y();
    int& shadow_mode();
    int& light_cull_face();
    int& light_size_uv();
    float& min_radius();
    float& max_radius();
    int& samples();
    float& specular_strength();
    float& cloud_speed();
    float& cloud_threshold_low();
    float& cloud_threshold_high();
    float& refract_strength();

    float& underwater_fog_density();
    float& water_density();

    const glm::mat4& world_proj_matrix() const;
    const TextureManager& texture_mamger() const;
    float delta_time() const;

    float window_height() const;
    float window_width() const;
    float frame_height() const;
    float frame_width() const;
    const glm::mat4& p_mat() const;

    const std::vector<VertexArray>& vao() const;
    void render_dev_panel(DevPanel& dev_panel);

    bool handle_event(const Event& e);

private:
    TextureManager& m_texture_manager;

    bool m_init = false;

    float m_aspect = 0.0f;
    float m_fov = DEFAULT_FOV;

    float m_delta_time = 0.0f;

    float m_frame_width = 0.0f;
    float m_frame_height = 0.0f;

    float m_window_width = 0.0f;
    float m_window_height = 0.0f;

    glm::mat4 m_world_proj_matrix;

    std::unique_ptr<VertexBuffer> m_sky_vbo;
    std::unique_ptr<VertexBuffer> m_outline_indices_vbo;
    std::unique_ptr<VertexBuffer> m_outline_vbo;
    std::unique_ptr<VertexBuffer> m_ui_vbo;
    std::unique_ptr<VertexBuffer> m_player_vbo;
    std::unique_ptr<VertexBuffer> m_quad_vbo;

    glm::mat4 m_ui_proj_matrix;
    ShaderManager m_shaders;

    /*
    0 - quad vao (center)
    1 - sky vao
    2 - outline vao
    3 - ui vao (top-left)
    4 - text vao
    */
    std::vector<VertexArray> m_vao;
    std::vector<Vertex2D> m_ui;

    WorldRenderer m_world_renderer;
    Config& m_config;

    bool handle_window_resize_event(const WindowResizeEvent& e);
    bool handle_frame_buffer_resize_event(const FrameBufferResizeEvent& e);
    void updata_framebuffer(int width, int height);
    void init_quad();
    void init_text();
};

} // namespace Cubed