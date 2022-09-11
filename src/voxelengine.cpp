#include "voxelengine.hpp"

VoxelEngine::VoxelEngine()
{
    glfwInit();
    // We specify the window hint in order for GLFW to not
    // create by default a OpenGL context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    User = usr(this);

    window_ = glfwCreateWindow(config_.window_width, config_.window_height, config_.window_title.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window_, this);
    glfwSetFramebufferSizeCallback(window_, framebuffer_resized);
    instance_ = new VulkanInstance(this);
}
void VoxelEngine::run()
{
    while (!glfwWindowShouldClose(window_)) {

            glfwPollEvents();
            instance_->render();

            /** @todo multithreading Scene and Interactive **/
            User.Interactive();
            User.Scene();

            instance_->stats.Update();
        }
    instance_->base.device.waitIdle();
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