#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <string>
#include <glm/glm.hpp>

class Shader
{
public:
    GLuint ID;

    Shader(const char* vertexPath, const char* fragmentPath);

    void use() const;

    void setFloat(const std::string& name, float value) const;
    void setInt(const std::string& name, int value) const;
    void setVec3(const std::string& name, float v0, float v1, float v2) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;

private:
    std::string readFile(const char* filePath) const;
    void checkCompileErrors(GLuint shader, const std::string& type) const;
};

#endif