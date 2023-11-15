

#include "geometry.h"
#include <limits>
#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>
#include <algorithm>

//-------------------------------AABOX------------------------------------//

void AABox::buildAABB(const Mesh& mesh) {
    glm::vec3 minVals(std::numeric_limits<float>::max());
    glm::vec3 maxVals(std::numeric_limits<float>::min());

    //Iterating through every vertx in the mesh
    for (int i = 0; i < mesh.vertices.size(); ++i) {
        //Setting max values
        maxVals.x = std::max(maxVals.x, mesh.vertices[i].position.x);
        maxVals.y = std::max(maxVals.y, mesh.vertices[i].position.y);
        maxVals.z = std::max(maxVals.z, mesh.vertices[i].position.z);

        //Setting min values
        minVals.x = std::min(minVals.x, mesh.vertices[i].position.x);
        minVals.y = std::min(minVals.y, mesh.vertices[i].position.y);
        minVals.z = std::min(minVals.z, mesh.vertices[i].position.z);
    }
    minPoints = minVals;
    maxPoints = maxVals;
}

void AABox::update(const glm::mat4& modelMatrix) {
}


float Plane::distance(const glm::vec3& points) {
    return glm::dot(normal, points) + D;
}

//Setup function using a normal and a point on the plane
void Plane::setNormalAndPoint(const glm::vec3& n, const glm::vec3& p0) {
    normal = n;
    D = -glm::dot(n, p0);
}

//-------------------------------FRUSTRUM------------------------------------//

void Frustum::setCamInternals() {
    float tanHalfFOV = (float)tan(glm::radians(fov / 2.0f));
    nearH = nearPlane * tanHalfFOV; //Half of the frustrum near plane height
    nearW = nearH * AR;
}

//Calculates frustrum planes in world space
void Frustum::updatePlanes(glm::mat4& viewMat, const glm::vec3& cameraPos) {
    setCamInternals();
    glm::vec3 X(viewMat[0][0], viewMat[0][1], viewMat[0][2]); 
    glm::vec3 Y(viewMat[1][0], viewMat[1][1], viewMat[1][2]); 
    glm::vec3 Z(viewMat[2][0], viewMat[2][1], viewMat[2][2]); 


    glm::vec3 nearCenter = cameraPos - Z * nearPlane;
    glm::vec3 farCenter = cameraPos - Z * farPlane;

    glm::vec3 point;
    glm::vec3 normal;


    pl[NEARP].setNormalAndPoint(-Z, nearCenter);

    pl[FARP].setNormalAndPoint(Z, farCenter);


    point = nearCenter + Y * nearH;
    normal = glm::normalize(point - cameraPos);
    normal = glm::cross(normal, X);
    pl[TOP].setNormalAndPoint(normal, point);

    //Bottom plane
    point = nearCenter - Y * nearH;
    normal = glm::normalize(point - cameraPos);
    normal = glm::cross(X, normal);
    pl[BOTTOM].setNormalAndPoint(normal, point);

    //Left plane
    point = nearCenter - X * nearW;
    normal = glm::normalize(point - cameraPos);
    normal = glm::cross(normal, Y);
    pl[LEFT].setNormalAndPoint(normal, point);

    //Right plane
    point = nearCenter + X * nearW;
    normal = glm::normalize(point - cameraPos);
    normal = glm::cross(Y, normal);
    pl[RIGHT].setNormalAndPoint(normal, point);

}

bool Frustum::checkIfInside(AABox* box) {

    //Check box outside or inside of frustrum
    for (int i = 0; i < 6; ++i) {
        int out = 0;
        out += ((pl[i].distance(glm::vec3(box->minPoints.x, box->minPoints.y, box->minPoints.z)) < 0.0) ? 1 : 0);
        out += ((pl[i].distance(glm::vec3(box->maxPoints.x, box->minPoints.y, box->minPoints.z)) < 0.0) ? 1 : 0);
        out += ((pl[i].distance(glm::vec3(box->minPoints.x, box->maxPoints.y, box->minPoints.z)) < 0.0) ? 1 : 0);
        out += ((pl[i].distance(glm::vec3(box->maxPoints.x, box->maxPoints.y, box->minPoints.z)) < 0.0) ? 1 : 0);
        out += ((pl[i].distance(glm::vec3(box->minPoints.x, box->minPoints.y, box->maxPoints.z)) < 0.0) ? 1 : 0);
        out += ((pl[i].distance(glm::vec3(box->maxPoints.x, box->minPoints.y, box->maxPoints.z)) < 0.0) ? 1 : 0);
        out += ((pl[i].distance(glm::vec3(box->minPoints.x, box->maxPoints.y, box->maxPoints.z)) < 0.0) ? 1 : 0);
        out += ((pl[i].distance(glm::vec3(box->maxPoints.x, box->maxPoints.y, box->maxPoints.z)) < 0.0) ? 1 : 0);

        if (out == 8) return false;
    }
    return true;
}