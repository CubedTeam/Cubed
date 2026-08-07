#include "Cubed/tools/model_loader.hpp"

#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/log.hpp"
#include "Cubed/tools/shader_tools.hpp"

#include <assimp/postprocess.h>
#include <stb_image.h>
namespace Cubed {
ModelLoader::ModelLoader() {}

ModelNode ModelLoader::load(const std::string& path) {
    const aiScene* scene = m_importer.ReadFile(
        path, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs |
                  aiProcess_JoinIdenticalVertices |
                  aiProcess_ImproveCacheLocality);
    if (!scene || !scene->mRootNode) {
        Logger::error("Load Model {} Error: {}", path,
                      m_importer.GetErrorString());
        return {};
    }
    return process_node(scene->mRootNode, scene);
}
ModelNode ModelLoader::process_node(aiNode* node, const aiScene* scene) {
    ModelNode result;
    result.name = node->mName.C_Str();
    result.transform = convert_matrix(node->mTransformation);

    for (unsigned i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        result.meshes.emplace_back(process_mesh(mesh, scene));
    }

    for (unsigned i = 0; i < node->mNumChildren; ++i) {
        result.children.emplace_back(process_node(node->mChildren[i], scene));
    }
    return result;
}

Mesh ModelLoader::process_mesh(aiMesh* mesh, const aiScene* scene) {
    Mesh result;
    for (unsigned i = 0; i < mesh->mNumVertices; ++i) {
        Vertex3D v{};
        v.x = mesh->mVertices[i].x;
        v.y = mesh->mVertices[i].y;
        v.z = mesh->mVertices[i].z;
        if (mesh->HasTextureCoords(0)) {
            v.s = mesh->mTextureCoords[0][i].x;
            v.t = mesh->mTextureCoords[0][i].y;
        }
        if (mesh->HasNormals()) {
            v.nx = mesh->mNormals[i].x;
            v.ny = mesh->mNormals[i].y;
            v.nz = mesh->mNormals[i].z;
        } else {
            Logger::warn("Model Does not have Normals!");
        }
        result.vertices.emplace_back(v);
    }

    for (unsigned i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];

        for (unsigned j = 0; j < face.mNumIndices; ++j) {
            result.indices.emplace_back(face.mIndices[j]);
        }
    }
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    if (!process_texture(result, material, scene, aiTextureType_BASE_COLOR)) {
        if (!process_texture(result, material, scene, aiTextureType_DIFFUSE)) {
            if (!process_texture(result, material, scene,
                                 aiTextureType_UNKNOWN)) {
                Logger::error("Can't Load model texture");
                ASSERT(false);
            }
        }
    }
    result.upload();
    return result;
}

bool ModelLoader::process_texture(Mesh& mesh, aiMaterial* material,
                                  const aiScene* scene, aiTextureType type) {
    aiString texture_path;
    if (material->GetTexture(type, 0, &texture_path) != AI_SUCCESS) {
        return false;
    }
    mesh.texture = std::make_unique<Texture>(TextureType::TEXTURE_2D);
    if (texture_path.C_Str()[0] == '*') {
        int index = std::stoi(texture_path.C_Str() + 1);
        aiTexture* texture = scene->mTextures[index];
        if (texture->mHeight == 0) {
            int width, height, channels;

            unsigned char* data = stbi_load_from_memory(
                reinterpret_cast<unsigned char*>(texture->pcData),
                texture->mWidth, &width, &height, &channels, STBI_rgb_alpha);

            mesh.texture->tex_image_2d(RGBA, RGBA, GL_UNSIGNED_BYTE, data,
                                       width, height);
            stbi_image_free(data);
        } else {
            mesh.texture->tex_image_2d(RGBA, BGRA, GL_UNSIGNED_BYTE,
                                       reinterpret_cast<char*>(texture->pcData),
                                       texture->mWidth, texture->mHeight);
        }
    } else {
        auto image_data =
            Tools::load_image_data(texture_path.C_Str(), true, true);

        mesh.texture->tex_image_2d(RGBA, RGBA, GL_UNSIGNED_BYTE,
                                   image_data.data, image_data.width,
                                   image_data.height);
    }
    mesh.texture->set_nearest();
    return true;
}

glm::mat4 ModelLoader::convert_matrix(const aiMatrix4x4& m) {
    return glm::mat4(m.a1, m.b1, m.c1, m.d1, m.a2, m.b2, m.c2, m.d2, m.a3, m.b3,
                     m.c3, m.d3, m.a4, m.b4, m.c4, m.d4);
}

} // namespace Cubed