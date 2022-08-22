#include "vulkaninstance.hpp"

VulkanInstance::VulkanInstance(VoxelEngine *engine) : engine_(engine)
{

    createInstance();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    /*createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandPool();
    createCommandBuffers();
    createSyncObjects();*/
}

VulkanInstance::~VulkanInstance()
{
    // cleanupSwapChain();

    vkDestroyPipeline(device_, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device_, pipelineLayout, nullptr);

    vkDestroyRenderPass(device_, renderPass, nullptr);

    for (size_t i = 0; i < engine_->config_.MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(device_, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device_, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device_, inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(device_, commandPool, nullptr);

    vkDestroyDevice(device_, nullptr);

    instance_.destroyDebugUtilsMessengerEXT(debug_messenger_, vk::Optional<const vk::AllocationCallbacks>(nullptr), dldy_);
    instance_.destroySurfaceKHR(surface_, nullptr);
    vkDestroyInstance(instance_, nullptr);
}
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
{
    switch (messageSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        LOGGING->verbose() << pCallbackData->pMessage << std::endl;
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        LOGGING->info() << pCallbackData->pMessage << std::endl;
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        LOGGING->warn() << pCallbackData->pMessage << std::endl;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        LOGGING->error() << pCallbackData->pMessage << std::endl;
    }

    return VK_FALSE;
}
void VulkanInstance::createInstance()
{
    vk::ApplicationInfo AppInfo{
        engine_->config_.window_title.c_str(), // Application Name
        1,                                     // Application Version
        nullptr,                               // Engine Name or nullptr
        0,                                     // Engine Version
        VK_API_VERSION_1_1                     // Vulkan API version
    };
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    const std::vector<const char *> validation_layers = {"VK_LAYER_KHRONOS_validation"};
    vk::InstanceCreateInfo InstanceCreateInfo(vk::InstanceCreateFlags(), // Flags
                                              &AppInfo,                  // Application Info
                                              validation_layers.size(),  // Layers count
                                              validation_layers.data(),
                                              extensions.size(),
                                              extensions.data()); // Layers

    instance_ = vk::createInstance(InstanceCreateInfo);
    LOGGING->verbose() << "Created instance\n";
    dldy_ = vk::DispatchLoaderDynamic(instance_, vkGetInstanceProcAddr);
    vk::DebugUtilsMessengerCreateInfoEXT debugInfo;
    debugInfo.messageSeverity =
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
    debugInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
    debugInfo.pfnUserCallback = debugCallback;
    debug_messenger_ = instance_.createDebugUtilsMessengerEXT(debugInfo, nullptr, dldy_);
    LOGGING->verbose() << "Created debug messenger\n";
}

void VulkanInstance::createSurface()
{
    VkSurfaceKHR surface;

    if (glfwCreateWindowSurface(instance_, engine_->window_, nullptr, &surface) != VK_SUCCESS)
    {
        throw EXCEPTION("Failed to create window surface!");
    }
    LOGGING->verbose() << "Created window surface\n";
    surface_ = vk::SurfaceKHR(surface);
}

void VulkanInstance::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;

    instance_.enumeratePhysicalDevices(&deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw EXCEPTION("Failed to find GPUs with Vulkan support");
    }

    std::vector<vk::PhysicalDevice> devices = instance_.enumeratePhysicalDevices();
    std::multimap<int, vk::PhysicalDevice> devicesmap;
        LOGGING->verbose()<<"Available devices: \n";
    for (const auto &device : devices)
    {
        int score = 1;
        LOGGING->print() << device.getProperties().deviceName << (device.getProperties().deviceType==vk::PhysicalDeviceType::eDiscreteGpu)<<'\n';
        if(device.getProperties().deviceType==vk::PhysicalDeviceType::eDiscreteGpu) score=1000;
        devicesmap.insert(std::make_pair(score, device));
    }
    
    physical_device_ = devicesmap.rbegin()->second;
    LOGGING->verbose() << "Selected device:  "<<physical_device_.getProperties().deviceName<<'\n';
}

VulkanInstance::QueueFamilyIndices VulkanInstance::findQueueFamilies()
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    std::vector<vk::QueueFamilyProperties> queueFamilies = physical_device_.getQueueFamilyProperties();
    int i = 0;
    for (const auto &queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags && VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }
        if (queueFamily.queueFlags && VK_QUEUE_COMPUTE_BIT)
        {
            indices.computeFamily = i;
        }

        vk::Bool32 presentSupport = false;
        if (physical_device_.getSurfaceSupportKHR(i, surface_))
        {
            indices.presentFamily = i;
        }

        if (indices.isComplete())
        {
            break;
        }

        i++;
    }

    return indices;
}

void VulkanInstance::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies();

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        vk::DeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    vk::PhysicalDeviceFeatures deviceFeatures{};

    vk::DeviceCreateInfo createInfo{};

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (engine_->config_.debugging_enabled)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }
    device_ =physical_device_.createDevice(createInfo);

    device_.getQueue(indices.graphicsFamily.value(), 0, &graphicsQueue);
    device_.getQueue(indices.presentFamily.value(), 0, &presentQueue);
    device_.getQueue(indices.computeFamily.value(), 0, &computeQueue);
}

VulkanInstance::SwapChainSupportDetails VulkanInstance::querySwapChainSupport() {
    SwapChainSupportDetails details;

    physical_device_.getSurfaceCapabilitiesKHR(surface_, &details.capabilities);

    uint32_t formatCount;
    physical_device_.getSurfaceFormatsKHR(surface_, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        physical_device_.getSurfaceFormatsKHR(surface_, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    physical_device_.getSurfacePresentModesKHR(surface_, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        physical_device_.getSurfacePresentModesKHR(surface_, &presentModeCount, details.presentModes.data());
    }

    return details;
}


    vk::SurfaceFormatKHR VulkanInstance::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear && 
            (physical_device_.getFormatProperties(availableFormat.format).optimalTilingFeatures&(vk::FormatFeatureFlagBits::eStorageImage|vk::FormatFeatureFlagBits::eColorAttachment))==(vk::FormatFeatureFlagBits::eStorageImage|vk::FormatFeatureFlagBits::eColorAttachment)) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    vk::PresentModeKHR VulkanInstance::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
                return availablePresentMode;
            }
        }

        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D VulkanInstance::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(engine_->window_, &engine_->config_.window_width, &engine_->config_.window_height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

void VulkanInstance::createSwapChain(){
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport();

        vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        vk::SwapchainCreateInfoKHR createInfo{};
        createInfo.surface = surface_;

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment|vk::ImageUsageFlagBits::eStorage;

        QueueFamilyIndices indices = findQueueFamilies();
        uint32_t queueFamilyIndices[] = {indices.computeFamily.value(), indices.presentFamily.value()};

        if (indices.computeFamily != indices.presentFamily) {
            createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        if (device_.createSwapchainKHR(&createInfo, nullptr, &swapChain) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create swap chain!");
        }

        device_.getSwapchainImagesKHR(swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        device_.getSwapchainImagesKHR(swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
}