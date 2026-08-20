#include "Cubed/localization.hpp"

#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/json_utils.hpp"
#include "Cubed/tools/log.hpp"

#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
namespace fs = std::filesystem;
using namespace rapidjson;
namespace cubed {
Localization::Localization() {}
Localization& Localization::instance() {
    static Localization s_instance;
    return s_instance;
}

void Localization::load_language(std::string_view language) {
    m_current_lang = language;
    std::string path =
        ASSETS_PATH + std::format("lang/{}.json", m_current_lang);
    std::ifstream file(path);
    if (!file.is_open()) {
        Logger::error("Can't Open File {}", path);
        ASSERT(false);
        return;
    }
    try {
        IStreamWrapper isw(file);
        Document doc;
        doc.ParseStream(isw);
        if (doc.HasParseError()) {
            auto code = doc.GetParseError();

            throw std::runtime_error(
                std::format("Parse {} failed, error code {}", path,
                            static_cast<int>(code)));
        }
        m_translations.clear();
        m_translations = tools::doc_to_map(doc);
    } catch (const std::exception& e) {
        Logger::error("JSON syntax error: {}", e.what());
    }
}

std::string_view Localization::lookup(const std::string& key) const {
    auto it = m_translations.find(key);
    if (it != m_translations.end()) {
        return it->second;
    }
    Logger::error("Can't find key {} in language {}", key, m_current_lang);

    return key;
}

bool Localization::has(const std::string& key) const {
    return m_translations.contains(key);
}

void Localization::replace_all(std::string& str, std::string_view from,
                               std::string_view to) {

    std::size_t pos = 0;

    while ((pos = str.find(from, pos)) != std::string::npos) {
        str.replace(pos, from.size(), to);
        pos += to.size();
    }
}

} // namespace cubed
