#include "voxelengine.hpp"

VoxelEngine::VoxelEngine()
{
    glfwInit();
    // We specify the window hint in order for GLFW to not
    // create by default a OpenGL context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    Setup();

    window = glfwCreateWindow(config.window_width, config.window_height, config.window_title.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebuffer_resized);
    instance_ = new VulkanInstance(this);
}
void VoxelEngine::run()
{
    while (!glfwWindowShouldClose(window)) {

            glfwPollEvents();
            instance_->render();

            /** @todo multithreading Scene and Interactive **/
            Interactive();
            Scene();

            stats.Update();
        }
    instance_->device_.waitIdle();
}
void VoxelEngine::framebuffer_resized(GLFWwindow* window, int width, int height)
{
    auto engine=reinterpret_cast<VoxelEngine*>(glfwGetWindowUserPointer(window));
}

VoxelEngine::~VoxelEngine()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}