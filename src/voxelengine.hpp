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

public:
    GLFWwindow *window_;
    VulkanInstance *instance_;
    Config config_;
    usr User;

    Camera *camera;
    Octree octree;
private:
    static void framebuffer_resized(GLFWwindow *window, int width, int height);
    friend class VulkanInstance;
};