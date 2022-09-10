#include "voxelengine.hpp"

VoxelEngine::VoxelEngine(Config config) : config_(config)
{

    glfwInit();
    // We specify the window hint in order for GLFW to not
    // create by default a OpenGL context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window_ = glfwCreateWindow(config_.window_width, config_.window_height, config.window_title.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window_, this);
    glfwSetFramebufferSizeCallback(window_, framebuffer_resized);
    instance_ = new VulkanInstance(this);
}
void VoxelEngine::run()
{
    while (!glfwWindowShouldClose(window_)) {
            glfwPollEvents();
            instance_->render();
        }
    instance_->device_.waitIdle();
}
void VoxelEngine::framebuffer_resized(GLFWwindow* window, int width, int height)
{
    auto engine=reinterpret_cast<VoxelEngine*>(glfwGetWindowUserPointer(window));
}

VoxelEngine::~VoxelEngine()
{
    glfwDestroyWindow(window_);
    glfwTerminate();
}