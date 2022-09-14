#pragma once

#include <GLFW/glfw3.h>
#include "config.hpp"
#include "stats.hpp"
#include "vulkaninstance.hpp"

class Object;
class Octree;
class Camera;
class MaterialCollection;
class LightCollection;
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
    MaterialCollection *materialCollection;
    LightCollection *lightCollection;
    std::vector<Object*> objects;
    
private:
    VulkanInstance *instance_p;
    int maxThreads_p;

    static void framebuffer_resized(GLFWwindow *window_, int width, int height);
    friend class VulkanInstance;
};