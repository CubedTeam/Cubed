#include "Cubed/gameplay/hitbox_manager.hpp"

#include "Cubed/tools/log.hpp"

#include <nlohmann/json.hpp>
namespace fs = std::filesystem;
using nlohmann::json;
namespace Cubed {
HitboxManager::HitboxManager() {}
HitboxManager::~HitboxManager() {}

HitboxManager& HitboxManager::instance() {
    static HitboxManager inst;
    return inst;
}

AABB HitboxManager::aabb(const std::string& key) {
    return instance().get_aabb(key);
}

AABB HitboxManager::get_aabb(const std::string& key) {
    {
        cacc c;
        if (m_hitboxes.find(c, key)) {
            return c->second;
        }
    }

    return load(key);
}

AABB HitboxManager::load(const std::string& path) {
    fs::path p = ASSETS_PATH + path;
    try {
        glm::vec3 center;
        glm::vec3 half;
        std::ifstream s{p};
        json j = json::parse(s);
        if (j.contains("boxes")) {
            if (j["boxes"].contains("center")) {
                center.x = j["boxes"]["center"].at(0).get<float>();
                center.y = j["boxes"]["center"].at(1).get<float>();
                center.z = j["boxes"]["center"].at(2).get<float>();
            }
            if (j["boxes"].contains("half")) {
                half.x = j["boxes"]["half"].at(0).get<float>();
                half.y = j["boxes"]["half"].at(1).get<float>();
                half.z = j["boxes"]["half"].at(2).get<float>();
            }
        }
        acc a;
        if (m_hitboxes.insert(a, path)) {
            a->second = AABB{center, half};
            return a->second;
        }
    } catch (const std::exception& e) {
        Logger::error("Load Hitbox error {}", e.what());
    }
    Logger::error("Load hitbox {} Failed", path);
    return AABB{glm::vec3(0.0f), glm::vec3(0.0f)};
}

} // namespace Cubed