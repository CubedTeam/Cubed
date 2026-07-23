#pragma once

#include <glad/glad.h>
#include <string>
#include <utility>
namespace Cubed {

namespace Tools {
void delete_image_data(unsigned char* data);
}

struct ImageData {
    unsigned char* data = nullptr;
    int width = 0;
    int height = 0;
    int channels = 0;
    ImageData(const ImageData&) = delete;
    ImageData(ImageData&& o) noexcept
        : data(std::exchange(o.data, nullptr)), width(o.width),
          height(o.height), channels(o.channels) {}
    ImageData& operator=(const ImageData&) = delete;
    ImageData& operator=(ImageData&& o) noexcept {
        if (this == &o) {
            return *this;
        }
        if (data) {
            Tools::delete_image_data(data);
        }
        data = std::exchange(o.data, nullptr);
        width = o.width;
        height = o.height;
        channels = o.channels;
        return *this;
    }
    ImageData(unsigned char* d, int w, int h, int c)
        : data(d), width(w), height(h), channels(c) {}
    ImageData() = default;
    ~ImageData() { Tools::delete_image_data(data); }
};

namespace Tools {
GLuint create_shader_program(const std::string& v_shader_path,
                             const std::string& f_shader_path);
void print_shader_log(GLuint shader);
void print_program_info(int prog);
bool check_opengl_error();
std::string read_shader_source(const std::string& file_path);

ImageData load_image_data(const std::string& tex_image_path,
                          bool check_exist = true);

} // namespace Tools

} // namespace Cubed
