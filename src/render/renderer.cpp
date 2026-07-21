#include "Cubed/render/renderer.hpp"

#include "Cubed/config.hpp"
#include "Cubed/debug_collector.hpp"
#include "Cubed/dev_panel.hpp"
#include "Cubed/primitive_data.hpp"
#include "Cubed/render/renderer_constants.hpp"
#include "Cubed/texture_manager.hpp"
#include "Cubed/tools/font.hpp"
#include "Cubed/tools/log.hpp"
#include "Cubed/tools/shader_tools.hpp"

#include <format>
#include <glm/gtc/type_ptr.hpp>

namespace Cubed {

Renderer::Renderer(TextureManager& texture_manager, Config& config)
    : m_texture_manager(texture_manager), m_world_renderer(*this),
      m_config(config) {}

Renderer::~Renderer() {
    if (m_init) {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        m_outline_vbo.reset();
        m_outline_indices_vbo.reset();
        m_sky_vbo.reset();
        m_ui_vbo.reset();
        m_player_vbo.reset();
        glBindVertexArray(0);
        m_vao.clear();
    }
}

void Renderer::reload_config() {
    update_fov(m_config.get("player.fov", 70.0f));
}

void Renderer::init() {

    Logger::info("OpenGL Version: {}.{}", GLVersion.major, GLVersion.minor);
    Logger::info("Renderer: {}",
                 reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

    m_shaders.init();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifdef DEBUG_MODE

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(
        [](GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar* message,
           const void*) {
            Logger::debug("GL Debug: {}",
                          reinterpret_cast<const char*>(message));
        },
        nullptr);

#endif

    m_vao.resize(NUM_VAO);
    VertexArray::unbind();

    m_outline_vbo = std::make_unique<VertexBuffer>();
    m_outline_indices_vbo =
        std::make_unique<VertexBuffer>(BufferType::ELEMENT_ARRAY_BUFFER);
    m_player_vbo = std::make_unique<VertexBuffer>();
    m_quad_vbo = std::make_unique<VertexBuffer>();
    m_sky_vbo = std::make_unique<VertexBuffer>();
    m_ui_vbo = std::make_unique<VertexBuffer>();

    m_vao[2].bind();

    m_outline_vbo->buffer_data(CUBE_VER, sizeof(CUBE_VER));
    m_vao[2].attribute(0, 3, GL_FLOAT, 0, 0);
    m_outline_indices_vbo->buffer_data(OUTLINE_CUBE_INDICES,
                                       sizeof(OUTLINE_CUBE_INDICES));

    m_vao[1].bind();
    m_sky_vbo->buffer_data(VERTICES_POS, sizeof(VERTICES_POS));

    m_vao[1].attribute(0, 3, GL_FLOAT, 0, 0);

    m_vao[3].bind();

    for (int i = 0; i < 6; i++) {
        Vertex2D vex{SQUARE_VERTICES_TOP_LEFT[i][0],
                     SQUARE_VERTICES_TOP_LEFT[i][1], SQUARE_TEXTURE_POS[i][0],
                     SQUARE_TEXTURE_POS[i][1], 0};
        m_ui.emplace_back(vex);
    }
    m_ui_vbo->buffer_data(m_ui.data(), m_ui.size() * sizeof(Vertex2D));

    m_vao[3].attribute(0, 3, GL_FLOAT, sizeof(Vertex2D), (void*)0);
    m_vao[3].attribute(1, 2, GL_FLOAT, sizeof(Vertex2D),
                       (void*)offsetof(Vertex2D, s));
    m_vao[3].attribute(2, 1, GL_FLOAT, sizeof(Vertex2D),
                       (void*)offsetof(Vertex2D, layer));

    init_quad();
    init_text();
    reload_config();

    m_world_renderer.init();

    VertexArray::unbind();
    VertexBuffer::unbind();

    m_init = true;
}

const Shader& Renderer::get_shader(const std::string& name) const {
    return m_shaders.get_shader(name);
}

void Renderer::init_quad() {
    m_vao[0].bind();
    m_quad_vbo->buffer_data(QUAD_VERTICES, sizeof(QUAD_VERTICES));

    m_vao[0].attribute(0, 2, GL_FLOAT, 4 * sizeof(float), (void*)0);

    m_vao[0].attribute(1, 2, GL_FLOAT, 4 * sizeof(float),
                       (void*)(2 * sizeof(float)));
}

void Renderer::init_text() {
    m_vao[4].bind();

    DebugCollector::get().init(m_window_width, m_window_height);
}
void Renderer::begin_frame() {
    glDisable(GL_FRAMEBUFFER_SRGB);
    // clear screen
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void Renderer::end_frame() {}

void Renderer::begin_render_ui() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_DEPTH_TEST);
}
void Renderer::end_render_ui() { glEnable(GL_DEPTH_TEST); }

void Renderer::render_world(ClientWorld& world) {
    m_world_renderer.render(world);
}

void Renderer::render_rect(const Rect& rect) {

    auto& shader = get_shader("rect");
    shader.use();
    shader.set_loc("proj_matrix", m_ui_proj_matrix);
    auto pos = rect.pos();
    glm::mat4 model_matrix =
        glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, 0.0f)) *
        glm::scale(glm::mat4(1.0f),
                   glm::vec3(rect.width(), rect.height(), 1.0f));
    shader.set_loc("model_matrix", model_matrix);

    auto color = color_value(rect.color());
    shader.set_loc("inColor",
                   glm::vec4(color.x, color.y, color.z, rect.alpha()));

    m_vao[3].bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::render_lable(const Label& label) {

    const auto& shader = get_shader("text");

    shader.use();

    shader.set_loc("projection", m_ui_proj_matrix);

    Font::get().text_texture()->bind(0);
    auto& data = label.data();
    if (!data.m_vao) {
        return;
    }
    auto pos = label.pos();
    auto& text_style = label.text_style();
    auto color = color_value(text_style.color);
    data.m_vao->bind();
    glm::mat4 model_matrix =
        glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, 0.0f)) *
        glm::scale(glm::mat4(1.0f),
                   glm::vec3(label.scale(), label.scale(), 1.0f));

    shader.set_loc("textColor", glm::vec3(color.x, color.y, color.z));
    shader.set_loc("mv_matrix", model_matrix);

    glDrawArrays(GL_TRIANGLES, 0, data.m_sum);
}

