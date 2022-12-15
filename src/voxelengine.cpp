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
  
  	const siv::PerlinNoise::seed_type seed = 123456u;
	const siv::PerlinNoise perlin{ seed };


    glm::vec3 initialPosition = glm::vec3(0,0,0);
    glm::vec3 initialDirection = glm::vec3(1,0,0);

    int octreeLength = 1<<8;

    Octree *octree = new Octree(8);
    Camera *camera = new Camera(window, initialPosition, initialDirection);
    instance_->setCamera(camera);
    std::thread renderThread(&VulkanInstance::run, instance_);


    for(int i = 0; i < octreeLength; i++){
        for(int j = 0; j < octreeLength; j++){
            for(int k = 0; k < octreeLength; k++){
                float randIn5 = perlin.octave3D_01(((double)i * 0.01), ((double)j * 0.01), ((double)k * 0.01), 2);
                if(randIn5 < 0.5)continue;

                int r = 255 * perlin.octave3D_01(((double)i * 0.02), ((double)j * 0.01), ((double)k * 0.01), 3);
                int g = 255 * perlin.octave3D_01(((double)i * 0.01), ((double)j * 0.02), ((double)k * 0.01), 3);
                int b = 255 * perlin.octave3D_01(((double)i * 0.01), ((double)j * 0.01), ((double)k * 0.02), 3);

                glm::u8vec3 rgb = glm::u8vec3((uint)(r), (uint)(g), (uint)(b));
                Octree::Leaf node;
                node.isNode = false;
                node.type = Octree::DEFAULT;
                node.data=Octree::utils_rgb(rgb.r, rgb.g, rgb.b);
                octree->insert(glm::uvec3(i,j,k), node);
            }
        }
    }
    
    octree->upload(instance_);
    
    while (!glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_ESCAPE)!= GLFW_PRESS)
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