#include "Cubed/tools/json_utils.hpp"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <string>
#include <unordered_map>
namespace Tools {

using rapidjson::Document;
using rapidjson::Value;

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
} // namespace Tools