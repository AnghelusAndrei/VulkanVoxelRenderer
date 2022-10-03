#include "voxelengine.hpp"

VoxelEngine::VoxelEngine(Config config) : config_(config)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // We specify the window hint in order for GLFW to not create by default a OpenGL context

    
    window =glfwCreateWindow(config_.window_width, config_.window_height, config_.window_title.c_str(), nullptr, nullptr);

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
    running_=true;
    //std::thread inputThread();
    std::thread renderThread(&VulkanInstance::run, instance_);

    Octree *octree = new Octree(4);
    Octree::Node node;
    node.isNode=false;
    glm::u8vec3 rgb = glm::u8vec3(255, 0, 0);
    node.leaf.data=Octree::utils_rgb(rgb.r, rgb.g, rgb.b);

    octree->insert({1,0,0}, node);
    octree->insert({1,0,1}, node);
    octree->insert({1,1,0}, node);
    octree->insert({0,0,1}, node);
    octree->upload(instance_);

    while (!glfwWindowShouldClose(window))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    running_=false;
    renderThread.join();
    instance_->device_.waitIdle();
}
void VoxelEngine::framebuffer_resized(GLFWwindow *window_, int width, int height)
{
    auto engine = reinterpret_cast<VoxelEngine *>(glfwGetWindowUserPointer(window_));
    engine->config_.window_height = height;
    engine->config_.window_width = width;
}
void VoxelEngine::window_maximized(GLFWwindow *window_, int maximized)
{
    int width, height;
    glfwGetFramebufferSize(window_, &width, &height);
    auto engine = reinterpret_cast<VoxelEngine *>(glfwGetWindowUserPointer(window_));
    engine->config_.window_height = height;
    engine->config_.window_width = width;
}

VoxelEngine::~VoxelEngine()
{
    delete instance_;
    glfwDestroyWindow(window);
    glfwTerminate();
}