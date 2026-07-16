#include "Cubed/render/world_renderer.hpp"

#include "Cubed/camera.hpp"
#include "Cubed/debug_collector.hpp"
#include "Cubed/gameplay/client_world.hpp"
#include "Cubed/render/renderer.hpp"
#include "Cubed/render/renderer_constants.hpp"
#include "Cubed/scene/world_scene.hpp"
#include "Cubed/texture_manager.hpp"
#include "Cubed/tools/math_tools.hpp"
namespace Cubed {
WorldRenderer::WorldRenderer(Renderer& renderer)
    : m_renderer(renderer), m_player_renderer(renderer),
      m_texture_manager(renderer.texture_mamger()) {}
WorldRenderer::~WorldRenderer() {
    m_accum_texture.reset();
    m_reveal_texture.reset();

    m_world_fbo.reset();
    m_screen_texture.reset();
    m_screen_depth_texture.reset();

    m_oit_fbo.reset();

    m_oit_depth_texture.reset();

    m_depth_map_fbo.reset();
    m_depth_map_texture.reset();
}

void WorldRenderer::init() { m_player_renderer.init(); }

void WorldRenderer::render(ClientWorld& world) {
    // update view matrix;
    view_matrix = world.world_scene().camera().get_camera_lookat();

    m_world_fbo->bind();
    // clear world framebuffer
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    day_night_calculation(world);

    render_sky(world);
    render_world(world);
    render_outline(world);
    render_player(world);

    FrameBuffer::unbind();

    glEnable(GL_FRAMEBUFFER_SRGB);

    glDisable(GL_DEPTH_TEST);
    // clear screen
    glClearColor(0.0f, 0.0f, 0.0f, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    render_underwater(world);
    glDisable(GL_FRAMEBUFFER_SRGB);
}

void WorldRenderer::day_night_calculation(ClientWorld& world) {

    m_parallel_light.sundir = glm::normalize(world.sunlight_dir());
    m_parallel_light.sun_height = (-m_parallel_light.sundir).y;
    m_parallel_light.lightdir = m_parallel_light.sundir;

    m_parallel_light.day_light =
        glm::smoothstep(0.15f, 0.3f, m_parallel_light.sun_height);

    m_parallel_light.sun_color = mix(SUNSET_SUNLIGHT_COLOR, NOON_SUNLIGHT_COLOR,
                                     m_parallel_light.day_light);

    glm::vec3 ambient_color = mix(SUNSET_AMBIENT_COLOR, NOON_AMBIENT_COLOR,
                                  m_parallel_light.day_light);

    m_parallel_light.day_factor =
        glm::smoothstep(-0.15f, 0.05f, m_parallel_light.sun_height);

    auto day_factor = m_parallel_light.day_factor;

    float light_intensity =
        glm::smoothstep(moon_intensity, sun_intensity, day_factor);

    m_parallel_light.directional_light_color =
        glm::mix(MOON_COLOR, m_parallel_light.sun_color, day_factor) *
        light_intensity;

    m_parallel_light.finnal_ambient_color =
        glm::mix(NIGHT_AMBIENT_COLOR, ambient_color, day_factor);

    m_ambient_strength = glm::mix(0.45f, 0.25f, day_factor);
}

void WorldRenderer::render_sky(ClientWorld& world) {

    auto& camera = world.world_scene().camera();

    glm::vec3 zenith = {0.20f, 0.45f, 0.95f};

    glm::vec3 horizon = {0.55f, 0.75f, 1.00f};

    glm::vec3 sunset_zenith = {0.05f, 0.10f, 0.25f};

    glm::vec3 sunset_horizon = {1.0f, 0.35f, 0.10f};

    glm::vec3 night_zenith = {0.018f, 0.023f, 0.048f};
    glm::vec3 night_horizon = {0.022f, 0.027f, 0.052f};

    constexpr float NIGHT_SHARPNESS = 0.35f;
    constexpr float SUNSET_SHARPNESS = 0.6f;
    constexpr float NOON_SHARPNESS = 0.35f;

    constexpr float NIGHT_CLOUD_MIX = 0.3f;
    constexpr float SUNSET_CLOUD_MIX = 0.4f;
    constexpr float NOON_CLOUD_MIX = 0.7;

    glm::vec3 day_top = mix(sunset_zenith, zenith, m_parallel_light.day_light);

    glm::vec3 day_bottom =
        mix(sunset_horizon, horizon, m_parallel_light.day_light);

    m_sky_uniform.sky_top =
        mix(night_zenith, day_top, m_parallel_light.day_factor);

    m_sky_uniform.sky_bottom =
        mix(night_horizon, day_bottom, m_parallel_light.day_factor);

    float day_sharpness =
        glm::mix(SUNSET_SHARPNESS, NOON_SHARPNESS, m_parallel_light.day_light);

    m_sky_uniform.horizon_sharpness =
        glm::mix(NIGHT_SHARPNESS, day_sharpness, m_parallel_light.day_factor);

    float day_cloud_mix =
        glm::mix(SUNSET_CLOUD_MIX, NOON_CLOUD_MIX, m_parallel_light.day_light);

    m_sky_uniform.cloud_white_mix =
        glm::mix(NIGHT_CLOUD_MIX, day_cloud_mix, m_parallel_light.day_factor);

    m_cloud_time += m_renderer.delta_time() * m_cloud_speed;

    const auto& sky_shader = m_renderer.get_shader("sky");

    sky_shader.use();

    glm::mat4 model_mat = glm::translate(
        glm::mat4(1.0f), camera.get_camera_pos() - glm::vec3(0.5f, 0.5f, 0.5f));

    glm::mat4 mv_mat = view_matrix * model_mat;

    auto& proj_mat = m_renderer.p_mat();

    m_sky_uniform.sun_dir_view = (-m_parallel_light.sundir);

    sky_shader.set_loc("mv_matrix", mv_mat);
    sky_shader.set_loc("proj_matrix", proj_mat);
    sky_shader.set_loc("skyTop", m_sky_uniform.sky_top);
    sky_shader.set_loc("skyBottom", m_sky_uniform.sky_bottom);
    sky_shader.set_loc("sunDir", m_sky_uniform.sun_dir_view);
    sky_shader.set_loc("sunColor", m_parallel_light.directional_light_color);
    sky_shader.set_loc("horizonSharpness", m_sky_uniform.horizon_sharpness);
    sky_shader.set_loc("time", m_cloud_time);
    sky_shader.set_loc("cloudWhiteMix", m_sky_uniform.cloud_white_mix);
    sky_shader.set_loc("cloudThresholdLow", m_cloud_threshold_low);
    sky_shader.set_loc("cloudThresholdHigh", m_cloud_threshold_high);

    auto& m_vao = m_renderer.vao();

    m_vao[1].bind();

    glDisable(GL_DEPTH_TEST);

    glDrawArrays(GL_TRIANGLES, 0, 36);
    glEnable(GL_DEPTH_TEST);

    // draw sun and moon
    const auto& billboard = m_renderer.get_shader("billboard");
    billboard.use();
    glDepthMask(GL_FALSE);

    m_vao[0].bind();
    auto billboard_drawer = [this, &billboard,
                             &proj_mat](const glm::vec3& pos, float size,
                                        const glm::vec3& color) {
        glm::vec3 view_pos = glm::vec3(view_matrix * glm::vec4(pos, 1.0f));
        glm::mat4 mv_mat =
            glm::translate(glm::mat4(1.0f), view_pos) *
            glm::scale(glm::mat4(1.0f), glm::vec3(size)) *
            glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -0.5f, 0.0f));

        billboard.set_loc("mv_matrix", mv_mat);
        billboard.set_loc("proj_matrix", proj_mat);
        billboard.set_loc("color", color);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    };
    // draw sun
    glm::vec3 sun_pos =
        camera.get_camera_pos() +
        normalize(-m_parallel_light.sundir) * (FAR_PLANE * 0.9f);
    billboard_drawer(sun_pos, SUN_SIZE, SUN_COLOR);

