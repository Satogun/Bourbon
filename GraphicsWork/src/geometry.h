#ifndef GEOMETRY_H
#define GEOMETRY_H


#include "../include/glm/glm.hpp"
#include "mesh.h"


struct AABox 
{
    glm::vec3 minPoints;
    glm::vec3 maxPoints;

    void buildAABB(const Mesh& mesh);
    void update(const glm::mat4& modelMatrix);
};

//Only used in frustrum culling, a frustrum has 6 planes
//Equation is Ax + By + Cz + D = 0 
struct Plane {
    glm::vec3 normal;
    float D;

    float distance(const glm::vec3& points);
    void setNormalAndPoint(const glm::vec3& normal, const glm::vec3& point);
};
struct Frustum 
{
    enum planes
    {
        TOP = 0,
        BOTTOM,
        LEFT,
        RIGHT,
        NEARP,
        FARP
    };

    Plane pl[6];
    float fov, nearPlane, farPlane, AR, nearH, nearW;

    void setCamInternals();
    void updatePlanes(glm::mat4& viewMat, const glm::vec3& cameraPos);
    bool checkIfInside(AABox* bounds);
};
#endif