#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "vulkaninstance.hpp"

struct Stats;
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

        void Setup(GLFWwindow *window_, Stats *stats_, Properties *properties_, glm::vec3 position_, glm::vec3 direction_);
        void FirstPersonHandler(const keyLayout &layout, float speed, glm::vec3 up, double Sensitivity);

    public:
        glm::vec3 position = glm::vec3(0,0,0);
        glm::vec3 direction = glm::vec3(1,0,0);
        Properties *properties;

        enum ProjectionEnums{
            PERSPECTIVE = 0,
            ORTHOGRAPHIC = 1
        };

        enum keyEnum{
            MOUSE_X = 0,
            MOUSE_Y = 1
        };

    private:
        GLFWwindow *window_p;
        Stats *stats_p;

        glm::dvec2 mousePosition_p;
        glm::dvec2 rotation_p;
};

#endif