#include "utils.hpp"

glm::vec3 utils::GetVNormal(glm::uvec3 data){
    glm::vec3 normal;
    normal.x = (data.x>>8)/100 - 1;
    normal.y = (data.y>>8)/100 - 1;
    normal.z = (data.z>>8)/100 - 1;
}

glm::uvec3 utils::SetVNormal(glm::vec3 normal){
    glm::vec3 data;
    data.x += (uint8_t)(normal.x*100 + 100)<<8;
    data.y += (uint8_t)(normal.y*100 + 100)<<8;
    data.z += (uint8_t)(normal.z*100 + 100)<<8;
    return data;
}

glm::uvec3 utils::RemoveNormal(glm::uvec3 data){
    glm::uvec3 newData;
    newData.x = data.x % 256; 
    newData.y = data.y % 256; 
    newData.z = data.z % 256; 
    return newData;
}

glm::uvec3 utils::AddNormal(glm::uvec3 data, glm::uvec3 normalData){
    glm::uvec3 newData;
    newData = data + normalData;
    return newData;
}

glm::vec3 utils::eulerToVector(glm::uvec2 rotation){
    glm::vec3 vec;
    vec.x = cosf(glm::radians(rotation.x))*cosf(glm::radians(rotation.y));
    vec.y = sinf(glm::radians(rotation.x))*cosf(glm::radians(rotation.y));
    vec.z = sinf(glm::radians(rotation.y));
    return vec;
}

glm::uvec2 utils::vectorToEuler(glm::vec3 vector){
    glm::vec2 rot;
    rot.x = atan2(vector.z, vector.x);
    rot.y = asin(vector.y);
    return rot;
}