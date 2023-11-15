#pragma once

/*
AUTHOR       : Angel Ortiz (angelo12 AT vt DOT edu)
PROJECT      : Hybrid Rendering Engine
LICENSE      : This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
DATE	     : 2018-11-19
PURPOSE      : Abstract file loading and checking from scene and other implementation
               classes.
SPECIAL NOTES: Pretty bare bones, should look into which functions could be moved here.
*/

//Header files
#include "../include/glad/glad.h"
#include "../include/GLFW/glfw3.h"
#include <string>

namespace FILELOAD 
{
    bool checkFileValidity(const std::string& filePath);
    std::string getFileExtension(const std::string& filePath);
}

#define ASSERT(x) if (!(x)) __debugbreak(); 
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))

void GLClearError();

bool GLLogCall(const char* function, const char* file, int line);