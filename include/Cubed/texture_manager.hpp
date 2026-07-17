#pragma once
#include "Cubed/config.hpp"
#include "Cubed/gameplay/block.hpp"
#include "Cubed/input/event.hpp"
#include "Cubed/render/texture.hpp"

#include <glad/glad.h>
#include <memory>

namespace Cubed {

class TextureManager {
private:
    bool m_need_reload = false;
    bool m_init = false;
    std::unique_ptr<Texture> m_block_status_array;
    std::unique_ptr<Texture> m_texture_array;
    std::unique_ptr<Texture> m_cross_plane_array;
    std::unique_ptr<Texture> m_normal_texture_array;
    std::unique_ptr<Texture> m_skin;
    std::vector<std::unique_ptr<Texture>> m_item_textures;
    std::unordered_map<std::string, std::unique_ptr<Texture>> m_ui_map;
    GLfloat m_max_aniso = 0.0f;
    Config& m_config;
    int m_aniso = 1;

    void load_block_status(unsigned status_id);
    void load_block_texture(unsigned block_id);
    void load_block_item_texture(unsigned id);
    void load_cross_plane_texture(unsigned id);
    const Texture* load_image_texture(const std::string& path);
    void load_pbr_texture(unsigned id);
    void init_item();
    void init_block();
    void init_ui();
    void init_block_status();
    void init_skin();
    void hot_reload();
    bool handle_key_event(const KeyEvent& e);

public:
    TextureManager(Config& config);
    ~TextureManager();

    void delete_texture();
    const Texture* get_block_status_array() const;
    const Texture* get_texture_array() const;
    const Texture* get_cross_plane_array() const;
    const Texture* get_image_texture(const std::string& path);
    const Texture* get_pbr_texture() const;
    const std::vector<std::unique_ptr<Texture>>& item_textures() const;
    const Texture* get_skin() const;
    void init_texture();

    void need_reload();
    void update();
    int max_aniso() const;
    bool handle_event(const Event& e);
};

} // namespace Cubed