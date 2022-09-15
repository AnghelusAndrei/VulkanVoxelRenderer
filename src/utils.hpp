#ifndef UTILS_HPP
#define UTILS_HPP

#include "vulkaninstance.hpp"

class utils{
public:

    glm::vec3 GetVNormal(glm::uvec3 data);
    glm::uvec3 SetVNormal(glm::vec3 data);
    glm::uvec3 RemoveNormal(glm::uvec3 data);
    glm::uvec3 AddNormal(glm::uvec3 data, glm::uvec3 normal);

    glm::vec3 eulerToVector(glm::uvec2 rotation);
    glm::uvec2 vectorToEuler(glm::vec3 vector);

};

#endif