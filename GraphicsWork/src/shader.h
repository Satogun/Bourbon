
#ifndef SHADER_H
#define SHADER_H

#include "../include/glad/glad.h"
#include <string>

#include "../include/glm/glm.hpp"

struct Shader {
    //Initialization 
    bool setup( std::string vertexPath,  std::string fragmentPath,
         std::string geometryPath = "");

    //Shader program marked for deletion on destructor call
    //CAREFUL with passing by value, any destructor call wipes the shader out!!
    ~Shader() {
        glDeleteProgram(ID);
    }

    //Activate shader program for use
    void use() const;

    //Uniform setting functions
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;
    void setVec3(const std::string& name, const glm::vec3& vec) const;

    //Shader program ID for referencing
    unsigned int ID;
};

//Compute shaders require a different constructor since they are simpler and stored elsewhere
struct ComputeShader : public Shader {
    bool setup(const std::string computePath);
    void dispatch(unsigned int x, unsigned int y = 1, unsigned int z = 1) const;
};

#endif