#pragma once
#include "Cubed/render/model_node.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <filesystem>

namespace cubed {

class ModelLoader {
public:
    ModelLoader();
    ModelNode load(const std::filesystem::path& path);

private:
    Assimp::Importer m_importer;
    ModelNode process_node(aiNode* node, const aiScene* scene);
    Mesh process_mesh(aiMesh* mesh, const aiScene* scene);
    bool process_texture(Mesh& mesh, aiMaterial* material, const aiScene* scene,
                         aiTextureType type);
    glm::mat4 convert_matrix(const aiMatrix4x4& matrix);
};

} // namespace cubed
