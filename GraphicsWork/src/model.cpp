

#include "model.h"
#include "geometry.h"
#include "Util.h"
#include <string>
#include <iostream>

void Model::loadModel(std::string path) 
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_OptimizeMeshes | aiProcess_CalcTangentSpace | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
    {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }

    fileExtension = FILELOAD::getFileExtension(path);
    directory = path.substr(0, path.find_last_of('/'));
    directory += "/";

    processNode(scene->mRootNode, scene);
}

void Model::draw(const Shader& shader, const  bool textured) {
    shader.setBool("IBL", IBL);
    for (int i = 0; i < meshes.size(); ++i) 
    {
        meshes[i].draw(shader, textured);
    }
}


void Model::update(const unsigned int deltaT) {
}


void Model::processNode(aiNode* node, const aiScene* scene) {

    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }


    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}


Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int > indices;
    std::vector<unsigned int > textures;


    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {

        Vertex vertex;
        glm::vec3 vector;

        //Process position
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;

        //Process tangent
        //vector.x = mesh->mTangents[i].x;
        //vector.y = mesh->mTangents[i].y;
        //vector.z = mesh->mTangents[i].z;
        //vertex.tangent = vector;
        //
        ////Process biTangent
        //vector.x = mesh->mBitangents[i].x;
        //vector.y = mesh->mBitangents[i].y;
        //vector.z = mesh->mBitangents[i].z;
        //vertex.biTangent = vector;
        //
        ////Process normals
        //vector.x = mesh->mNormals[i].x;
        //vector.y = mesh->mNormals[i].y;
        //vector.z = mesh->mNormals[i].z;
        //vertex.normal = vector;
        //
        //Process texture coords
        if (mesh->HasTextureCoords(0)) {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoords = vec;
        }
        else {
            vertex.texCoords = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    //Process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }

    //Process material and texture info
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    textures = processTextures(material);

    return Mesh(vertices, indices, textures);
}


std::vector<unsigned int> Model::processTextures(const aiMaterial* material) {
    std::vector<unsigned int> textures;


    aiString texturePath;
    aiTextureType type;
    std::string fullTexturePath;

    for (int tex = aiTextureType_NONE; tex <= aiTextureType_UNKNOWN; tex++) {
        type = static_cast<aiTextureType>(tex); 
        fullTexturePath = directory;

        if (material->GetTextureCount(type) > 0) {
       
            material->GetTexture(type, 0, &texturePath);
            fullTexturePath = fullTexturePath.append(texturePath.C_Str());

            //If this texture has not been added to the atlas yet we load it
            if (textureAtlas.count(fullTexturePath) == 0) {
                Texture texture;
                bool srgb = false;
                texture.loadTexture(fullTexturePath, srgb);
                textureAtlas.insert({ fullTexturePath, texture });
            }

            //We add it to the texture index array of loaded texture for a given mesh
            textures.push_back(textureAtlas.at(fullTexturePath).textureID);
        }
        else {

            switch (type) {
            case aiTextureType_LIGHTMAP:
            case aiTextureType_EMISSIVE:
            case aiTextureType_NORMALS:
            case aiTextureType_UNKNOWN:
                textures.push_back(0);
                break;
            }
        }
    }
    return textures;
}