void Renderer::render_image(const Image& image) {
    if (!image.texture()) {
        Logger::error("Image not set image!");
        return;
    }
    const auto& shader = get_shader("image");
    shader.use();
    auto pos = image.pos();
    glm::mat4 model_matrix =
        glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, 0.0f)) *
        glm::scale(glm::mat4(1.0f),
                   glm::vec3(image.width(), image.height(), 1.0f));
    shader.set_loc("model_matrix", model_matrix);
    shader.set_loc("proj_matrix", m_ui_proj_matrix);

    m_vao[3].bind();

    image.texture()->bind(0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    Tools::check_opengl_error();
}

void Renderer::update(float delta_time) { m_delta_time = delta_time; }

void Renderer::update_fov(float fov) {
    m_fov = fov;

    m_world_proj_matrix =
        glm::perspective(glm::radians(fov), m_aspect, NEAR_PLANE, FAR_PLANE);
}

void Renderer::updata_framebuffer(int width, int height) {
    if (width <= 0 || height <= 0)
        return;

    m_world_renderer.updata_framebuffer(width, height);

    FrameBuffer::unbind();
}

void Renderer::render_dev_panel(DevPanel& dev_panel) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    dev_panel.render();
    glEnable(GL_DEPTH_TEST);
}

bool Renderer::handle_event(const Event& e) {
    return std::visit(Overloaded{[](const MouseMoveEvent&) { return false; },
                                 [](const MouseButtonEvent&) { return false; },
                                 [](const MouseWheelEvent&) { return false; },
                                 [](const KeyEvent&) { return false; },
                                 [](const TextInputEvent&) { return false; },
                                 [this](const WindowResizeEvent& e) {
                                     handle_window_resize_event(e);
                                     return false;
                                 },
                                 [this](const FrameBufferResizeEvent& e) {
                                     handle_frame_buffer_resize_event(e);
                                     return false;
                                 }

                      },
                      e);
}
bool Renderer::handle_window_resize_event(const WindowResizeEvent& e) {
    m_window_width = static_cast<float>(e.width);
    m_window_height = static_cast<float>(e.height);
    m_ui_proj_matrix =
        glm::ortho(0.0f, m_window_width, m_window_height, 0.0f, -1.0f, 1.0f);
    return false;
}
bool Renderer::handle_frame_buffer_resize_event(
    const FrameBufferResizeEvent& e) {
    int frame_height = e.height;
    int frame_width = e.width;
    m_frame_width = static_cast<float>(e.width);
    m_frame_height = static_cast<float>(e.height);
    m_aspect = m_frame_width / m_frame_height;
    glViewport(0, 0, frame_width, frame_height);
    m_world_proj_matrix =
        glm::perspective(glm::radians(m_fov), m_aspect, NEAR_PLANE, FAR_PLANE);
    updata_framebuffer(frame_width, frame_height);
    return false;
}