    // draw moon

    glm::vec3 moon_pos =
        camera.get_camera_pos() +
        normalize(m_parallel_light.sundir) * (FAR_PLANE * 0.9f);
    billboard_drawer(moon_pos, MOON_SIZE, MOON_COLOR);

    glDepthMask(GL_TRUE);
}

void WorldRenderer::render_world(ClientWorld& world) {

    // shader map

    auto height = m_renderer.frame_height();
    auto width = m_renderer.frame_width();

    glm::mat4 model_mat =
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));

    glm::mat4 mv_mat = view_matrix * model_mat;

    glm::mat4 norm_mat = glm::transpose(glm::inverse(mv_mat));

    if (m_shader_on) {
        shadow_map_generate(world);
    }

    m_world_fbo->bind();

    glCullFace(GL_BACK);
    glViewport(0, 0, width, height);

    render_normal_block(model_mat, mv_mat, norm_mat, world);

    // copy depth buffer
    m_world_fbo->bind(FrameBufferType::READ_FRAMEBUFFER);

    m_oit_fbo->bind(FrameBufferType::DRAW_FRAMEBUFFER);

    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
                      GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    m_oit_fbo->bind(FrameBufferType::DRAW_FRAMEBUFFER);

    // pass one accumulate
    m_oit_fbo->bind();

    glClearBufferfv(GL_COLOR, 0, glm::value_ptr(glm::vec4(0.0f)));
    float one = 1.0f;
    glClearBufferfv(GL_COLOR, 1, &one);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glEnable(GL_BLEND);
    glBlendFunci(0, GL_ONE, GL_ONE);

    glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
    render_transparent_block(mv_mat, norm_mat, world);
}

