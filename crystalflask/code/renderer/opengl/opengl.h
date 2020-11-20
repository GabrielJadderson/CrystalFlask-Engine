#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/ext/matrix_transform.hpp>



struct Light
{
    glm::vec3 Direction;
    glm::vec3 Radiance;
};

Light Light;
float LightMultiplier = 0.3f;


