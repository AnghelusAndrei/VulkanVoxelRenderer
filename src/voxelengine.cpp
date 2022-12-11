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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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
    Octree::Leaf node1,node2;
    node1.isNode=false;
    node1.type = Octree::DEFAULT;
    node2.isNode=false;
    node2.type = Octree::DEFAULT;
    glm::u8vec3 rgb = glm::u8vec3(255, 255, 255);
    node1.data=Octree::utils_rgb(rgb.r, rgb.g, rgb.b);
    rgb = glm::u8vec3(255, 94, 5);
    node2.data=Octree::utils_rgb(rgb.r, rgb.g, rgb.b);

    octree->insert(glm::uvec3(0,0,0), node1); /// bitte stfu
    octree->insert(glm::uvec3(1,1,1), node2);
    
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

    if(maximized) {
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }else {
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

VoxelEngine::~VoxelEngine()
{
    delete instance_;
    glfwDestroyWindow(window);
    glfwTerminate();
}