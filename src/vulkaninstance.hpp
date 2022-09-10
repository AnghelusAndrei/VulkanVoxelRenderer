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

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include <glm/vec2.hpp>

#include "voxelengine.hpp"
#include "logging.hpp"

struct Buffer
{
    VmaAllocation allocation;
    vk::Buffer buffer;
};

class VoxelEngine;
class VulkanInstance
{
public:
    VulkanInstance(VoxelEngine *engine);
    void render();
    ~VulkanInstance();

public:
private:
    void createInstance();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createResources();
    void createFrameResources();
    void createSurface();
    void createSwapChain();
    void createImageViews();
    void createBuffer();
    void createDescriptorSetLayout();
    void createComputePipeline();
    void createCommandPool();
    void createDescriptorPool();
    void createDescriptorSets();
    void createCommandBuffers();
    void createSyncObjects();
    void recreateSwapChain();

private:
    VmaAllocator allocator_;

    VoxelEngine *engine_;
    Buffer staging_buffer_;
    Buffer local_buffer_;
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> computeFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete()
        {
            return presentFamily.has_value() && computeFamily.has_value();
        }
    };

    struct SwapChainSupportDetails
    {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

    QueueFamilyIndices findQueueFamilies();
    SwapChainSupportDetails querySwapChainSupport();
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats);
    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);
    void recordImageBarrier(vk::CommandBuffer buffer, vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
                            vk::AccessFlags scrAccess, vk::AccessFlags dstAccess, vk::PipelineStageFlags srcBind, vk::PipelineStageFlags dstBind);
    vk::ShaderModule createShaderModule(const std::vector<char> &code);
    static std::vector<char> readFile(const std::string &filename);

    const std::vector<const char *> validationLayers = {
        "VK_LAYER_KHRONOS_validation"};

    const std::vector<const char *> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
    

    vk::SwapchainKHR swapChain;
    std::vector<vk::Image> swapChainImages;
    vk::Format swapChainImageFormat;
    vk::Extent2D swapChainExtent;
    std::vector<vk::ImageView> swapChainImageViews;

    vk::PipelineLayout pipelineLayout;
    vk::Pipeline graphicsPipeline;

    std::vector<vk::CommandBuffer> commandBuffers;
    vk::Buffer buffer_;
    std::vector<vk::Semaphore> imageAvailableSemaphores;
    std::vector<vk::Semaphore> renderFinishedSemaphores;
    std::vector<vk::Fence> inFlightFences;
    std::vector<vk::Fence> imagesInFlight;
    uint32_t currentFrame = 0;
    void cleanupSwapChain();
    bool framebufferResized = false;
    static const uint32_t maxFrames = 2;
    struct VulkanBase
    {
        vk::Instance instance;
        vk::DispatchLoaderDynamic dispatch;
        vk::DebugUtilsMessengerEXT debugMessenger;
        vk::PhysicalDevice physicalDevice;
        vk::Device device;
        vk::SurfaceKHR surface;
        vk::SwapchainKHR swapchain;
        vk::Queue presentQueue;
        vk::Queue computeQueue;

        vk::Pipeline pipelines[3];
        vk::CommandPool commandPool;
        vk::DescriptorPool descriptorPools[3];
        Buffer stagingBuffer;
        Buffer octreeBuffer;
        Buffer screenBuffer;
        Buffer voxelApparitionBuffer;
        Buffer voxelIlluminationBuffer;
    } base;

    
    struct FrameResources
    {
        vk::Image image;
        vk::ImageView imageView;
        vk::DescriptorSet descriptorSets[3];

        vk::CommandBuffer commandBuffer;
        vk::CommandBuffer copyCommandBuffer;
        // Synchronization
        vk::Semaphore imageAvailable;
        vk::Fence inFlight;
        vk::Fence imageInFlight;
        vk::Semaphore imageRenderFinished;
    } frameResources[maxFrames];


    /**
     *  OCTREE BUFFER -> FIRST STEP -> VOXEL BUFFER; SCREEN BUFFER -> SECOND STEP -> THIRD STEP 
     * 
     * 
     */
    friend class VoxelEngine;
};