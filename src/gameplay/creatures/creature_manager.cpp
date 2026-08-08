#include "Cubed/gameplay/creatures/creature_manager.hpp"

#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/json_utils.hpp"

#include <filesystem>
#include <rapidjson/document.h>
namespace fs = std::filesystem;
using namespace rapidjson;
namespace Cubed {
CreatureManager::CreatureManager() {}
CreatureManager::~CreatureManager() {}
CreatureManager& CreatureManager::instance() {
    static CreatureManager inst;
    return inst;
}

void CreatureManager::init() {

    fs::path root_path{ResourceLocation::get_assets_path_prefix(
        ResourceLocation::DEFAULT_NAMESPACE)};
    fs::path creature_path = root_path / "creatures";

    fs::create_directories(creature_path);

    for (auto& entry : fs::recursive_directory_iterator(creature_path)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        if (entry.path().extension() != ".json") {
            continue;
        }

        Document doc;
        if (!Tools::parse_json(doc, entry.path())) {
            continue;
        }

        std::string name;
        if (!Tools::get_json_value(doc, "name", name)) {
            Logger::error("creature json {} doesn't have name",
                          entry.path().string());
            continue;
        }

        CreatureData data;
        auto n = ResourceLocation::parse(name);
        if (!n) {
            continue;
        }
        data.name = *n;
        std::string s;
        if (Tools::get_json_value(doc, "model", s)) {
            data.model = ResourceLocation::parse(s);
        }
        if (Tools::get_json_value(doc, "animation", s)) {
            data.animation = ResourceLocation::parse(s);
        }
        if (Tools::get_json_value(doc, "collision", s)) {
            data.collision = ResourceLocation::parse(s);
        }
    }
}

const CreatureData&
CreatureManager::get_creature_data(std::string_view name) const {
    auto l = ResourceLocation::parse(name);
    if (l) {
        return get_creature_data(*l);
    } else {
        Logger::error("Can't get creature {} data", name);
        ASSERT(false);
        return EMPTY;
    }
}

const CreatureData&
CreatureManager::get_creature_data(const ResourceLocation& location) const {
    cacc c;
    if (m_creature_map.find(c, location)) {
        return c->second;
    }
    Logger::error("Can't get creature {} data", location.to_string());
    return EMPTY;
}

const CreatureData& CreatureManager::data(std::string_view name) {
    return instance().get_creature_data(name);
}
const CreatureData& CreatureManager::data(const ResourceLocation& location) {
    return instance().get_creature_data(location);
}

} // namespace Cubed