#ifndef OBJECT_HPP
#define OBJECT_HPP

#include "vulkaninstance.hpp"

class Octree;
class Object{
public:
    Object(Octree *octree, glm::uvec3 position_, glm::uvec3 rotation_, glm::uvec3 scale_, uint32_t object_id);
    ~Object();
    bool loadWavefrontObj(std::string filename, bool hasTexture, bool hasNormal);

    void setPosition(glm::uvec3 p);
    void setRotation(glm::vec3 r);

    void update();
    void remove();
public:
    glm::uvec3 position;
    glm::uvec3 rotation;
    glm::uvec3 scale;
    uint32_t id;
    std::string tag;
private:
    std::vector<glm::uvec3> voxels_p[2]; //position
    std::vector<glm::uvec3> voxels_data_p[2]; //normal & albedo
    std::vector<glm::vec3> normals_p; //normal
    Octree *octree_p;

    struct triangle{
            glm::vec3 v[3];
            glm::vec3 t[3];
            glm::vec3 normal[3];
            int vertex_id[3];
    };
};

#endif