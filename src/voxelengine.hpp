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

    void Setup();
    void Interactive();
    void Scene();
    void CreateWindow();

    void run();
    
    ~VoxelEngine();

public:
    GLFWwindow *window;
    Config config;
    Stats stats;

    Camera *camera;
    Octree *octree;
    
private:
    VulkanInstance *instance_;
    int maxThreads;

    static void framebuffer_resized(GLFWwindow *window_, int width, int height);
    friend class VulkanInstance;
};