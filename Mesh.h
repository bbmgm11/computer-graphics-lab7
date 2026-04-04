#ifndef MESH_H
#define MESH_H

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "shader.h"

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
};

class Mesh
{
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    GLuint VAO;

    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    void Draw(Shader& shader) const;

private:
    GLuint VBO, EBO;
    void setupMesh();
};

#endif