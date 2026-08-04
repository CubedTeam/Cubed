
#include <rapidjson/document.h>
#include <string>
#include <unordered_map>
namespace Tools {
std::unordered_map<std::string, std::string>
doc_to_map(const rapidjson::Document& doc);
}