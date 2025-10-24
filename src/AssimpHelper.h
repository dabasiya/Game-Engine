#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <assimp/matrix4x4.h>

#include <assimp/quaternion.h>

class AssimpHelper {
public:

    static glm::mat4 ConvertMatrixToGLM(const aiMatrix4x4& aimat) {
        glm::mat4 result;

        result[0][0] = aimat.a1; result[1][0] = aimat.a2; result[2][0] = aimat.a3; result[3][0] = aimat.a4;
        result[0][1] = aimat.b1; result[1][1] = aimat.b2; result[2][1] = aimat.b3; result[3][1] = aimat.b4;
        result[0][2] = aimat.c1; result[1][2] = aimat.c2; result[2][2] = aimat.c3; result[3][2] = aimat.c4;
        result[0][3] = aimat.d1; result[1][3] = aimat.d2; result[2][3] = aimat.d3; result[3][3] = aimat.d4;

        return result;
    }

    static glm::vec3 ConvertVec3ToGLM(const aiVector3D& vec) {
        glm::vec3 result;

        result.x = vec.x;
        result.y = vec.y;
        result.z = vec.z;

        return result;
    }

    static glm::quat ConvertQuatToGLM(const aiQuaternion& quat) {
        glm::quat result;
        result.w = quat.w;
        result.x = quat.x;
        result.y = quat.y;
        result.z = quat.z;

        return result;
    }
};