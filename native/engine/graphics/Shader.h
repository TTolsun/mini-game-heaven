#pragma once

#include <GLES3/gl3.h>

#include <string_view>

namespace engine {

// Compiled + linked GLSL ES program.
class Shader {
public:
    Shader() = default;
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    bool build(std::string_view vertexSource, std::string_view fragmentSource);
    void use() const;

    GLint uniform(const char* name) const;
    GLuint id() const { return program_; }

private:
    static GLuint compile(GLenum type, std::string_view source);

    GLuint program_ = 0;
};

}  // namespace engine
