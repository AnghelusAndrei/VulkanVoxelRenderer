#ifndef CAMERA_HPP
#define CAMERA_HPP 1

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <glm/vec3.hpp>

#include "vulkaninstance.hpp"


class Camera{
    public:
        struct Properties{
            uint8_t FOV = 90;
            uint8_t projection = PERSPECTIVE;
        };

        Camera(Properties &p);
        void FirstPersonHandler();

    public:
        glm::vec3 position;
        glm::vec3 direction;

        enum Enums{
            PERSPECTIVE = 0,
            ORTHOGRAPHIC = 1
        };

    private:
        Properties p_;

};

#endif