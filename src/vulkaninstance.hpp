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

#include "voxelengine.hpp"
#include "logging.hpp"

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
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createImageViews();
    void createDescriptorSetLayout();
    void createComputePipeline();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();
private:
    VoxelEngine *engine_;
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> computeFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete()
        {
            return graphicsFamily.has_value() && presentFamily.has_value()&&computeFamily.has_value();
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
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
    void recordImageBarrier(vk::CommandBuffer buffer, vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
    vk::AccessFlags scrAccess, vk::AccessFlags dstAccess, vk::PipelineStageFlags srcBind, vk::PipelineStageFlags dstBind);
    vk::ShaderModule createShaderModule(const std::vector& code);
    static std::vector readFile(const std::string& filename);

    const std::vector<const char *> validationLayers = {
        "VK_LAYER_KHRONOS_validation"};

    const std::vector<const char *> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    #ifdef NDEBUG
        const bool enableValidationLayers = false;
    #else
        const bool enableValidationLayers = true;
    #endif

    vk::Instance instance_;
    vk::DebugUtilsMessengerEXT debug_messenger_;
    vk::SurfaceKHR surface_;
    vk::DispatchLoaderDynamic dldy_;
    vk::PhysicalDevice physical_device_;
    vk::Device device_;
    vk::DescriptorSetLayout descriptorSetLayout;
    vk::Sampler imageSampler;
    vk::Pipeline pipeline;

    vk::Queue graphicsQueue;
    vk::Queue presentQueue;
    vk::Queue computeQueue;

    vk::SwapchainKHR swapChain;
    std::vector<vk::Image> swapChainImages;
    vk::Format swapChainImageFormat;
    vk::Extent2D swapChainExtent;
    std::vector<vk::ImageView> swapChainImageViews;
    std::vector<vk::Framebuffer> swapChainFramebuffers;

    vk::RenderPass renderPass;
    vk::PipelineLayout pipelineLayout;
    vk::Pipeline graphicsPipeline;

    vk::CommandPool commandPool;
    std::vector<vk::CommandBuffer> commandBuffers;

    std::vector<vk::Semaphore> imageAvailableSemaphores;
    std::vector<vk::Semaphore> renderFinishedSemaphores;
    std::vector<vk::Fence> inFlightFences;
    uint32_t currentFrame = 0;

    bool framebufferResized = false;



    friend class VoxelEngine;
};


