#pragma once


#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>
#include <array>
#include <map>
#include <stack>
#include <thread>
#include <mutex>
#include <strstream>

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <GLFW/glfw3.h>

#include "config.hpp"
#include "voxelengine.hpp"
#include "logging.hpp"
#include "octree.hpp"
#include "camera.hpp"
#include "stats.hpp"
#include "object.hpp"
#include "materials.hpp"
#include "lights.hpp"
#include "objectCollection.hpp"
#include "utils.hpp"

#define MULTITHREADED

/**
 * @brief Represents all rendering infrastructure
 * 
 */
class VoxelEngine;
class VulkanInstance {
public:
    VulkanInstance(VoxelEngine *engine);
    void render();
    ~VulkanInstance();

private:

    struct VmaBuffer {
        vk::Buffer buffer;
        VmaAllocation allocation;
    };

    struct QueueSupportDetails {
        std::optional<uint32_t> computeFamily;
        std::optional<uint32_t> presentFamily;
        bool hasValues()
        {
            return computeFamily.has_value() && presentFamily.has_value();   
        }
    };
    struct SwapChainSupportDetails
    {
        vk::SurfaceCapabilitiesKHR capabilities;
        vk::SurfaceFormatKHR format = vk::Format::eUndefined;
        vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
        vk::Extent2D extent;
    };

    struct RaycastSpecialization 
    {
        uint32_t window_width, window_height;
    };
    
    struct LightingSpecialization 
    {
        
    };

    struct RenderSpecialization 
    {
        
    };
    struct SpecializationConstants 
    {
        RaycastSpecialization raycast;
        LightingSpecialization lighting;
        RenderSpecialization render;
    };
    VoxelEngine *engine_;
    vk::Instance instance_;
    vk::DispatchLoaderDynamic dispatch_;
    vk::DebugUtilsMessengerEXT debugMessenger_;
    vk::PhysicalDevice physicalDevice_;
    vk::SurfaceKHR surface_;
    vk::Device device_;
    vk::Queue presentQueue_, computeQueue_;
    VmaAllocator allocator_;
    vk::CommandPool commandPool_;
    vk::DescriptorPool raycastPool_, lightingPool_, renderPool_;
    VmaBuffer stagingBuffer_, octreeBuffer_, lightingBuffer_;
    std::vector<vk::Semaphore> imageAvailableSemaphores_, renderFinishedSemaphores_;
    std::vector<vk::Fence> inFlightFences_, imagesInFlightFences_;
    size_t currentFrame_=0;

    vk::SwapchainKHR swapChain_;
    std::vector<vk::Image> images_;
    std::vector<vk::ImageView> imageViews_;
    vk::Sampler imageSampler_;
    vk::DescriptorSetLayout raycastSetLayout_, lightingSetLayout_, renderSetLayout_;
    std::vector<vk::DescriptorSet> raycastDescriptorSets_, lightingDescriptorSets_, renderDescriptorSets_;  
    vk::PipelineLayout raycastPipelineLayout_, lightingPipelineLayout_, renderPipelineLayout_;
    vk::Pipeline raycastPipeline_, lightingPipeline_, renderPipeline_;
    std::vector<vk::CommandBuffer> commandBuffers_, copyCommandBuffers_; 
    std::vector<std::vector<vk::DescriptorSet>> jointDescriptorSets_;
    void createInstance();
    void selectPhysicalDevice(); 
    void createPermanentObjects();
    void createSwapchainObjects();
    void setupFrameObjects();


    QueueSupportDetails utils_getQueueSupportDetails();
    vk::DescriptorPool utils_createDescriptorPool(std::vector<vk::DescriptorType> descriptorTypes);
    VmaBuffer utils_createBuffer(vk::DeviceSize size, vk::BufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags);
    SwapChainSupportDetails utils_getSwapChainSupportDetails();
    vk::DescriptorSetLayout utils_createDescriptorSetLayout(std::vector<vk::DescriptorSetLayoutBinding> bindings);
    std::vector<vk::DescriptorSet> utils_allocateDescriptorSets(vk::DescriptorPool pool, vk::DescriptorSetLayout layout);
    vk::ShaderModule utils_createShaderModule(std::string path);
    const std::vector<const char *> utils_validationLayers = {
        "VK_LAYER_KHRONOS_validation"};
    const std::vector<const char*> utils_deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    friend class VoxelEngine;
};  