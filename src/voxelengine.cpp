#include "voxelengine.hpp"
#include "camera.hpp"

VoxelEngine::VoxelEngine(Config config) : config_(config)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // We specify the window hint in order for GLFW to not create by default a OpenGL context

    
    window =glfwCreateWindow(config_.window_width, config_.window_height, config_.window_title.c_str(), nullptr, nullptr);

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebuffer_resized);
    glfwSetWindowMaximizeCallback(window, window_maximized);
    instance_ = new VulkanInstance(this);
}

void VoxelEngine::run()
{
    running_=true;
  


    glm::vec3 initialPosition = glm::vec3(0,0,0);
    glm::vec3 initialDirection = glm::vec3(1,0,0);

    Octree *octree = new Octree(3);
    Camera *camera = new Camera(window, initialPosition, initialDirection);
    instance_->setCamera(camera);
    std::thread renderThread(&VulkanInstance::run, instance_);
    Octree::Node node;
    node.isNode=false;
    node.leaf.type = Octree::DEFAULT;
    glm::u8vec3 rgb = glm::u8vec3(255, 255, 255);
    node.leaf.data=Octree::utils_rgb(rgb.r, rgb.g, rgb.b);

    octree->insert({2,0,0}, node);
    
    octree->upload(instance_);
    
    while (!glfwWindowShouldClose(window))
    {
        camera->input();
        glfwPollEvents();
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