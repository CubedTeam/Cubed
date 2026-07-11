#pragma once

#include "Cubed/config.hpp"
#include "Cubed/constants.hpp"
#include "Cubed/primitive_data.hpp"
#include "Cubed/render/player_renderer.hpp"
#include "Cubed/render/shader_manager.hpp"
#include "Cubed/render/vertex_array.hpp"
#include "Cubed/render/vertex_buffer.hpp"
#include "Cubed/render/world_renderer.hpp"
#include "Cubed/shader.hpp"
#include "Cubed/ui/label.hpp"

#include <glm/glm.hpp>
#include <vector>
namespace Cubed {

class Camera;
class TextureManager;
class ClientWorld;
class DevPanel;
class Renderer {
public:
    constexpr static int NUM_VAO = 7;

    Renderer(const Camera& camera, ClientWorld& world,
             const TextureManager& texture_manager, DevPanel& dev_panel,
             Config& config);
    ~Renderer();
    void hot_reload();
    void init(bool debug_on);
    const Shader& get_shader(const std::string& name) const;
    void render();

    void render_lable(const Label& label);

    void update(float delta_time);
    void update_fov(float fov);
    void update_proj_matrix(float aspect, float width, float height);
    void updata_framebuffer(int width, int height);
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

    const Camera& camera() const;
    const ClientWorld& world() const;
    ClientWorld& world();
    const glm::mat4& world_proj_matrix() const;
    const TextureManager& texture_mamger() const;
    float delta_time() const;

    float height() const;
    float width() const;
    const glm::mat4& p_mat() const;

    const std::vector<VertexArray>& vao() const;

private:
    const Camera& m_camera;
    DevPanel& m_dev_panel;
    const TextureManager& m_texture_manager;
    ClientWorld& m_world;

    bool m_init = false;

    float m_aspect = 0.0f;
    float m_fov = DEFAULT_FOV;

    float m_delta_time = 0.0f;

    float m_width = 0.0f;
    float m_height = 0.0f;

    glm::mat4 m_world_proj_matrix;

    std::unique_ptr<VertexBuffer> m_sky_vbo;
    std::unique_ptr<VertexBuffer> m_outline_indices_vbo;
    std::unique_ptr<VertexBuffer> m_outline_vbo;
    std::unique_ptr<VertexBuffer> m_ui_vbo;
    std::unique_ptr<VertexBuffer> m_player_vbo;
    std::unique_ptr<VertexBuffer> m_quad_vbo;

    glm::mat4 m_ui_proj_matrix;
    glm::mat4 m_ui_model_matrix;
    ShaderManager m_shaders;

    /*
    0 - quad vao
    1 - sky vao
    2 - outline vao
    3 - ui vao
    4 - text vao
    */
    std::vector<VertexArray> m_vao;
    std::vector<Vertex2D> m_ui;

    WorldRenderer m_world_renderer;
    Config& m_config;
    void init_quad();
    void init_text();

    void day_night_calculation();

    void render_sky();

    void render_ui();
    void render_crosshair();
    void render_dev_panel();
};

} // namespace Cubed