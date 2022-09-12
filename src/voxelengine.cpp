#include "voxelengine.hpp"

VoxelEngine::VoxelEngine() : octree(), camera()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // We specify the window hint in order for GLFW to not create by default a OpenGL context

    Setup();
    maxThreads = std::thread::hardware_concurrency();

    window = glfwCreateWindow(config.window_width, config.window_height, config.window_title.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebuffer_resized);
    instance_ = new VulkanInstance(this);
}
void VoxelEngine::run()
{
    while (!glfwWindowShouldClose(window)) {

#ifdef MULTITHREADED
            std::thread UserInteractive(Interactive);
            std::thread UserScene(Scene);

            glfwPollEvents();
            instance_->render();

            UserInteractive.join();
            UserScene.join();
#else
            Interactive();
            Scene();

            glfwPollEvents();
            instance_->render();
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