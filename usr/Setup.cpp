#include "../src/voxelengine.hpp"

struct VoxelEngine::globals_t
{
    /* data */
};

void VoxelEngine::Setup(){
    VoxelEngine::config = {
        800,
        600,
        "VulkanVoxelRenderer",
        true
    };
    VoxelEngine::CreateWindow();
    VoxelEngine::octree->Setup(9);
    Camera::Properties camProperties = {
        .FOV = 90,
        .projection = Camera::PERSPECTIVE
    };
    VoxelEngine::camera->Setup(window, &stats, &camProperties, glm::vec3(0,0,0), glm::vec3(1,0,0));

    Object a(octree, glm::vec3(0,0,0), glm::vec3(0,0,0), glm::vec3(1,1,1));
    a.loadWavefrontObj("file", false, false);
    
    MaterialCollection::Material m1 = {
        /* data */
    };
    LightCollection::Light l1 = {
        /* data */
    };

    materials->add(&m1);
    lights->add(&l1);
    objects->add(&a, m1.id);
}