#include "voxelengine.hpp"

VoxelEngine::VoxelEngine() : octree(new Octree), camera(new Camera), lights(new LightCollection), materials(new MaterialCollection), objects(new ObjectCollection)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // We specify the window hint in order for GLFW to not create by default a OpenGL context

    
    glfwCreateWindow(config.window_width, config.window_height, config.window_title.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebuffer_resized);
    glfwSetWindowMaximizeCallback(window, window_maximized);
    instance_ = new VulkanInstance(this);

    maxThreads_p = std::thread::hardware_concurrency();
    LOGGING->verbose() << "Found " << maxThreads_p << " threads on the CPU" << std::endl;
#ifdef MULTITHREADED
    LOGGING->verbose() << "Running using multithreading" << std::endl;
#else
    LOGGING->verbose() << "Running without multithreading" << std::endl;
#endif
}

void VoxelEngine::run()
{
    
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        instance_->render();
    }
    instance_->device_.waitIdle();
}
void VoxelEngine::framebuffer_resized(GLFWwindow *window_, int width, int height)
{
    auto engine = reinterpret_cast<VoxelEngine *>(glfwGetWindowUserPointer(window_));
    engine->config.window_height = height;
    engine->config.window_width = width;
}
void VoxelEngine::window_maximized(GLFWwindow *window_, int maximized)
{
    int width, height;
    glfwGetFramebufferSize(window_, &width, &height);
    auto engine = reinterpret_cast<VoxelEngine *>(glfwGetWindowUserPointer(window_));
    engine->config.window_height = height;
    engine->config.window_width = width;
}

VoxelEngine::~VoxelEngine()
{
    delete materials;
    delete lights;
    delete objects;
    delete octree;
    delete camera;
    glfwDestroyWindow(window);
    glfwTerminate();
}