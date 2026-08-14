#include "Cubed/tools/json_utils.hpp"

#include "Cubed/tools/file_utils.hpp"
#include "Cubed/tools/log.hpp"

#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <string>
#include <unordered_map>
namespace Cubed::Tools {

using rapidjson::Document;
using rapidjson::IStreamWrapper;
using rapidjson::Value;
namespace fs = std::filesystem;
namespace {
std::string serialize_value(const Value& v, rapidjson::StringBuffer& buf) {
    rapidjson::Writer<rapidjson::StringBuffer> w(buf);
    v.Accept(w);
    return buf.GetString();
}
} // namespace

std::unordered_map<std::string, std::string> doc_to_map(const Document& doc) {
    std::unordered_map<std::string, std::string> m;
    if (!doc.IsObject())
        return m;

    for (auto it = doc.MemberBegin(); it != doc.MemberEnd(); ++it) {
        const std::string KEY(it->name.GetString(), it->name.GetStringLength());
        const Value& v = it->value;
        std::string val;
        if (v.IsString()) {
            val.assign(v.GetString(), v.GetStringLength());
        } else if (v.IsInt()) {
            val = std::to_string(v.GetInt());
        } else if (v.IsUint()) {
            val = std::to_string(v.GetUint());
        } else if (v.IsInt64()) {
            val = std::to_string(v.GetInt64());
        } else if (v.IsUint64()) {
            val = std::to_string(v.GetUint64());
        } else if (v.IsDouble()) {
            val = std::to_string(v.GetDouble());
        } else if (v.IsBool()) {
            val = v.GetBool() ? "true" : "false";
        } else if (v.IsNull()) {
            val = "null";
        } else {
            rapidjson::StringBuffer buf;
            val = serialize_value(v, buf);
        }
        m.emplace(KEY, val);
    }
    return m;
}

bool parse_json(rapidjson::Document& doc, const std::filesystem::path& path) {

    std::ifstream file{path};
    if (!file.is_open()) {
        Logger::error("Can't parse json {}", path.string());
        return false;
    }
    IStreamWrapper isw{file};
    doc.ParseStream(isw);
    if (doc.HasParseError()) {
        auto code = doc.GetParseError();
        Logger::error("Parse {} failed, error code {}", path.string(),
                      static_cast<int>(code));
        return false;
    }
    return true;
}

bool parse_json_from_string(rapidjson::Document& doc, std::string_view json) {
    doc.Parse(json.data(), json.size());
    if (doc.HasParseError()) {
        const auto CODE = doc.GetParseError();
        Logger::error("Parse json string failed, error code {}",
                      static_cast<int>(CODE));
        return false;
    }
    return true;
}

void save_json(const rapidjson::Document& doc,
               const std::filesystem::path& path) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

    if (!doc.Accept(writer)) {
        throw std::runtime_error("Failed to serialize JSON");
    }
    if (path.parent_path().empty()) {
        throw std::runtime_error(
            std::format("Failed to create path {}", path.string()));
    }
    fs::create_directories(path.parent_path());
    fs::path temp_path = path;
    temp_path += ".tmp";
    try {
        {
            std::ofstream file(temp_path, std::ios::binary | std::ios::trunc);

            if (!file) {
                throw std::runtime_error("Failed to open " +
                                         temp_path.string());
            }

            file.write(buffer.GetString(),
                       static_cast<std::streamsize>(buffer.GetSize()));

            file.flush();

            if (!file) {
                throw std::runtime_error("Failed to write " +
                                         temp_path.string());
            }
        }

        atomic_replace(temp_path, path);
    } catch (...) {
        std::error_code ec;
        fs::remove(temp_path, ec);
        throw;
    }
}

std::string to_json_string(const rapidjson::Value& value) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    value.Accept(writer);

    return {buffer.GetString(), buffer.GetSize()};
}

} // namespace Cubed::Tools