#pragma once
#include "Cubed/render/frame_buffer.hpp"
#include "Cubed/render/player_renderer.hpp"
#include "Cubed/render/texture.hpp"

#include <glm/glm.hpp>
#include <memory>
namespace Cubed {
class Renderer;
class ClientWorld;
class TextureManager;
class Camera;
class WorldRenderer {
public:
    struct ParallelLight {
        glm::vec3 sundir; // direction from sun to vertex
        glm::vec3 lightdir;
        float sun_height = 0.0f;
        float day_light = 0.0f;
        float day_factor = 0.0f;
        glm::vec3 sun_color;
        glm::vec3 directional_light_color;
        glm::vec3 finnal_ambient_color;
        glm::mat4 light_space_matrix;
    };

    struct SkyUniform {
        glm::vec3 sky_top;
        glm::vec3 sky_bottom;
        glm::vec3 sun_dir_view;
        float horizon_sharpness;
        float cloud_white_mix;
    };
    WorldRenderer(Renderer& renderer);
    ~WorldRenderer();

    WorldRenderer(const WorldRenderer&) = delete;
    WorldRenderer(WorldRenderer&&) = delete;
    WorldRenderer& operator=(const WorldRenderer&) = delete;
    WorldRenderer& operator=(WorldRenderer&&) = delete;
    void init();
    void render();
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

    const FrameBuffer* world_fbo() const;

private:
    Renderer& m_renderer;
    PlayerRenderer m_player_renderer;
    std::unique_ptr<Texture> m_accum_texture;
    std::unique_ptr<Texture> m_reveal_texture;

    std::unique_ptr<FrameBuffer> m_world_fbo;
    std::unique_ptr<Texture> m_screen_texture;
    std::unique_ptr<Texture> m_screen_depth_texture;

    std::unique_ptr<FrameBuffer> m_oit_fbo;

    std::unique_ptr<Texture> m_oit_depth_texture;

    std::unique_ptr<FrameBuffer> m_depth_map_fbo;
    std::unique_ptr<Texture> m_depth_map_texture;

    glm::vec3 m_blend_from_lightdir;
    glm::vec3 m_blend_to_lightdir;
    float m_blend_t = 1.0f;
    bool m_blend_initialized = false;
    static constexpr float BLEND_DURATION = 0.15f;

    float m_underwater_fog_density = 0.08f;

    float m_water_density = 0.12f;

    float m_ambient_strength = 0.1f;
    bool m_discard_tranparent = true;
    bool m_shader_on = true;
    bool m_water_perturb = true;
    bool m_water_depth_fade = true;
    bool m_pbr = true;
    bool m_flip_y = false;
    int m_shadow_mode = 0;
    int m_light_size_uv = 20;
    float m_min_radius = 2.0f;
    float m_max_radius = 20.0f;
    int m_samples = 16;
    float m_specular_strength = 0.5f;
    float m_cloud_threshold_low = 0.5f;
    float m_cloud_threshold_high = 0.75f;
    float m_cloud_time = 0.0f;
    float m_cloud_speed = 5.0f;
    float m_refract_strength = 0.03f;
    int m_light_cull_face = 0;

    float moon_intensity = 0.3f;
    float sun_intensity = 1.00f;

    ParallelLight m_parallel_light;
    SkyUniform m_sky_uniform;

    glm::mat4 view_matrix;

    ClientWorld& m_world;
    const Camera& m_camera;
    const TextureManager& m_texture_manager;
    void day_night_calculation();

    void render_sky();

    void render_world();

    void shadow_map_generate();

    void render_underwater();
    void render_outline();
    void render_player();

    void render_normal_block(const glm::mat4& model_mat,
                             const glm::mat4& mv_mat,
                             const glm::mat4& norm_mat);

    void render_transparent_block(const glm::mat4& mv_mat,
                                  const glm::mat4& norm_mat);

    glm::vec3 quantize_sun_direction(const glm::vec3& sundir,
                                     float angle_step_deg) const;
    glm::vec3 get_smoothed_shadow_lightdir(const glm::vec3& raw_shadow_sundir,
                                           float dt);
};
} // namespace Cubed