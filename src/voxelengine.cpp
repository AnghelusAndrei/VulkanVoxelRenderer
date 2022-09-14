#include "voxelengine.hpp"

VoxelEngine::VoxelEngine()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // We specify the window hint in order for GLFW to not create by default a OpenGL context

    octree = new Octree();
    camera = new Camera();
    lightCollection = new LightCollection();
    materialCollection = new MaterialCollection();

    Setup();

    if(!window)glfwCreateWindow(config.window_width, config.window_height, config.window_title.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebuffer_resized);

    instance_p = new VulkanInstance(this);

    maxThreads_p = std::thread::hardware_concurrency();
    LOGGING->verbose() << "Found "<<maxThreads_p<<" threads on the CPU" << std::endl;
#ifdef MULTITHREADED
    LOGGING->verbose() << "Running using multithreading" << std::endl;
#else
    LOGGING->verbose() << "Running without multithreading" << std::endl;
#endif

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
            instance_p->render();

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
    instance_p->device_.waitIdle();
}
void VoxelEngine::framebuffer_resized(GLFWwindow* window_, int width, int height)
{
    auto engine=reinterpret_cast<VoxelEngine*>(glfwGetWindowUserPointer(window_));
}

VoxelEngine::~VoxelEngine()
{
    glfwDestroyWindow(window);
    glfwTerminate();
    delete materialCollection;
    delete lightCollection;
    delete octree;
    delete camera;
    delete instance_p;
}