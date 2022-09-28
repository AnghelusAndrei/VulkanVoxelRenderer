#pragma once

#include <GLFW/glfw3.h>
#include "config.hpp"
#include "stats.hpp"
#include "objectCollection.hpp"
#include "vulkaninstance.hpp"
class Object;
class Octree;
class Camera;
class MaterialCollection;
class LightCollection;
class ObjectCollection;
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

    MaterialCollection *materials;
    LightCollection *lights;
    ObjectCollection *objects;
    
private:
    VulkanInstance *instance_;
    int maxThreads_p;

    void renderT_();
    static void framebuffer_resized(GLFWwindow *window_, int width, int height);
    static void window_maximized(GLFWwindow *window_, int maximized);
    
    friend class VulkanInstance;
};