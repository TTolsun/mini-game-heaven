#define LOG_TAG "Shader"
#include "engine/graphics/Shader.h"

#include <string>

#include "engine/core/Log.h"

namespace engine {

Shader::~Shader() {
    if (program_ != 0) {
        glDeleteProgram(program_);
    }
}

GLuint Shader::compile(GLenum type, std::string_view source) {
    const GLuint shader = glCreateShader(type);
    const GLchar* text = source.data();
    const GLint length = static_cast<GLint>(source.size());
    glShaderSource(shader, 1, &text, &length);
    glCompileShader(shader);

    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (ok == GL_FALSE) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        LOGE("compile failed (%s): %s", type == GL_VERTEX_SHADER ? "vertex" : "fragment", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool Shader::build(std::string_view vertexSource, std::string_view fragmentSource) {
    const GLuint vs = compile(GL_VERTEX_SHADER, vertexSource);
    const GLuint fs = compile(GL_FRAGMENT_SHADER, fragmentSource);
    if (vs == 0 || fs == 0) {
        glDeleteShader(vs);
        glDeleteShader(fs);
        return false;
    }

    program_ = glCreateProgram();
    glAttachShader(program_, vs);
    glAttachShader(program_, fs);
    glLinkProgram(program_);
    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint ok = GL_FALSE;
    glGetProgramiv(program_, GL_LINK_STATUS, &ok);
    if (ok == GL_FALSE) {
        char log[1024];
        glGetProgramInfoLog(program_, sizeof(log), nullptr, log);
        LOGE("link failed: %s", log);
        glDeleteProgram(program_);
        program_ = 0;
        return false;
    }
    return true;
}

void Shader::use() const {
    glUseProgram(program_);
}

GLint Shader::uniform(const char* name) const {
    return glGetUniformLocation(program_, name);
}

}  // namespace engine
