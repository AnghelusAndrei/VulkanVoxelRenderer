#include "vulkaninstance.hpp"

VulkanInstance::VulkanInstance(VoxelEngine *engine) : engine_(engine)
{
    createInstance();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createDescriptorSetLayout();
    createComputePipeline();
    createCommandPool();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
    createSyncObjects();
}

VulkanInstance::~VulkanInstance()
{
    // cleanupSwapChain();

    device_.destroyPipeline(graphicsPipeline, nullptr);
    device_.destroyPipelineLayout(pipelineLayout, nullptr);

    device_.destroyRenderPass(renderPass, nullptr);

    for (size_t i = 0; i < engine_->config_.MAX_FRAMES_IN_FLIGHT; i++)
    {
        device_.destroySemaphore(renderFinishedSemaphores[i], nullptr);
        device_.destroySemaphore(imageAvailableSemaphores[i], nullptr);
        device_.destroyFence(inFlightFences[i], nullptr);
    }

    device_.destroyCommandPool(commandPool, nullptr);
    device_.destroy(nullptr);

    instance_.destroyDebugUtilsMessengerEXT(debug_messenger_, vk::Optional<const vk::AllocationCallbacks>(nullptr), dldy_);
    instance_.destroySurfaceKHR(surface_, nullptr);
    instance_.destroy(nullptr);
}
VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
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
/**
 * @brief Creates the base vulkan instance and sets up extensions
 * and debugging
 */
void VulkanInstance::createInstance()
{
    vk::ApplicationInfo app_info{
        engine_->config_.window_title.c_str(), // Application Name
        1,                                     // Application Version
        nullptr,                               // Engine Name or nullptr
        0,                                     // Engine Version
        VK_API_VERSION_1_1                     // Vulkan API version
    };

    // Get required extensions for displaying to a window
    uint32_t glfw_extension_count = 0;
    const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    std::vector<const char *> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
    /** @todo Check for <obviously> always supported debug extensions and validation layer **/

    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    const std::vector<const char *> validation_layers = {"VK_LAYER_KHRONOS_validation"};
    vk::InstanceCreateInfo instance_create_info(vk::InstanceCreateFlags(), // Flags
                                                &app_info,                 // Application Info
                                                validation_layers.size(),  // Layers count
                                                validation_layers.data(),  // Layers
                                                extensions.size(),         // Extensions count
                                                extensions.data());        // Extensions

    instance_ = vk::createInstance(instance_create_info);

    LOGGING->verbose() << "Created instance" << std::endl;

    // Dynamic dispatch loader is needed since debug messenger is part of a extension
    dldy_ = vk::DispatchLoaderDynamic(instance_, vkGetInstanceProcAddr);
    vk::DebugUtilsMessengerCreateInfoEXT debug_info{
        vk::DebugUtilsMessengerCreateFlagsEXT{},                                                               // Create flags
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning, // Message severity
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation, // Message type
        debug_callback                                         // Debug callback
    };
    debug_messenger_ = instance_.createDebugUtilsMessengerEXT(debug_info, nullptr, dldy_);
    LOGGING->verbose() << "Created debug messenger\n";
}
/**
 * @brief Creates the surface of the window
 */
void VulkanInstance::createSurface()
{
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance_, engine_->window_, nullptr, &surface) != VK_SUCCESS)
    {
        throw EXCEPTION("Failed to create window surface!");
    }
    LOGGING->verbose() << "Created window surface" << std::endl ;
    surface_ = vk::SurfaceKHR(surface);
}
/**
 * @brief Picks the physical device
 * 
 */
