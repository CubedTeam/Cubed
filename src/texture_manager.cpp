#include "Cubed/texture_manager.hpp"

#include "Cubed/config.hpp"
#include "Cubed/constants.hpp"
#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/log.hpp"
#include "Cubed/tools/shader_tools.hpp"

namespace {
constexpr int BLOCK_SIZE = 16;
constexpr int BLOCK_NORMAL_SIZE = 128;
constexpr int CROSS_PLANE_SIZE = 16;
constexpr int BLOCK_ITEM_SIZE = 16;
constexpr int UI_SIZE = 16;
constexpr int BLOCK_STATUS_SIZE = 16;
constexpr int SKIN_SIZE = 64;

unsigned char* generate_flat_normal_map(int width = BLOCK_NORMAL_SIZE,
                                        int height = BLOCK_NORMAL_SIZE) {
    unsigned char* data = new unsigned char[width * height * 4];
    for (int i = 0; i < width * height; i++) {
        data[i * 4 + 0] = 128; // R -> X = 0
        data[i * 4 + 1] = 128; // G -> Y = 0
        data[i * 4 + 2] = 255; // B -> Z = 1
        data[i * 4 + 3] = 255; // A
    }
    return data;
}

} // namespace

namespace Cubed {

TextureManager::TextureManager(Config& config) : m_config(config) {}

TextureManager::~TextureManager() { delete_texture(); }

void TextureManager::delete_texture() {
    if (m_init) {
        m_texture_array.reset();
        m_block_status_array.reset();
        m_cross_plane_array.reset();
        m_normal_texture_array.reset();
        m_item_textures.clear();
        m_skin.reset();
        Logger::info("Successfully delete all texture");
    }
}

const Texture* TextureManager::get_block_status_array() const {
    return m_block_status_array.get();
}

const Texture* TextureManager::get_texture_array() const {
    return m_texture_array.get();
}

const Texture* TextureManager::get_cross_plane_array() const {
    return m_cross_plane_array.get();
}
const Texture* TextureManager::get_ui_array() const { return m_ui_array.get(); }

const Texture* TextureManager::get_pbr_texture() const {
    return m_normal_texture_array.get();
}

const std::vector<std::unique_ptr<Texture>>&
TextureManager::item_textures() const {
    return m_item_textures;
}

const Texture* TextureManager::get_skin() const { return m_skin.get(); }

void TextureManager::load_block_status(unsigned id) {

    ASSERT_MSG(id < MAX_BLOCK_STATUS, "Exceed the max status sum limit");

    std::string path = "texture/status/" + std::to_string(id) + ".png";

    auto image_data = (Tools::load_image_data(path));

    m_block_status_array->tex_sub_image_3d(
        TextureFormat::RGBA, GL_UNSIGNED_BYTE, image_data.data, 0, 0, id,
        BLOCK_STATUS_SIZE, BLOCK_STATUS_SIZE);
}

void TextureManager::load_block_texture(unsigned id) {
    ASSERT_MSG(id < BlockManager::sums(), "Exceed the max block sum limit");
    std::string name{BlockManager::name_form_id(id)};
    // air don`t need texture
    if (id == 0) {
        return;
    }

    if (BlockManager::is_cross_plane(id)) {
        load_cross_plane_texture(id);
        return;
    }

    std::array<ImageData, 6> image_data;

    std::string block_texture_path = "texture/block/" + name;
    image_data[0] = (Tools::load_image_data(block_texture_path + "/front.png"));
    image_data[1] = (Tools::load_image_data(block_texture_path + "/right.png"));
    image_data[2] = (Tools::load_image_data(block_texture_path + "/back.png"));
    image_data[3] = (Tools::load_image_data(block_texture_path + "/left.png"));
    image_data[4] = (Tools::load_image_data(block_texture_path + "/top.png"));
    image_data[5] = (Tools::load_image_data(block_texture_path + "/base.png"));

    Tools::check_opengl_error();
    for (int i = 0; i < 6; i++) {
        m_texture_array->tex_sub_image_3d(TextureFormat::RGBA, GL_UNSIGNED_BYTE,
                                          image_data[i].data, 0, 0, id * 6 + i,
                                          BLOCK_SIZE, BLOCK_SIZE);
        Tools::check_opengl_error();
    }
}

void TextureManager::load_block_item_texture(unsigned id) {

    ASSERT_MSG(id < BlockManager::sums(), "Exceed the max block sum limit");
    std::string name{BlockManager::name_form_id(id)};

    std::string path = "texture/item/block/" + name + ".png";

    auto data = Tools::load_image_data(path);
    std::unique_ptr<Texture> texture =
        std::make_unique<Texture>(TextureType::TEXTURE_2D);
    texture->tex_image_2d(TextureFormat::RGBA8, TextureFormat::RGBA,
                          GL_UNSIGNED_BYTE, data.data, BLOCK_ITEM_SIZE,
                          BLOCK_ITEM_SIZE);

    texture->set_nearest();
    texture->set_clamp_to_border();

    m_item_textures.push_back(std::move(texture));
}

void TextureManager::load_cross_plane_texture(unsigned id) {
    std::string path =
        "texture/block/" + BlockManager::name_form_id(id) + "/cross.png";
    auto image_data = Tools::load_image_data(path);
    m_cross_plane_array->tex_sub_image_3d(TextureFormat::RGBA, GL_UNSIGNED_BYTE,
                                          image_data.data, 0, 0,
                                          BlockManager::cross_plane_index(id),
                                          CROSS_PLANE_SIZE, CROSS_PLANE_SIZE);
}

void TextureManager::load_ui_texture(unsigned id) {
    ASSERT_MSG(id < MAX_UI_NUM, "Exceed the max ui sum limit");

    std::string path = "texture/ui/" + std::to_string(id) + ".png";
    auto image_data = (Tools::load_image_data(path));
    m_ui_array->tex_sub_image_3d(TextureFormat::RGBA, GL_UNSIGNED_BYTE,
                                 image_data.data, 0, 0, id, UI_SIZE, UI_SIZE);
}

void TextureManager::load_pbr_texture(unsigned id) {

    if (id == 0) {
        return;
    }

    if (BlockManager::is_cross_plane(id)) {

        return;
    }

    std::string path = "normal/block/" + BlockManager::name_form_id(id);

    std::array<ImageData, 6> image_data;

    image_data[0] = (Tools::load_image_data(path + "/front_n.png", false));
    image_data[1] = (Tools::load_image_data(path + "/right_n.png", false));
    image_data[2] = (Tools::load_image_data(path + "/back_n.png", false));
    image_data[3] = (Tools::load_image_data(path + "/left_n.png", false));
    image_data[4] = (Tools::load_image_data(path + "/top_n.png", false));
    image_data[5] = (Tools::load_image_data(path + "/base_n.png", false));

    for (int i = 0; i < 6; i++) {
        unsigned char* data = image_data[i].data;
        bool is_fallback = false;
        if (!data) {
            is_fallback = true;
            data = generate_flat_normal_map();
        }
        m_normal_texture_array->tex_sub_image_3d(
            TextureFormat::RGBA, GL_UNSIGNED_BYTE, data, 0, 0, id * 6 + i,
            BLOCK_NORMAL_SIZE, BLOCK_NORMAL_SIZE);

        if (is_fallback) {
            delete[] data;
        } else {
        }
    }
}

void TextureManager::init_block() {
    m_texture_array = std::make_unique<Texture>(TextureType::TEXTURE_2D_ARRAY);
    m_texture_array->tex_image_3d(TextureFormat::RGBA, TextureFormat::RGBA,
                                  GL_UNSIGNED_BYTE, nullptr, BLOCK_SIZE,
                                  BLOCK_SIZE, BlockManager::sums() * 6);

    m_cross_plane_array =
        std::make_unique<Texture>(TextureType::TEXTURE_2D_ARRAY);
    m_cross_plane_array->tex_image_3d(
        TextureFormat::RGBA, TextureFormat::RGBA, GL_UNSIGNED_BYTE, nullptr,
        CROSS_PLANE_SIZE, CROSS_PLANE_SIZE, BlockManager::cross_plane_sum());

    m_normal_texture_array =
        std::make_unique<Texture>(TextureType::TEXTURE_2D_ARRAY);
    m_normal_texture_array->tex_image_3d(
        TextureFormat::RGBA8, TextureFormat::RGBA, GL_UNSIGNED_BYTE, nullptr,
        BLOCK_NORMAL_SIZE, BLOCK_NORMAL_SIZE, BlockManager::sums() * 6);
    for (unsigned i = 0; i < BlockManager::sums(); i++) {
        load_block_texture(i);
        load_block_item_texture(i);
        load_pbr_texture(i);
    }

    m_texture_array->set_nearest_and_minpmap();
    m_texture_array->set_repeat(false, true, true);
    m_texture_array->set_aniso(m_aniso);

    m_cross_plane_array->set_nearest_and_minpmap();
    m_texture_array->set_repeat(false, true, true);
    m_cross_plane_array->set_clamp_to_edge(m_aniso);

    m_normal_texture_array->set_nearest_and_minpmap();
    m_normal_texture_array->set_repeat(false, true, true);
    m_normal_texture_array->set_aniso(m_aniso);

    Logger::info("Block Texture Load Success");
}
void TextureManager::init_ui() {
    m_ui_array = std::make_unique<Texture>(TextureType::TEXTURE_2D_ARRAY);
    m_ui_array->tex_image_3d(TextureFormat::RGBA, TextureFormat::RGBA,
                             GL_UNSIGNED_BYTE, nullptr, UI_SIZE, UI_SIZE,
                             MAX_UI_NUM);
    for (int i = 0; i < MAX_UI_NUM; i++) {
        load_ui_texture(i);
    }

    m_ui_array->set_nearest();
}

void TextureManager::init_skin() {
    m_skin = std::make_unique<Texture>(TextureType::TEXTURE_2D);
    std::string path = "texture/skin/player001.png";

    auto image_data = (Tools::load_image_data(path));
    m_skin->tex_image_2d(TextureFormat::RGBA, TextureFormat::RGBA,
                         GL_UNSIGNED_BYTE, image_data.data, SKIN_SIZE,
                         SKIN_SIZE);
    m_skin->set_nearest_and_minpmap();
    m_skin->set_aniso(m_aniso);
}

void TextureManager::init_block_status() {
    m_block_status_array =
        std::make_unique<Texture>(TextureType::TEXTURE_2D_ARRAY);
    m_block_status_array->tex_image_3d(
        TextureFormat::RGBA, TextureFormat::RGBA, GL_UNSIGNED_BYTE, nullptr,
        BLOCK_STATUS_SIZE, BLOCK_STATUS_SIZE, MAX_BLOCK_STATUS);
    for (int i = 0; i < MAX_BLOCK_STATUS; i++) {
        load_block_status(i);
    }

    m_block_status_array->set_nearest_and_minpmap();
    m_block_status_array->set_aniso(m_aniso);
}
void TextureManager::init_texture() {
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &m_max_aniso);
    if (m_max_aniso > 0.0f) {
        Logger::info("Support anisotropic filtering max_aniso is {}",
                     m_max_aniso);
    }
    m_aniso = m_config.get("texture.aniso", 1);
    m_aniso = std::min(static_cast<int>(m_max_aniso), m_aniso);
    Logger::info("Setting Texture Aniso is {}", m_aniso);
    Logger::info("Map Init Success");

    init_block();
    init_block_status();
    init_ui();
    init_skin();
    m_init = true;
}

void TextureManager::update() {
    if (m_need_reload) {
        hot_reload();
    }
}

void TextureManager::need_reload() { m_need_reload = true; }

void TextureManager::hot_reload() {
    delete_texture();

    init_texture();
    m_need_reload = false;
}

int TextureManager::max_aniso() const { return static_cast<int>(m_max_aniso); }

} // namespace Cubed