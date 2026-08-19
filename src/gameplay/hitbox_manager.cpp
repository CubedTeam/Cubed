#include "Cubed/gameplay/hitbox_manager.hpp"

#include "Cubed/gameplay/creatures/creature_manager.hpp"
#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/json_utils.hpp"
#include "Cubed/tools/log.hpp"
#include "Cubed/tools/resource_location.hpp"

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
namespace fs = std::filesystem;
using namespace rapidjson;
namespace cubed {

namespace {
const HitboxManager::Handle EMPTY{};
}

HitboxManager::HitboxManager() { HitboxMap::accessor a; }

HitboxManager::~HitboxManager() {}

HitboxManager& HitboxManager::instance() {
    static HitboxManager inst;
    return inst;
}

HitboxManager::Handle HitboxManager::hitbox(const std::string& key) {
    return instance().get_hitbox(key);
}

HitboxManager::Handle HitboxManager::hitbox(HitboxID id) {
    return instance().get_hitbox(id);
}
HitboxID HitboxManager::get_hitbox_id(const std::string& name) {
    IDMap::const_accessor cacc;
    if (m_id_map.find(cacc, name)) {
        return cacc->second;
    }
    return load(name).id;
}
std::string HitboxManager::get_hitbox_name(HitboxID id) {
    NameMap::const_accessor cacc;
    if (m_name_map.find(cacc, id)) {
        return cacc->second;
    }
    ASSERT_MSG(false, std::format("ModelManager: Can't find {}", id));
    static std::string n = "";
    return n;
}

HitboxManager::Handle HitboxManager::get_hitbox(HitboxID id) {
    {
        HitboxMap::const_accessor c;
        if (m_hitboxes.find(c, id)) {
            return {c->second, c->first};
        }
    }

    return load(get_hitbox_name(id));
}
HitboxManager::Handle HitboxManager::get_hitbox(const std::string& name) {
    return get_hitbox(get_hitbox_id(name));
}
HitboxManager::Handle HitboxManager::load(std::string_view name) {

    auto location = CreatureManager::data(name);
    if (!location.collision) {
        Logger::error("Can't find {} collision.json", name);
        ASSERT(false);
        return EMPTY;
    }
    fs::path p =
        location.collision->assets_path_prefix() / location.collision->path;

    try {
        glm::vec3 center{0.0f};
        glm::vec3 half{0.0f};

        Document doc;
        if (!tools::parse_json(doc, p)) {
            Logger::error("Can't parse hitbox {}", name);
            ASSERT(false);
            return EMPTY;
        }
        if (doc.HasMember("boxes")) {
            const Value& box = doc["boxes"];
            if (box.IsArray() && !box.Empty()) {
                const Value& b = box[0];
                if (b.HasMember("center")) {
                    center.x = b["center"][0].GetFloat();
                    center.y = b["center"][1].GetFloat();
                    center.z = b["center"][2].GetFloat();
                }
                if (b.HasMember("half")) {
                    half.x = b["half"][0].GetFloat();
                    half.y = b["half"][1].GetFloat();
                    half.z = b["half"][2].GetFloat();
                }
            }
        }
        {
            HitboxMap::accessor a;
            if (m_hitboxes.insert(a, std::pair<HitboxID, Hitbox>{
                                         m_next++, Hitbox{center, half}})) {
                m_name_map.emplace(a->first, name);
                m_id_map.emplace(name, a->first);
                return {a->second, a->first};
            }
        }
        HitboxMap::const_accessor cacc;
        if (m_hitboxes.find(cacc, get_hitbox_id(std::string(name)))) {
            return {cacc->second, cacc->first};
        }
    } catch (const std::exception& e) {
        Logger::error("Load Hitbox error {}", e.what());
    }
    Logger::error("Load hitbox {} Failed", name);
    return {Hitbox{glm::vec3(0.0f), glm::vec3(0.0f)}, 0};
}

} // namespace cubed