void VulkanInstance::pickPhysicalDevice() /** @todo Return physical device? **/
{
    
    std::vector<vk::PhysicalDevice> devices = instance_.enumeratePhysicalDevices();
    if (devices.size() == 0)
        throw EXCEPTION("Failed to find GPUs with Vulkan support");

    std::multimap<int, vk::PhysicalDevice> devicesmap;
    LOGGING->verbose() << "Available devices" << std::endl;
    for (const auto &device : devices)
    {
        /** @todo More advanced device selection(perhaps use maxComputeWorkGroupInvocations, and other compute parameters) **/
        int score = 1;
        LOGGING->print(VERBOSE) << device.getProperties().deviceName << std::endl;
        if (device.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
            score = 1000;
        devicesmap.insert(std::make_pair(score, device));
    }
    /** @todo Check score **/
    physical_device_ = devicesmap.rbegin()->second;
    LOGGING->verbose() << "Selected device:  " << physical_device_.getProperties().deviceName << '\n';
}

VulkanInstance::QueueFamilyIndices VulkanInstance::findQueueFamilies()
{
    QueueFamilyIndices indices;
    std::vector<vk::QueueFamilyProperties> queueFamilies = physical_device_.getQueueFamilyProperties();
    size_t i = 0;
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
    device_ = physical_device_.createDevice(createInfo);

    device_.getQueue(indices.graphicsFamily.value(), 0, &graphicsQueue);
    device_.getQueue(indices.presentFamily.value(), 0, &presentQueue);
    device_.getQueue(indices.computeFamily.value(), 0, &computeQueue);
}

VulkanInstance::SwapChainSupportDetails VulkanInstance::querySwapChainSupport()
{
    SwapChainSupportDetails details;

    physical_device_.getSurfaceCapabilitiesKHR(surface_, &details.capabilities);

    uint32_t formatCount;
    physical_device_.getSurfaceFormatsKHR(surface_, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        physical_device_.getSurfaceFormatsKHR(surface_, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    physical_device_.getSurfacePresentModesKHR(surface_, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        physical_device_.getSurfacePresentModesKHR(surface_, &presentModeCount, details.presentModes.data());
    }

    return details;
}

vk::SurfaceFormatKHR VulkanInstance::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats)
{
    for (const auto &availableFormat : availableFormats)
    {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear &&
            (physical_device_.getFormatProperties(availableFormat.format).optimalTilingFeatures & (vk::FormatFeatureFlagBits::eStorageImage | vk::FormatFeatureFlagBits::eColorAttachment)) == (vk::FormatFeatureFlagBits::eStorageImage | vk::FormatFeatureFlagBits::eColorAttachment))
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

vk::PresentModeKHR VulkanInstance::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes)
{
    for (const auto &availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox)
        {
            return availablePresentMode;
        }
    }

    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D VulkanInstance::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(engine_->window_, &engine_->config_.window_width, &engine_->config_.window_height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)};

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void VulkanInstance::createSwapChain()
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport();

    vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo{};
    createInfo.surface = surface_;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eStorage;

    QueueFamilyIndices indices = findQueueFamilies();
    uint32_t queueFamilyIndices[] = {indices.computeFamily.value(), indices.presentFamily.value()};

    if (indices.computeFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    if (device_.createSwapchainKHR(&createInfo, nullptr, &swapChain) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to create swap chain!");
    }

    device_.getSwapchainImagesKHR(swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    device_.getSwapchainImagesKHR(swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void VulkanInstance::createImageViews()
{
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        vk::ImageViewCreateInfo createInfo{};
        createInfo.sType = vk::StructureType::eImageViewCreateInfo;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = vk::ImageViewType::e2D;
        createInfo.format = swapChainImageFormat;
        createInfo.components.r = vk::ComponentSwizzle::eIdentity;
        createInfo.components.g = vk::ComponentSwizzle::eIdentity;
        createInfo.components.b = vk::ComponentSwizzle::eIdentity;
        createInfo.components.a = vk::ComponentSwizzle::eIdentity;
        createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (device_.createImageView(&createInfo, nullptr, &swapChainImageViews[i]) != vk::Result::eSuccess)
        {
            throw std::runtime_error("failed to create image views!");
        }
    }
}

void VulkanInstance::createDescriptorSetLayout()
{
    vk::DescriptorSetLayoutBinding uboLayoutBinding{}; // VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = vk::DescriptorType::eStorageImage;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eCompute;
    uboLayoutBinding.pImmutableSamplers = &imageSampler;

    vk::DescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = vk::StructureType::eDescriptorSetLayoutCreateInfo;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (device_.createDescriptorSetLayout(&layoutInfo, nullptr, &descriptorSetLayout) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

std::vector<char> VulkanInstance::readFile(const std::string &filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> file_buffer(fileSize);

    file.seekg(0);
    file.read(file_buffer.data(), fileSize);

    file.close();

    return file_buffer;
}

vk::ShaderModule VulkanInstance::createShaderModule(const std::vector<char> &code)
{
    vk::ShaderModuleCreateInfo createInfo{}; // VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO
    createInfo.sType = vk::StructureType::eShaderModuleCreateInfo;
    createInfo.codeSize = code.size();
    createInfo.pCode = (uint32_t *)(code.data());

    vk::ShaderModule shaderModule;
    if (device_.createShaderModule(&createInfo, nullptr, &shaderModule) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

void VulkanInstance::createComputePipeline()
{
    auto shader = readFile("shaders/comp.spv");
    vk::ShaderModule shaderModule = createShaderModule(shader);

    vk::PipelineShaderStageCreateInfo shaderStageInfo{}; // VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO
    shaderStageInfo.sType = vk::StructureType::ePipelineShaderStageCreateInfo;
    shaderStageInfo.stage = vk::ShaderStageFlagBits::eCompute;
    shaderStageInfo.module = shaderModule;
    shaderStageInfo.pName = "main";

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = vk::StructureType::ePipelineLayoutCreateInfo;
    pipelineLayoutInfo.setLayoutCount = 1;                 // Optional
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0;         // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr;      // Optional

    if (device_.createPipelineLayout(&pipelineLayoutInfo, nullptr, &pipelineLayout) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    vk::ComputePipelineCreateInfo info{};
    info.sType = vk::StructureType::eComputePipelineCreateInfo;
    info.layout = pipelineLayout;
    info.basePipelineIndex = -1;
    info.basePipelineHandle = VK_NULL_HANDLE;
    info.stage = shaderStageInfo;

    if (device_.createComputePipelines(VK_NULL_HANDLE, 1, &info, nullptr, &pipeline) != vk::Result::eSuccess)
    {
        throw std::runtime_error("compute shader");
    }

    device_.destroyShaderModule(shaderModule, nullptr);
}

void VulkanInstance::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies();

    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.queueFamilyIndex = queueFamilyIndices.computeFamily.value();

    if (device_.createCommandPool(&poolInfo, nullptr, &commandPool) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}

void VulkanInstance::recordImageBarrier(vk::CommandBuffer buffer, vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
                                        vk::AccessFlags scrAccess, vk::AccessFlags dstAccess, vk::PipelineStageFlags srcBind, vk::PipelineStageFlags dstBind)
{
    vk::ImageMemoryBarrier barrier{};
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcAccessMask = scrAccess;
    barrier.dstAccessMask = dstAccess; // VK_IMAGE_ASPECT_COLOR_BIT
    barrier.sType = vk::StructureType::eImageMemoryBarrier;
    vk::ImageSubresourceRange sub{};
    sub.aspectMask = vk::ImageAspectFlagBits::eColor;
    sub.baseArrayLayer = 0;
    sub.baseMipLevel = 0;
    sub.layerCount = VK_REMAINING_MIP_LEVELS;
    sub.levelCount = VK_REMAINING_MIP_LEVELS;
    barrier.subresourceRange = sub;
    buffer.pipelineBarrier(srcBind, dstBind, vk::DependencyFlags{}, 0, nullptr, 0, nullptr, 1, &barrier);
}

void VulkanInstance::createDescriptorPool()
{
    vk::DescriptorPoolSize poolSize{};
    poolSize.type = vk::DescriptorType::eStorageImage;
    poolSize.descriptorCount = static_cast<uint32_t>(swapChainImages.size());

    vk::DescriptorPoolCreateInfo poolInfo{}; // VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO
    poolInfo.sType = vk::StructureType::eDescriptorPoolCreateInfo;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(swapChainImages.size());
    if (device_.createDescriptorPool(&poolInfo, nullptr, &descriptorPool) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void VulkanInstance::createDescriptorSets()
{
    std::vector layouts(swapChainImages.size(), descriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = vk::StructureType::eDescriptorSetAllocateInfo;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(swapChainImages.size());
    if (device_.allocateDescriptorSets(&allocInfo, descriptorSets.data()) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    vk::SamplerCreateInfo samplerInfo{}; // VK_FILTER_LINEAR
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 16.0f; // VK_BORDER_COLOR_INT_OPAQUE_BLACK
    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;           // VK_COMPARE_OP_ALWAYS
    samplerInfo.compareOp = vk::CompareOp::eAlways; // VK_SAMPLER_MIPMAP_MODE_LINEAR
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    device_.createSampler(&samplerInfo, nullptr, &imageSampler);

    for (size_t i = 0; i < swapChainImages.size(); i++) // eroare pentru andrei aici :)
    {
        vk::DescriptorImageInfo info{};
        info.imageView = swapChainImageViews[i]; // VK_IMAGE_LAYOUT_GENERAL
        info.imageLayout = vk::ImageLayout::eGeneral;
        info.sampler = imageSampler;

        vk::WriteDescriptorSet descriptorWrite{}; // VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
        descriptorWrite.dstSet = descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0; // VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
        descriptorWrite.descriptorType = vk::DescriptorType::eStorageImage;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &info;

        device_.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
    }
}

void VulkanInstance::createCommandBuffers()
{
    commandBuffers.resize(swapChainImages.size());

    vk::CommandBufferAllocateInfo allocInfo{}; // VK_COMMAND_BUFFER_LEVEL_PRIMARY
    allocInfo.sType = vk::StructureType::eCommandBufferAllocateInfo;
    allocInfo.commandPool = commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if (device_.allocateCommandBuffers(&allocInfo, commandBuffers.data()) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    for (size_t i = 0; i < commandBuffers.size(); i++)
    {
        vk::CommandBufferBeginInfo beginInfo{};

        commandBuffers[i].begin(beginInfo);

        recordImageBarrier(commandBuffers[i], swapChainImages[i],
                           vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
                           vk::AccessFlagBits::eMemoryWrite, vk::AccessFlagBits::eMemoryRead,
                           vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eComputeShader);

        commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);
        commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);
        commandBuffers[i].dispatch(255, 255, 0);

        recordImageBarrier(commandBuffers[i], swapChainImages[i],
                           vk::ImageLayout::eGeneral, vk::ImageLayout::ePresentSrcKHR,
                           vk::AccessFlagBits::eMemoryWrite, vk::AccessFlagBits::eMemoryRead,
                           vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eAllCommands);

        /* VkImageCopy copy{};
        copy.dstOffset = { 0,0,0 };
        copy.extent = { swapChainExtent.width, swapChainExtent.height,1};
        copy.srcOffset = { 0,0,0 };

        VkImageSubresourceLayers subresource{};
        subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresource.baseArrayLayer = 0;
        subresource.layerCount = 1;
        subresource.mipLevel = 0;
        copy.srcSubresource = subresource;
        copy.dstSubresource = subresource;

        recordImageBarrier(commandBuffers[i], swapChainImages[i],
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_MEMORY_WRITE_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_HOST_BIT);

        vkCmdCopyImage(commandBuffers[i], renderTargetImages[i], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            swapChainImages[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1,©);

        recordImageBarrier(commandBuffers[i], swapChainImages[i],
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT,
            VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

            */
        commandBuffers[i].end();
    }
}

void VulkanInstance::createSyncObjects()
{
    imageAvailableSemaphores.resize(engine_->config_.MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(engine_->config_.MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(engine_->config_.MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

    vk::SemaphoreCreateInfo semaphoreInfo{};

    vk::FenceCreateInfo fenceInfo{vk::FenceCreateFlagBits::eSignaled};

    for (size_t i = 0; i < engine_->config_.MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (device_.createSemaphore(&semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != vk::Result::eSuccess ||
            device_.createSemaphore(&semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != vk::Result::eSuccess ||
            device_.createFence(&fenceInfo, nullptr, &inFlightFences[i]) != vk::Result::eSuccess)
        {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}