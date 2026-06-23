#include "Cubed/tools/shader_tools.hpp"

#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/log.hpp"

#include <filesystem>
#include <fstream>
#include <unordered_set>

namespace Cubed {

namespace fs = std::filesystem;

namespace {

bool is_include(const std::string& line) {

    std::string_view sv(line);

    auto first = sv.find_first_not_of(" \t");

    return first != std::string::npos && sv.substr(first, 8) == "#include";
}

std::string parse_include(const std::string& line) {
    auto pos = line.find("#include");
    auto start = line.find_first_of("<\"", pos);
    auto end = line.find_first_of(">\"", start + 1);

    if (start == std::string::npos || end == std::string::npos) {
        Logger::error("Invalid include: {}", line);
        return {};
    }
    return line.substr(start + 1, end - start - 1);
}

void load_shader(std::string& source, const fs::path& file,
                 std::unordered_set<fs::path>& included) {
    if (!fs::is_regular_file(file)) {
        Logger::error("File {} don't Exist!", file.string());
        return;
    }
    auto canon = fs::canonical(file);
    if (!included.insert(canon).second) {
        return;
    }
    std::ifstream in(file);
    if (!in.is_open()) {
        Logger::error("Can't open {}", file.string());
        return;
    }
    std::string line;
    while (std::getline(in, line)) {
        if (is_include(line)) {

            auto include_file = file.parent_path() / parse_include(line);
            source += "// begin include ";
            source += include_file.string();
            source += "\n";
            load_shader(source, include_file, included);
            source += "// end include ";
            source += include_file.string();
            source += "\n";
        } else {
            source.append(line + "\n");
        }
    }
}

} // namespace

namespace Tools {

GLuint create_shader_program(const std::string& v_shader_path,
                             const std::string& f_shader_path) {

    std::string v_shader_str =
        Tools::read_shader_source(ASSETS_PATH + v_shader_path);
    std::string f_shader_str =
        Tools::read_shader_source(ASSETS_PATH + f_shader_path);
    const char* v_shader_source = v_shader_str.c_str();
    const char* f_shader_source = f_shader_str.c_str();

    GLuint v_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint f_shader = glCreateShader(GL_FRAGMENT_SHADER);

    GLint vc, fc;
    glShaderSource(v_shader, 1, &v_shader_source, NULL);
    glShaderSource(f_shader, 1, &f_shader_source, NULL);
    glCompileShader(v_shader);
    Tools::check_opengl_error();
    glGetShaderiv(v_shader, GL_COMPILE_STATUS, &vc);
    if (vc != 1) {
        Logger::error("vertex compilation failed");
        Tools::print_shader_log(v_shader);
        ASSERT(0);
    }
    glCompileShader(f_shader);
    Tools::check_opengl_error();
    glGetShaderiv(f_shader, GL_COMPILE_STATUS, &fc);
    if (fc != 1) {
        Logger::error("fragment compilation failed");
        Tools::print_shader_log(f_shader);
        ASSERT(0);
    }
    GLuint vf_program = glCreateProgram();
    glAttachShader(vf_program, v_shader);
    glAttachShader(vf_program, f_shader);
    glLinkProgram(vf_program);

    GLint linked;
    Tools::check_opengl_error();
    glGetProgramiv(vf_program, GL_LINK_STATUS, &linked);
    if (linked != 1) {
        Logger::error("linking failed");
        Tools::print_program_info(vf_program);
        ASSERT(0);
    }
    glDeleteShader(v_shader);
    glDeleteShader(f_shader);
    return vf_program;
}

void print_shader_log(GLuint shader) {
    int len = 0;
    int ch_written = 0;
    char* log;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    if (len > 0) {
        log = (char*)malloc(len);
        glGetShaderInfoLog(shader, len, &ch_written, log);
        Logger::info("Shader Info Log: {}", log);
        free(log);
    }
}

void print_program_info(int prog) {
    int len = 0;
    int ch_written = 0;
    char* log;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
    if (len > 0) {
        log = (char*)malloc(len);
        glGetProgramInfoLog(prog, len, &ch_written, log);
        Logger::info("Program Info Log: {}", log);
        free(log);
    }
}

bool check_opengl_error() {
    bool found_error = false;
    int gl_err = glGetError();
    while (gl_err != GL_NO_ERROR) {
        Logger::error("glEorr: {}", gl_err);
        found_error = true;
        gl_err = glGetError();
    }

    return found_error;
}

std::string read_shader_source(const std::string& file_path) {
    std::string content;
    fs::path path(file_path);
    std::unordered_set<fs::path> included;
    load_shader(content, path, included);
    return content;
}

void delete_image_data(unsigned char* data) {
    if (data == nullptr) {
        return;
    }
    SOIL_free_image_data(data);
}

unsigned char* load_image_data(const std::string& tex_image_path,
                               bool check_exist) {
    fs::path path = ASSETS_PATH + tex_image_path;
    if (check_exist) {
        ASSERT_MSG(fs::is_regular_file(path), path.c_str());
    }
    unsigned char* data = nullptr;
    int width, height, channels;
    data =
        SOIL_load_image(path.string().c_str(), &width, &height, &channels,
                        SOIL_LOAD_RGBA); // Materials are all RGBA; must force
    // RGBA, otherwise sampling will fail.
    if (check_exist) {
        if (!data) {
            ASSERT_MSG(data, "Could not load texture" + path.string());
            std::abort();
        }
    }

    return data;
}

} // namespace Tools

} // namespace Cubed