float& Renderer::ambient_strength() {
    return m_world_renderer.ambient_strength();
}
bool& Renderer::discard_transparent() {
    return m_world_renderer.discard_transparent();
}
bool& Renderer::shader_on() { return m_world_renderer.shader_on(); }
bool& Renderer::water_perturb() { return m_world_renderer.water_perturb(); }
bool& Renderer::water_depth_fade() {
    return m_world_renderer.water_depth_fade();
}
bool& Renderer::pbr() { return m_world_renderer.pbr(); }
bool& Renderer::flip_y() { return m_world_renderer.flip_y(); }
int& Renderer::shadow_mode() { return m_world_renderer.shadow_mode(); }
int& Renderer::light_cull_face() { return m_world_renderer.light_cull_face(); }
int& Renderer::light_size_uv() { return m_world_renderer.light_size_uv(); }
float& Renderer::min_radius() { return m_world_renderer.min_radius(); }
float& Renderer::max_radius() { return m_world_renderer.max_radius(); }
int& Renderer::samples() { return m_world_renderer.samples(); }
float& Renderer::specular_strength() {
    return m_world_renderer.specular_strength();
}
float& Renderer::cloud_speed() { return m_world_renderer.cloud_speed(); }
float& Renderer::cloud_threshold_low() {
    return m_world_renderer.cloud_threshold_low();
}
float& Renderer::cloud_threshold_high() {
    return m_world_renderer.cloud_threshold_high();
}
float& Renderer::refract_strength() {
    return m_world_renderer.refract_strength();
}
float& Renderer::underwater_fog_density() {
    return m_world_renderer.underwater_fog_density();
}
float& Renderer::water_density() { return m_world_renderer.water_density(); }

const glm::mat4& Renderer::world_proj_matrix() const {
    return m_world_proj_matrix;
}
const TextureManager& Renderer::texture_mamger() const {
    return m_texture_manager;
}

float Renderer::delta_time() const { return m_delta_time; }

float Renderer::window_height() const { return m_window_height; }
float Renderer::window_width() const { return m_window_width; }
float Renderer::frame_height() const { return m_frame_height; }
float Renderer::frame_width() const { return m_frame_width; }
const glm::mat4& Renderer::p_mat() const { return m_world_proj_matrix; }
const std::vector<VertexArray>& Renderer::vao() const { return m_vao; }
} // namespace Cubed