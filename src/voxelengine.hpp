#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include "config.hpp"
#include "vulkaninstance.hpp"

class VulkanInstance;
class VoxelEngine
{
public:
    VoxelEngine();
    void run();
    ~VoxelEngine();

    void Setup();
    void Interactive();
    void Scene();

public:
    GLFWwindow *window;
    Config config;

    Camera *camera;
    Octree octree;
public:
    struct Stats{
        double FPS;
        double MS;

        double time1, time2 = glfwGetTime();
        void Update(){
            time1 = time2;
            time2 = glfwGetTime();

            MS = time2-time1;
            FPS = 1000/MS;
        }
    } stats;
private:
    VulkanInstance *instance_;

    static void framebuffer_resized(GLFWwindow *window_, int width, int height);
    friend class VulkanInstance;
};