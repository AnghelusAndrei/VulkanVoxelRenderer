#ifndef OBJECT_HPP
#define OBJECT_HPP

#include "vulkaninstance.hpp"

class utils;
class Octree;
class Object{
public:
    Object(Octree *octree, glm::uvec3 position_, glm::uvec2 rotation_, glm::vec3 scale_);
    ~Object();
    bool loadWavefrontObj(std::string filename, bool hasTexture, bool hasNormal);

    void update();
    void custom(void (*customVoxelIntruction)(uint32_t voxel_Id, Octree *octree));
    void remove();
public:
    glm::uvec3 position;
    glm::uvec2 rotation;
    glm::vec3 scale;
    uint16_t id;
private:
    std::vector<glm::uvec3> voxels_p; //position
    std::vector<glm::uvec3> voxels_data_p; //normal & albedo
    std::vector<glm::vec3> normals_p; //normal
    utils *Utils;
    Octree *octree_p;

    const glm::vec3 upVector = glm::vec3(0,1,0);

    struct MeshData{
        bool hasTexture;
        bool hasNormal;
    } mesh_p;

    struct AABB{
        glm::vec3 MIN;
        glm::vec3 MAX;
    } BoundingBox_p;

    struct triangle{
            glm::vec3 v[3];
            glm::vec3 t[3];
            glm::vec3 normal[3];
            int vertex_id[3];
    };

    glm::uvec3 oldPosition_p;
    glm::uvec2 oldRotation_p;
private:
    void Rasterize(triangle t);
    glm::vec3 GetRaw(glm::uvec3 voxel);
    glm::uvec3 SetCurrent(glm::vec3 raw);
};

#endif