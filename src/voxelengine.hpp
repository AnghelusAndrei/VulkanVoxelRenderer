#pragma once

#include <GLFW/glfw3.h>
#include "config.hpp"
#include "stats.hpp"
#include "vulkaninstance.hpp"

class Octree;
class Camera;
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
    Stats stats;

    Camera *camera;
    Octree *octree;
public:

private:
    VulkanInstance *instance_;
    uint8_t maxThreads;

    static void framebuffer_resized(GLFWwindow *window_, int width, int height);
    friend class VulkanInstance;
};