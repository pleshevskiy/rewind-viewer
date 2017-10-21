//
// Created by valdemar on 14.10.17.
//

#include "Shader.h"

#include <glm/gtc/type_ptr.hpp>

#include <cstring>
#include <stdexcept>

namespace {

std::string load_file(const std::string &file_path) {
    FILE *fd = fopen(file_path.c_str(), "r");
    if (!fd) {
        char err_buf[512];
        sprintf(err_buf, "Load file: %s", strerror(errno));
        throw std::runtime_error(err_buf);
    }

    constexpr uint16_t CHUNK_SIZE = 256;
    char buf[CHUNK_SIZE];
    std::string content;
    while (size_t sz = fread(buf, 1, CHUNK_SIZE, fd)) {
        buf[sz] = '\0';
        content += buf;
    }
    return content;
}

bool validate_shader(GLuint shader) {
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar buf[512];
        GLsizei len;
        glGetShaderInfoLog(shader, 512, &len, buf);
        fprintf(stderr, "Error::Compile shader:: %*s", len, buf);
        return false;
    }
    return true;
}

bool validate_program(GLuint program) {
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar buf[512];
        GLsizei len;
        glGetShaderInfoLog(program, 512, &len, buf);
        fprintf(stderr, "Error::Link shader program:: %*s", len, buf);
        return false;
    }
    return true;
}

GLuint create_shader(GLenum type, const std::string &source) {
    GLuint shader = glCreateShader(type);
    const auto source_ptr = source.data();
    glShaderSource(shader, 1, &source_ptr, nullptr);
    glCompileShader(shader);
    if (!validate_shader(shader)) {
        fprintf(stderr, "Shader type: %s\n", (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment"));
        throw std::runtime_error("Compile shader");
    }
    return shader;
}

GLuint create_shader_program(GLuint vert_shader, GLuint frag_shader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);
    glLinkProgram(program);
    if (!validate_program(program)) {
        throw std::runtime_error("Link shader program");
    }
    return program;
}

} // anonymous namespace

Shader::Shader(const std::string &vertex_shader_path, const std::string &fragment_shader_path) {
    const auto vs_src = load_file(vertex_shader_path);
    const auto fs_src = load_file(fragment_shader_path);

    auto v_shader = create_shader(GL_VERTEX_SHADER, vs_src);
    auto f_shader = create_shader(GL_FRAGMENT_SHADER, fs_src);

    program_ = create_shader_program(v_shader, f_shader);

    glDeleteShader(v_shader);
    glDeleteShader(f_shader);
}

Shader::~Shader() {
    glDeleteProgram(program_);
}

void Shader::use() {
    glUseProgram(program_);
}

GLint Shader::uniform(const std::string &name) {
    GLint loc = glGetUniformLocation(program_, name.c_str());
    if (loc == -1) {
        fprintf(stderr, "Warning::No such uniform::%s", name.c_str());
    }
    return loc;
}

void Shader::set_mat4(const std::string &name, const glm::mat4 &v) {
    glUniformMatrix4fv(uniform(name), 1, GL_FALSE, glm::value_ptr(v));
}

void Shader::set_vec3(const std::string &name, const glm::vec3 &v) {
    glUniform3f(uniform(name), v.x, v.y, v.z);
}

void Shader::set_mat4(const std::string &name, float *pv) {
    glUniformMatrix4fv(uniform(name), 1, GL_FALSE, pv);
}