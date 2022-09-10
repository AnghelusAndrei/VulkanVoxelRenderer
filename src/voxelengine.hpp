#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include "config.hpp"
#include "vulkaninstance.hpp"
#include "../usr/usr.hpp"

class VulkanInstance;
class VoxelEngine
{
public:
    VoxelEngine(Config config);
    void run();
    ~VoxelEngine();

private:
    static void framebuffer_resized(GLFWwindow *window, int width, int height);
    Config config_;
    GLFWwindow *window_;
    VulkanInstance *instance_;
    friend class VulkanInstance;
};