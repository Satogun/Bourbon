#ifndef MODEL_H
#define MODEL_H

#include "shader.h"
#include "texture.h"
#include "mesh.h"

#include "../include/assimp/Importer.hpp"
#include "../include/assimp/scene.h"
#include "../include/assimp/postprocess.h"
#include "../include/glm/glm.hpp"
#include "../include/glm/gtc/matrix_transform.hpp"


#include <vector>
#include <unordered_map>
#include <string>

struct TransformParameters 
{
    TransformParameters() : scaling(glm::vec3(1.0f)) {};
    glm::vec3 translation;
    float angle;
    glm::vec3 rotationAxis;
    glm::vec3 scaling;
};

struct Model 
{
    Model(const std::string meshPath, const TransformParameters initParameters, bool IBL) : IBL(IBL) {
        loadModel(meshPath);
        modelMatrix = glm::mat4(1.0);
        modelMatrix = glm::translate(modelMatrix, initParameters.translation);
        modelMatrix = glm::rotate(modelMatrix, initParameters.angle, initParameters.rotationAxis);
        modelMatrix = glm::scale(modelMatrix, initParameters.scaling);
    }
    //Base interface
    void loadModel(std::string path);
    void update(const unsigned int deltaT);
    void draw(const Shader& shader, const bool textured);

    //Model processing/loading functions
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<unsigned int> processTextures(const aiMaterial* material);

    //Object to world space matrix
    bool IBL;
    glm::mat4 modelMatrix;
    std::vector<Mesh> meshes; //Does it need to be a vector after initialization?

    //To avoid textures being loaded from disk more than once they are indexed into a dictionary
    std::unordered_map<std::string, Texture> textureAtlas;
    std::string directory, fileExtension;
};

#endif 