void WorldRenderer::render_outline(ClientWorld& world) {
    const auto& shader = m_renderer.get_shader("outline");
    shader.use();

    const auto& block_pos = world.get_look_block_pos();

    if (block_pos != std::nullopt) {

        glm::mat4 model_mat =
            glm::translate(glm::mat4(1.0f), glm::vec3(block_pos.value().pos));

        glm::mat4 m_mv_mat = view_matrix * model_mat;
        auto& proj_mat = m_renderer.p_mat();
        shader.set_loc("mv_matrix", m_mv_mat);
        shader.set_loc("proj_matrix", proj_mat);

        auto& m_vao = m_renderer.vao();
        m_vao[2].bind();

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glLineWidth(4.0f);
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    }
}

void WorldRenderer::shadow_map_generate(ClientWorld& world) {
    float texels_per_unit = 0.0f;
    const auto& lightdir = m_parallel_light.lightdir;

    auto m_delta_time = m_renderer.delta_time();
    auto& camera = world.world_scene().camera();
    // shader map
    glm::mat4& light_space_matrix = m_parallel_light.light_space_matrix;

    auto& m_render_snapshots = world.render_snapshots();
    auto& camera_pos = camera.get_camera_pos();

    const auto& depth_shader = m_renderer.get_shader("depth_shader");

    depth_shader.use();

    glm::vec3 cam_pos = camera.get_camera_pos();
    glm::vec3 cam_fwd = camera.get_camera_front();
    float half_extent = 128.0f;

    glm::vec3 center = cam_pos + cam_fwd * (half_extent * 0.5f);

    glm::vec3 raw_shadow_lightdir =
        quantize_sun_direction(lightdir, ANGLE_STEP_DEG);
    glm::vec3 shadow_lightdir =
        get_smoothed_shadow_lightdir(raw_shadow_lightdir, m_delta_time);
    glm::vec3 up = fabs(shadow_lightdir.y) > 0.99f ? glm::vec3(0, 0, 1)
                                                   : glm::vec3(0, 1, 0);

    glm::mat4 light_basis = glm::lookAt(glm::vec3(0.0f), shadow_lightdir, up);
    texels_per_unit = DEPTH_MAP_SIZE / (half_extent * 2.0f);
    glm::vec3 ls_center = glm::vec3(light_basis * glm::vec4(center, 1.0f));
    ls_center.x = std::round(ls_center.x * texels_per_unit) / texels_per_unit;
    ls_center.y = std::round(ls_center.y * texels_per_unit) / texels_per_unit;
    glm::vec3 snapped_center =
        glm::vec3(glm::inverse(light_basis) * glm::vec4(ls_center, 1.0f));

    float distance = half_extent * 1.5f;
    float near_plane = 1.0f;
    float far_plane = distance + half_extent * 2.0f;
    glm::vec3 light_pos = snapped_center - shadow_lightdir * distance;
    glm::mat4 light_view = glm::lookAt(light_pos, snapped_center, up);
    glm::mat4 light_projection =
        glm::ortho(-half_extent, half_extent, -half_extent, half_extent,
                   near_plane, far_plane);

    light_space_matrix = light_projection * light_view;
    depth_shader.set_loc("lightSpaceMatrix", light_space_matrix);
    depth_shader.set_loc("is_discard_tranparent", m_discard_tranparent);

    glViewport(0, 0, DEPTH_MAP_SIZE, DEPTH_MAP_SIZE);
    if (m_light_cull_face == 0) {
        glCullFace(GL_FRONT);
    } else if (m_light_cull_face == 1) {
        glCullFace(GL_BACK);
    } else {
        Logger::warn("Light Cull Face {} Over The Max Selection",
                     m_light_cull_face);
        glCullFace(GL_BACK);
    }

    m_depth_map_fbo->bind();
    glClear(GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    for (const auto& snapshot : m_render_snapshots) {
        if (!snapshot) {
            continue;
        }
        m_texture_manager.get_texture_array()->bind(1);
        glBindVertexArray(snapshot->normal_vao);

        glDrawArrays(GL_TRIANGLES, 0, snapshot->normal_vertices_count);
    }

    // cross_plane and discard

    for (const auto& snapshot : m_render_snapshots) {
        if (!snapshot) {
            continue;
        }
        glm::vec2 camera_pos_xz{camera_pos.x, camera_pos.z};
        if (snapshot->cross_vertices_count != 0) {
            glm::vec2 center_xz{snapshot->center.x, snapshot->center.z};
            float dist2d = glm::distance(camera_pos_xz, center_xz);
            if (dist2d <= CROSS_PLANE_DISTANCE * 16) {
                m_texture_manager.get_cross_plane_array()->bind(1);
                glBindVertexArray(snapshot->cross_vao);

                glDrawArrays(GL_TRIANGLES, 0, snapshot->cross_vertices_count);
            }
        }
        if (snapshot->normal_discard_vertices_count != 0) {

            m_texture_manager.get_texture_array()->bind(1);

            glBindVertexArray(snapshot->normal_discard_vao);

            glDrawArrays(GL_TRIANGLES, 0,
                         snapshot->normal_discard_vertices_count);
        }
    }
    // player
    auto& player_shadow = m_renderer.get_shader("player_depth");
    m_player_renderer.shadow_render(player_shadow, light_space_matrix, world);
}

void WorldRenderer::render_underwater(ClientWorld& world) {

    const auto& shader = m_renderer.get_shader("under_water");
    auto& camera = world.world_scene().camera();
    shader.use();

    auto& m_vao = m_renderer.vao();

    m_vao[0].bind();

    auto& proj_mat = m_renderer.p_mat();

    shader.set_loc("u_sceneTexture", 0);
    shader.set_loc("u_time", static_cast<float>(glfwGetTime()));
    shader.set_loc("u_underwater", camera.is_under_water());
    shader.set_loc("u_waterColor", glm::vec3(0.1f, 0.25f, 0.35f));
    shader.set_loc("u_fogDensity", m_underwater_fog_density);
    shader.set_loc("cameraPos", camera.get_camera_pos());
    shader.set_loc("sunDir", -m_parallel_light.sundir);
    shader.set_loc("waterDensity", m_water_density);
    shader.set_loc("InverseViewProjection",
                   glm::inverse(proj_mat * view_matrix));
    shader.set_loc("sunColor", m_parallel_light.sun_color);
    shader.set_loc("u_lightSpaceMatrix", m_parallel_light.light_space_matrix);

    m_screen_texture->bind(0);
    m_screen_depth_texture->bind(1);
    m_depth_map_texture->bind(2);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void WorldRenderer::render_normal_block(const glm::mat4& model_mat,
                                        const glm::mat4& mv_mat,
                                        const glm::mat4& norm_mat,
                                        ClientWorld& world) {

    // shader map
    glm::mat4& light_space_matrix = m_parallel_light.light_space_matrix;
    auto& camera = world.world_scene().camera();
    auto& m_render_snapshots = world.render_snapshots();
    auto& camera_pos = camera.get_camera_pos();

    const auto& lightdir = m_parallel_light.lightdir;

    const auto& normal_block_shader = m_renderer.get_shader("normal_block");

    normal_block_shader.use();

    glm::vec3 light_dir_view =
        glm::normalize(glm::mat3(view_matrix) * lightdir);
    auto& proj_mat = m_renderer.p_mat();
    auto m_pbr = m_renderer.pbr();

    normal_block_shader.set_loc("enablePBR", m_pbr);
    normal_block_shader.set_loc("model_matrix", model_mat);
    normal_block_shader.set_loc("mv_matrix", mv_mat);
    normal_block_shader.set_loc("proj_matrix", proj_mat);
    normal_block_shader.set_loc("norm_matrix", norm_mat);
    normal_block_shader.set_loc("lightSpaceMatrix", light_space_matrix);
    normal_block_shader.set_loc("ambientStrength", m_ambient_strength);
    normal_block_shader.set_loc("sunlightColor",
                                m_parallel_light.directional_light_color);
    normal_block_shader.set_loc("ambientColor",
                                m_parallel_light.finnal_ambient_color);
    normal_block_shader.set_loc("sunlightDir", light_dir_view);
    normal_block_shader.set_loc("shadowMode", m_shadow_mode);
    normal_block_shader.set_loc("shader_on", m_shader_on);
    normal_block_shader.set_loc("lightSizeUV",
                                static_cast<float>(m_light_size_uv));

    normal_block_shader.set_loc("minRadius", m_min_radius);
    normal_block_shader.set_loc("maxRadius", m_max_radius);
    normal_block_shader.set_loc("samples", m_samples);
    normal_block_shader.set_loc("specularStrength", m_specular_strength);
    normal_block_shader.set_loc("cameraPos", camera.get_camera_pos());
    normal_block_shader.set_loc("flipY", m_flip_y);
    normal_block_shader.set_loc("renderDistance", world.rendering_distance());
    normal_block_shader.set_loc("skyColor", m_sky_uniform.sky_top);

    glm::mat4 mvp_mat = proj_mat * mv_mat;

    auto& m_planes = world.planes();

    Math::extract_frustum_planes(mvp_mat, m_planes);

    int rendered_sum = 0;

    glEnable(GL_DEPTH_TEST);

    m_depth_map_texture->bind(0);

    m_texture_manager.get_texture_array()->bind(1);

    m_texture_manager.get_pbr_texture()->bind(2);
    // normal block
    for (const auto& snapshot : m_render_snapshots) {
        if (!snapshot) {
            continue;
        }
        if (Math::is_aabb_in_frustum(snapshot->center, snapshot->half_extents,
                                     m_planes)) {

            glBindVertexArray(snapshot->normal_vao);

            glDrawArrays(GL_TRIANGLES, 0, snapshot->normal_vertices_count);

            rendered_sum++;
        }
    }
    // discard
    for (const auto& snapshot : m_render_snapshots) {
        if (!snapshot) {
            continue;
        }
        if (!Math::is_aabb_in_frustum(snapshot->center, snapshot->half_extents,
                                      m_planes)) {
            continue;
        }
        if (snapshot->normal_discard_vertices_count != 0) {
            glBindVertexArray(snapshot->normal_discard_vao);

            glDrawArrays(GL_TRIANGLES, 0,
                         snapshot->normal_discard_vertices_count);
        }
    }
    // cross_plane
    m_texture_manager.get_cross_plane_array()->bind(1);
    normal_block_shader.set_loc("enablePBR", false);
    for (const auto& snapshot : m_render_snapshots) {
        if (!snapshot) {
            continue;
        }
        if (!Math::is_aabb_in_frustum(snapshot->center, snapshot->half_extents,
                                      m_planes)) {
            continue;
        }
        glm::vec2 camera_pos_xz{camera_pos.x, camera_pos.z};
        if (snapshot->cross_vertices_count != 0) {
            glm::vec2 center_xz{snapshot->center.x, snapshot->center.z};
            float dist2d = glm::distance(camera_pos_xz, center_xz);
            if (dist2d <= CROSS_PLANE_DISTANCE * 16) {
                glBindVertexArray(snapshot->cross_vao);

                glDrawArrays(GL_TRIANGLES, 0, snapshot->cross_vertices_count);
            }
        }
    }
    DebugCollector::get().report(
        "rendered_chunk", "Rendered Chunk: " + std::to_string(rendered_sum));
}

void WorldRenderer::render_transparent_block(const glm::mat4& mv_mat,
                                             const glm::mat4& norm_mat,
                                             ClientWorld& world) {

    auto& m_render_snapshots = world.render_snapshots();
    auto& camera = world.world_scene().camera();
    const auto& lightdir = m_parallel_light.lightdir;
    glm::vec3 light_dir_view =
        glm::normalize(glm::mat3(view_matrix) * lightdir);

    auto& m_p_mat = m_renderer.p_mat();

    auto set_accum_loc = [&](const Shader& accum_shader) {
        accum_shader.set_loc("mv_matrix", mv_mat);
        accum_shader.set_loc("proj_matrix", m_p_mat);
        accum_shader.set_loc("norm_matrix", norm_mat);
        accum_shader.set_loc("ambientStrength", m_ambient_strength);
        accum_shader.set_loc("sunlightColor",
                             m_parallel_light.directional_light_color);
        accum_shader.set_loc("ambientColor",
                             m_parallel_light.finnal_ambient_color);
        accum_shader.set_loc("sunlightDir", light_dir_view);
        accum_shader.set_loc("shader_on", m_shader_on);
        accum_shader.set_loc("specularStrength", m_specular_strength);
    };
    // accum pass
    auto& accum_shader = m_renderer.get_shader("accum");
    accum_shader.use();

    set_accum_loc(accum_shader);
    accum_shader.set_loc("cameraPos", camera.get_camera_pos());

    m_texture_manager.get_texture_array()->bind(0);

    auto& m_planes = world.planes();

    for (const auto& snapshot : m_render_snapshots) {
        if (!snapshot) {
            continue;
        }
        if (!Math::is_aabb_in_frustum(snapshot->center, snapshot->half_extents,
                                      m_planes)) {
            continue;
        }

        if (snapshot->normal_blend_vertices_count != 0) {

            glBindVertexArray(snapshot->normal_blend_vao);

            glDrawArrays(GL_TRIANGLES, 0,
                         snapshot->normal_blend_vertices_count);
        }
    }

    // use SSR

    auto& water_shader = m_renderer.get_shader("water");
    water_shader.use();

    set_accum_loc(water_shader);

    water_shader.set_loc("sceneColorTex", 1);
    water_shader.set_loc("sceneDepthTex", 2);
    water_shader.set_loc("inv_proj_matrix", glm::inverse(m_p_mat));
    water_shader.set_loc("inv_view_matrix", glm::inverse(view_matrix));

    // sky loc
    water_shader.set_loc("skyTop", m_sky_uniform.sky_top);
    water_shader.set_loc("skyBottom", m_sky_uniform.sky_bottom);
    water_shader.set_loc("sunDir", m_sky_uniform.sun_dir_view);
    water_shader.set_loc("sunColor", m_parallel_light.directional_light_color);
    water_shader.set_loc("horizonSharpness", m_sky_uniform.horizon_sharpness);
    water_shader.set_loc("time", glfwGetTime());
    water_shader.set_loc("cloudWhiteMix", m_sky_uniform.cloud_white_mix);
    water_shader.set_loc("cloudThresholdLow", m_cloud_threshold_low);
    water_shader.set_loc("cloudThresholdHigh", m_cloud_threshold_high);
    water_shader.set_loc("underwater", camera.is_under_water());
    water_shader.set_loc("refractStrength", m_refract_strength);
    water_shader.set_loc("enablePerturb", m_water_perturb);
    water_shader.set_loc("enableDepthFade", m_water_depth_fade);

    m_screen_texture->bind(1);
    m_screen_depth_texture->bind(2);
    m_texture_manager.get_texture_array()->bind(0);

    for (const auto& snapshot : m_render_snapshots) {
        if (!snapshot) {
            continue;
        }
        if (!Math::is_aabb_in_frustum(snapshot->center, snapshot->half_extents,
                                      m_planes)) {
            continue;
        }

        if (snapshot->water_vertices_count != 0) {

            glBindVertexArray(snapshot->water_vao);

            glDrawArrays(GL_TRIANGLES, 0, snapshot->water_vertices_count);
        }
    }
    // composite pass

    auto& composite_shader = m_renderer.get_shader("composite");
    glDisable(GL_BLEND);

    composite_shader.use();
    composite_shader.set_loc("u_accumTex", 0);
    composite_shader.set_loc("u_revealTex", 1);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    auto& m_vao = m_renderer.vao();
    m_vao[0].bind();

    m_accum_texture->bind(0);
    m_reveal_texture->bind(1);

    m_world_fbo->bind();

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void WorldRenderer::render_player(ClientWorld& world) {
    auto& shader = m_renderer.get_shader("player");
    shader.use();
    glm::vec3 light_dir_view =
        glm::normalize(glm::mat3(view_matrix) * m_parallel_light.lightdir);

    shader.set_loc("lightSpaceMatrix", m_parallel_light.light_space_matrix);
    shader.set_loc("ambientStrength", m_ambient_strength);
    shader.set_loc("sunlightColor", m_parallel_light.directional_light_color);
    shader.set_loc("ambientColor", m_parallel_light.finnal_ambient_color);
    shader.set_loc("sunlightDir", light_dir_view);
    shader.set_loc("shadowMode", m_shadow_mode);
    shader.set_loc("shader_on", m_shader_on);
    shader.set_loc("lightSizeUV", static_cast<float>(m_light_size_uv));
    shader.set_loc("minRadius", m_min_radius);
    shader.set_loc("maxRadius", m_max_radius);
    shader.set_loc("samples", m_samples);

    // shader.set_loc("renderDistance", m_world.rendering_distance());
    // shader.set_loc("skyColor", m_sky_uniform.sky_top);

    m_depth_map_texture->bind(0);
    m_player_renderer.render(shader, world);
}

glm::vec3 WorldRenderer::quantize_sun_direction(const glm::vec3& lightdir,
                                                float angle_step_deg) const {
    float elevation = std::asin(glm::clamp(lightdir.y, -1.0f, 1.0f));
    float azimuth = std::atan2(lightdir.z, lightdir.x);

    float step = glm::radians(angle_step_deg);

    float quantized_elevation = std::round(elevation / step) * step;
    float quantized_azimuth = std::round(azimuth / step) * step;

    glm::vec3 quantized_dir;
    quantized_dir.x =
        std::cos(quantized_elevation) * std::cos(quantized_azimuth);
    quantized_dir.z =
        std::cos(quantized_elevation) * std::sin(quantized_azimuth);
    quantized_dir.y = std::sin(quantized_elevation);

    return glm::normalize(quantized_dir);
}

glm::vec3 WorldRenderer::get_smoothed_shadow_lightdir(
    const glm::vec3& raw_shadow_lightdir, float dt) {
    if (!m_blend_initialized) {

        m_blend_from_lightdir = raw_shadow_lightdir;
        m_blend_to_lightdir = raw_shadow_lightdir;
        m_blend_t = 1.0f;
        m_blend_initialized = true;
        return raw_shadow_lightdir;
    }

    if (raw_shadow_lightdir != m_blend_to_lightdir) {
        glm::vec3 current_displayed = glm::normalize(
            Math::slerp(m_blend_from_lightdir, m_blend_to_lightdir, m_blend_t));

        m_blend_from_lightdir = current_displayed;
        m_blend_to_lightdir = raw_shadow_lightdir;
        m_blend_t = 0.0f;
    }

    m_blend_t = glm::min(m_blend_t + dt / BLEND_DURATION, 1.0f);

    return glm::normalize(
        Math::slerp(m_blend_from_lightdir, m_blend_to_lightdir, m_blend_t));
}

void WorldRenderer::updata_framebuffer(int width, int height) {
    if (width <= 0 || height <= 0)
        return;
    if (m_world_fbo == 0) {
        m_world_fbo = std::make_unique<FrameBuffer>();
    }
    if (m_oit_fbo == 0) {
        m_oit_fbo = std::make_unique<FrameBuffer>();
    }

    m_screen_texture = std::make_unique<Texture>(TextureType::TEXTURE_2D);
    m_screen_texture->tex_image_2d(TextureFormat::RGB, TextureFormat::RGB,
                                   GL_UNSIGNED_BYTE, nullptr, width, height);

    m_screen_texture->set_linear();

    m_world_fbo->attach(Attachment::COLOR_ATTACHMENT0, *m_screen_texture);

    m_screen_depth_texture = std::make_unique<Texture>(TextureType::TEXTURE_2D);
    m_screen_depth_texture->tex_image_2d(TextureFormat::DEPTH_COMPONENT32F,
                                         TextureFormat::DEPTH_COMPONENT,
                                         GL_FLOAT, nullptr, width, height);
    // m_screen_depth_texture->set_nearest();
    m_screen_depth_texture->set_linear();
    m_world_fbo->attach(Attachment::DEPTH_ATTACHMENT, *m_screen_depth_texture);

    m_world_fbo->check_status();
    m_accum_texture = std::make_unique<Texture>(TextureType::TEXTURE_2D);
    m_accum_texture->tex_image_2d(TextureFormat::RGBA16F, TextureFormat::RGBA,
                                  GL_HALF_FLOAT, nullptr, width, height);
    m_accum_texture->set_linear();

    m_oit_fbo->attach(Attachment::COLOR_ATTACHMENT0, *m_accum_texture);
    m_reveal_texture = std::make_unique<Texture>(TextureType::TEXTURE_2D);
    m_reveal_texture->tex_image_2d(TextureFormat::R16F, TextureFormat::RED,
                                   GL_HALF_FLOAT, nullptr, width, height);
    m_reveal_texture->set_linear();
    m_oit_fbo->attach(Attachment::COLOR_ATTACHMENT1, *m_reveal_texture);
    m_oit_depth_texture = std::make_unique<Texture>(TextureType::TEXTURE_2D);
    m_oit_depth_texture->tex_image_2d(TextureFormat::DEPTH_COMPONENT32F,
                                      TextureFormat::DEPTH_COMPONENT, GL_FLOAT,
                                      nullptr, width, height);
    // m_oit_depth_texture->set_nearest();
    m_oit_depth_texture->set_linear();
    m_oit_fbo->attach(Attachment::DEPTH_ATTACHMENT, *m_oit_depth_texture);

    std::array<GLenum, 2> draw_buffer = {GL_COLOR_ATTACHMENT0,
                                         GL_COLOR_ATTACHMENT1};
    m_oit_fbo->draw_buffer(draw_buffer);

    m_oit_fbo->check_status();

    // depth map fbo
    if (m_depth_map_fbo == 0) {
        m_depth_map_fbo = std::make_unique<FrameBuffer>();
    }

    m_depth_map_texture = std::make_unique<Texture>(TextureType::TEXTURE_2D);

    m_depth_map_texture->tex_image_2d(TextureFormat::DEPTH_COMPONENT32F,
                                      TextureFormat::DEPTH_COMPONENT, GL_FLOAT,
                                      nullptr, DEPTH_MAP_SIZE, DEPTH_MAP_SIZE);
    m_depth_map_texture->set_linear();
    m_depth_map_texture->set_clamp_to_border(false, true, true);
    float border_color[] = {1.0f, 1.0f, 1.0f, 1.0f};
    // Manually compare shadows
    m_depth_map_texture->parameterfv(TexturePname::BORDER_COLOR, border_color);
    m_depth_map_texture->parameter(TexturePname::COMPARE_MODE,
                                   TextureParam::T_NONE);

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
    //                 GL_COMPARE_REF_TO_TEXTURE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    m_depth_map_fbo->attach(Attachment::DEPTH_ATTACHMENT, *m_depth_map_texture);
    m_depth_map_fbo->draw_buffer(GL_NONE);
    m_depth_map_fbo->read_buffer(GL_NONE);
    m_depth_map_fbo->check_status();

    FrameBuffer::unbind();
}

float& WorldRenderer::underwater_fog_density() {
    return m_underwater_fog_density;
}
float& WorldRenderer::water_density() { return m_water_density; }
const FrameBuffer* WorldRenderer::world_fbo() const {
    return m_world_fbo.get();
}
float& WorldRenderer::ambient_strength() { return m_ambient_strength; }
bool& WorldRenderer::discard_transparent() { return m_discard_tranparent; }
bool& WorldRenderer::shader_on() { return m_shader_on; }
bool& WorldRenderer::water_perturb() { return m_water_perturb; }
bool& WorldRenderer::water_depth_fade() { return m_water_depth_fade; }
bool& WorldRenderer::pbr() { return m_pbr; }
bool& WorldRenderer::flip_y() { return m_flip_y; }
int& WorldRenderer::shadow_mode() { return m_shadow_mode; }
int& WorldRenderer::light_cull_face() { return m_light_cull_face; }
int& WorldRenderer::light_size_uv() { return m_light_size_uv; }
float& WorldRenderer::min_radius() { return m_min_radius; }
float& WorldRenderer::max_radius() { return m_max_radius; }
int& WorldRenderer::samples() { return m_samples; }
float& WorldRenderer::specular_strength() { return m_specular_strength; }
float& WorldRenderer::cloud_speed() { return m_cloud_speed; }
float& WorldRenderer::cloud_threshold_low() { return m_cloud_threshold_low; }
float& WorldRenderer::cloud_threshold_high() { return m_cloud_threshold_high; }
float& WorldRenderer::refract_strength() { return m_refract_strength; }

} // namespace Cubed