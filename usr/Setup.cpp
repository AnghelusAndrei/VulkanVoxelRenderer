#include "voxelengine.hpp"

usr::usr(VoxelEngine *engine) : engine_(engine){
    engine_->config_ = {
        800,
        600,
        "VulkanVoxelRenderer",
        true
    };
    engine_->octree = Octree(9);
    Camera::Properties camProperties = {
        .FOV = 90,
        .projection = Camera::PERSPECTIVE
    };
    engine_->camera = &Camera(engine_->window_, glm::vec3(0,0,0), glm::vec3(1,0,0), camProperties, engine_->instance_->stats);
}