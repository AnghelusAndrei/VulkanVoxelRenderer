#pragma once

#include <GLFW/glfw3.h>
#include "config.hpp"

#include "vulkaninstance.hpp"
class Object;
class Octree;

class VulkanInstance;
class VoxelEngine
{
public:
    VoxelEngine(Config config);


    void run();
    
    ~VoxelEngine();
public:

    GLFWwindow *window;
    bool running_;
    //Stats stats;
    //Camera *camera;
    //Octree *octree;

    //MaterialCollection *materials;
    //LightCollection *lights;
    //ObjectCollection *objects;
    
private:
    VulkanInstance *instance_;
    int maxThreads_p;
    Config config_;

    static void framebuffer_resized(GLFWwindow *window_, int width, int height);
    static void window_maximized(GLFWwindow *window_, int maximized);
    
    friend class VulkanInstance;
};