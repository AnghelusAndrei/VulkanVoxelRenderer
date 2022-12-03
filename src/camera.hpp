#pragma once

#include <mutex>
#include "vulkaninstance.hpp"
#include "settings.hpp"
struct Stats;
class VulkanInstance;
struct CameraUBO;
class Camera{
    public:
        Camera(GLFWwindow *window_, glm::vec3 position_, glm::vec3 direction_);
        void input();
        CameraUBO getUBO();
         
    public:
        glm::vec3 position = glm::vec3(0,0,0);
        glm::vec3 direction = glm::vec3(1,0,0);

        enum keyEnum{
            MOUSE_X = 0,
            MOUSE_Y = 1
        };

    private:
        GLFWwindow *window_;
        double speed;
        double lastTime_;
        glm::dvec2 mousePosition_;
        glm::dvec2 rotation_;
        std::mutex uboMutex_;
};
