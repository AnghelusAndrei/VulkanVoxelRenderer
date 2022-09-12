#ifndef CAMERA_HPP
#define CAMERA_HPP 1

#include "vulkaninstance.hpp"


class Camera{
    public:
        struct Properties{
            uint8_t FOV = 90;
            uint8_t projection = PERSPECTIVE;
        };

        struct keyLayout{
            uint16_t forward;
            uint16_t backward;
            uint16_t left;
            uint16_t right;
            uint16_t up;
            uint16_t down;
            uint16_t x_axis_direction;
            uint16_t y_axis_direction;
        };

        Camera(GLFWwindow *window, glm::vec3 position, glm::vec3 direction, Properties &p, VoxelEngine::Stats &stats);
        void FirstPersonHandler(keyLayout &layout, float speed, glm::vec3 up, double Sensitivity);

    public:
        glm::vec3 position = glm::vec3(0,0,0);
        glm::vec3 direction = glm::vec3(1,0,0);
        Properties properties;

        enum ProjectionEnums{
            PERSPECTIVE = 0,
            ORTHOGRAPHIC = 1
        };

        enum keyEnum{
            MOUSE_X = 0,
            MOUSE_Y = 1
        };

    private:
        GLFWwindow *window_;
        VoxelEngine::Stats &stats_;

        glm::dvec2 mousePosition;
        glm::dvec2 rotation;
};

#endif