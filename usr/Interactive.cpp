#include "voxelengine.hpp"

void usr::Interactive(){
    Camera::keyLayout fpKeyLayout = {
                .forward = GLFW_KEY_W,
                .backward = GLFW_KEY_S,
                .left = GLFW_KEY_A,
                .right = GLFW_KEY_D,
                .up = GLFW_KEY_SPACE,
                .down = GLFW_KEY_LEFT_SHIFT,
                .x_axis_direction = Camera::MOUSE_X,
                .y_axis_direction = Camera::MOUSE_Y
    };
    engine_->camera->FirstPersonHandler(fpKeyLayout, 1, glm::vec3(0,1,0), 2);
}