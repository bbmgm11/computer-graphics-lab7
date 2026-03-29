#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>

#include <assimp/scene.h>
#include "Mesh.h"

class Model
{
public:
    std::vector<Mesh> meshes;

    explicit Model(const std::string& path);
    void Draw(Shader& shader) const;
    void DrawMesh(size_t index, Shader& shader) const;
    size_t GetMeshCount() const;

private:
    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
};

#endif