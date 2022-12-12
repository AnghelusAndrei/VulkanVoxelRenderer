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

    int octreeLength = 1<<3;

    Octree *octree = new Octree(3);
    Camera *camera = new Camera(window, initialPosition, initialDirection);
    instance_->setCamera(camera);
    std::thread renderThread(&VulkanInstance::run, instance_);


    for(int i = 0; i < octreeLength; i++){
        for(int j = 0; j < octreeLength; j++){
            for(int k = 0; k < octreeLength; k++){
                int randIn5 = rand()%10;
                if(randIn5 != 1)continue;

                glm::u8vec3 rgb = glm::u8vec3((uint)(rand()%255), (uint)(rand()%255), (uint)(rand()%255));
                Octree::Leaf node;
                node.isNode = false;
                node.type = Octree::DEFAULT;
                node.data=Octree::utils_rgb(rgb.r, rgb.g, rgb.b);
                octree->insert(glm::uvec3(i,j,k), node);
            }
        }
    }
    
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