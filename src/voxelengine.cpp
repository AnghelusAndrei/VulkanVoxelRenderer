#include "voxelengine.hpp"

VoxelEngine::VoxelEngine()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // We specify the window hint in order for GLFW to not create by default a OpenGL context

    octree = new Octree();
    camera = new Camera();

    Setup();

    if(!window)glfwCreateWindow(config.window_width, config.window_height, config.window_title.c_str(), nullptr, nullptr);
    maxThreads = std::thread::hardware_concurrency();
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebuffer_resized);

    instance_ = new VulkanInstance(this);
}
void VoxelEngine::CreateWindow(){
    window = glfwCreateWindow(config.window_width, config.window_height, config.window_title.c_str(), nullptr, nullptr);
}
void VoxelEngine::run()
{
    while (!glfwWindowShouldClose(window)) {

#ifdef MULTITHREADED
            std::thread UserInteractive(&VoxelEngine::Interactive, this);
            std::thread UserScene(&VoxelEngine::Scene, this);

            glfwPollEvents();
            instance_->render();

            UserInteractive.join();
            UserScene.join();
#else

            glfwPollEvents();
            instance_->render();

            Interactive();
            Scene();
#endif

            stats.Update();
        }
    instance_->device_.waitIdle();
}
void VoxelEngine::framebuffer_resized(GLFWwindow* window_, int width, int height)
{
    auto engine=reinterpret_cast<VoxelEngine*>(glfwGetWindowUserPointer(window_));
}

VoxelEngine::~VoxelEngine()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}