#include "voxelengine.hpp"

void VoxelEngine::Setup(){
    VoxelEngine::config = {
        800,
        600,
        "VulkanVoxelRenderer",
        true
    };
    VoxelEngine::octree = Octree(9);
    Camera::Properties camProperties = {
        .FOV = 90,
        .projection = Camera::PERSPECTIVE
    };
    VoxelEngine::camera = &Camera(VoxelEngine::window, glm::vec3(0,0,0), glm::vec3(1,0,0), camProperties, VoxelEngine::stats);
}