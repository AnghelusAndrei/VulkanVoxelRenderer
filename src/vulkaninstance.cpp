#include "vulkaninstance.hpp"

VulkanInstance::VulkanInstance(VoxelEngine *engine) : engine_(engine)
{

    createInstance();
    LOGGING->info() << "Created instance" << std::endl;
    selectPhysicalDevice();
    LOGGING->info() << "Selected physical device" << std::endl;
    createPermanentObjects();
    LOGGING->info() << "Created permanent objects" << std::endl;
    createSwapchainObjects();
    LOGGING->info() << "Created swapchain objects" << std::endl;
}

void VulkanInstance::render()
{
    device_.waitForFences(1, &inFlightFences_[currentFrame_], VK_TRUE, UINT64_MAX);
    uint32_t imageIndex;
    vk::Result result = device_.acquireNextImageKHR(swapChain_, UINT64_MAX, imageAvailableSemaphores_[currentFrame_], VK_NULL_HANDLE, &imageIndex);
    if (result == vk::Result::eErrorOutOfDateKHR)
    {
        // -_-
        // recreateSwapChain();
        return;
    }
    else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    if (imagesInFlightFences_[imageIndex] != vk::Fence{})
    {
        device_.waitForFences(1, &imagesInFlightFences_[imageIndex], VK_TRUE, UINT64_MAX);
    }
    imagesInFlightFences_[imageIndex] = inFlightFences_[currentFrame_];
    device_.waitIdle();
    vk::SubmitInfo submitInfo{};

    vk::Semaphore waitSemaphores[] = {imageAvailableSemaphores_[currentFrame_]}; // K_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers_[imageIndex];

    vk::Semaphore signalSemaphores[] = {renderFinishedSemaphores_[currentFrame_]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    device_.resetFences(1, &inFlightFences_[currentFrame_]);
    if (computeQueue_.submit(1, &submitInfo, inFlightFences_[currentFrame_]) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    vk::PresentInfoKHR presentInfo{}; // VK_STRUCTURE_TYPE_PRESENT_INFO_KHR

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    vk::SwapchainKHR swapChains[] = {swapChain_};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = presentQueue_.presentKHR(&presentInfo); // VK_SUBOPTIMAL_KHR

    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) // || framebufferResized)
    {
        // framebufferResized = false;
        //  -_-
        // recreateSwapChain();
    }
    else if (result != vk::Result::eSuccess)
    {
        throw EXCEPTION("Failed to present swap chain image");
    }

    currentFrame_ = (currentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

VulkanInstance::~VulkanInstance()
{

    instance_.destroy();
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
 * @brief Creates the base instance and debugging means
 *
 */
void VulkanInstance::createInstance()
{
    vk::ApplicationInfo app_info{
        engine_->config.window_title.c_str(), // Application Name
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
    dispatch_ = vk::DispatchLoaderDynamic(instance_, vkGetInstanceProcAddr);
    vk::DebugUtilsMessengerCreateInfoEXT debug_info{
        vk::DebugUtilsMessengerCreateFlagsEXT{},                                                               // Create flags
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning, // Message severity
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation, // Message type
        debug_callback                                         // Debug callback
    };
    debugMessenger_ = instance_.createDebugUtilsMessengerEXT(debug_info, nullptr, dispatch_);
    LOGGING->verbose() << "Created debug messenger\n";
}

/**
 * @brief Selects the physical device used by the renderer
 */
void VulkanInstance::selectPhysicalDevice()
{
    std::vector<vk::PhysicalDevice> physicalDeviceList = instance_.enumeratePhysicalDevices();

    if (physicalDeviceList.empty())
        throw EXCEPTION("Failed to find GPUs with Vulkan support");

    std::multimap<uint32_t, vk::PhysicalDevice> physicalDeviceMap;
    for (auto &device : physicalDeviceList)
    {
        uint32_t score = 0;
        LOGGING->print(VERBOSE) << device.getProperties().deviceName << std::endl;
        if (device.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
            score = 1000;
        physicalDeviceMap.insert(std::make_pair(score, device));
    }
    physicalDevice_ = physicalDeviceMap.rbegin()->second;
}

/**
 * @brief Creates objects that do not need to be recreated every swapchain refresh.
 * They are in order: surface, logical device, allocator, command pool, descriptor pools, buffers
 *
 */
void VulkanInstance::createPermanentObjects()
{
    vk::Result result;

    VkSurfaceKHR surface;
    if ((result = (vk::Result)glfwCreateWindowSurface(instance_, engine_->window, nullptr, &surface)) != vk::Result::eSuccess)
    {
        throw EXCEPTION("Failed to create window surface!", result);
    }
    LOGGING->verbose() << "Created window surface" << std::endl;
    surface_ = vk::SurfaceKHR(surface);

    QueueSupportDetails queueSupportDetails = utils_getQueueSupportDetails();

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {queueSupportDetails.computeFamily.value(), queueSupportDetails.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
        queueCreateInfos.push_back(vk::DeviceQueueCreateInfo{{}, queueFamily, 1, &queuePriority});

    vk::PhysicalDeviceFeatures physicalDeviceFeatures{};
    vk::DeviceCreateInfo deviceCreateInfo{};

    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

    deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(utils_deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = utils_deviceExtensions.data();
    deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(utils_validationLayers.size());
    deviceCreateInfo.ppEnabledLayerNames = utils_validationLayers.data();
    device_ = physicalDevice_.createDevice(deviceCreateInfo);

    computeQueue_ = device_.getQueue(queueSupportDetails.computeFamily.value(), 0);
    presentQueue_ = device_.getQueue(queueSupportDetails.presentFamily.value(), 0);

    VmaAllocatorCreateInfo allocator_create_info{};
    allocator_create_info.vulkanApiVersion = VK_API_VERSION_1_1;
    allocator_create_info.physicalDevice = static_cast<VkPhysicalDevice>(physicalDevice_);
    allocator_create_info.device = static_cast<VkDevice>(device_);
    allocator_create_info.instance = static_cast<VkInstance>(instance_);

    if ((result = (vk::Result)vmaCreateAllocator(&allocator_create_info, &allocator_)) != vk::Result::eSuccess)
        throw EXCEPTION("Failed to create allocator", result);

    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.queueFamilyIndex = queueSupportDetails.computeFamily.value();

    if ((result = device_.createCommandPool(&poolInfo, nullptr, &commandPool_)) != vk::Result::eSuccess)
    {
        throw EXCEPTION("Failed to create command pool", result);
    }

    stagingBuffer_ = utils_createBuffer(65536, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
    octreeBuffer_ = utils_createBuffer(65536, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer, VMA_MEMORY_USAGE_AUTO, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    lightingBuffer_ = utils_createBuffer(65536, vk::BufferUsageFlagBits::eStorageBuffer, VMA_MEMORY_USAGE_AUTO, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

void VulkanInstance::createSwapchainObjects()
{
    vk::Result result;
    QueueSupportDetails queueSupportDetails = utils_getQueueSupportDetails();

    SwapChainSupportDetails swapChainSupportDetails = utils_getSwapChainSupportDetails();
    LOGGING->verbose() << "Got swapchain support" << std::endl;

    uint32_t imageCount = swapChainSupportDetails.capabilities.minImageCount + 1;
    if (swapChainSupportDetails.capabilities.maxImageCount > 0 && imageCount > swapChainSupportDetails.capabilities.maxImageCount)
    {
        imageCount = swapChainSupportDetails.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo{};

    createInfo.surface = surface_;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = swapChainSupportDetails.format.format;
    createInfo.imageColorSpace = swapChainSupportDetails.format.colorSpace;
    createInfo.imageExtent = swapChainSupportDetails.extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eStorage;
    uint32_t queueFamilyIndices[] = {queueSupportDetails.computeFamily.value(), queueSupportDetails.presentFamily.value()};

    if (queueFamilyIndices[0] != queueFamilyIndices[0])
    {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }
    createInfo.preTransform = swapChainSupportDetails.capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = swapChainSupportDetails.presentMode;
    createInfo.clipped = VK_TRUE;

    LOGGING->verbose() << "Trying to create swapchain" << std::endl;
    if ((result = device_.createSwapchainKHR(&createInfo, nullptr, &swapChain_)) != vk::Result::eSuccess)
    {
        throw EXCEPTION("Failed to create swap chain", result);
    }
    LOGGING->info() << "Created swapchain" << std::endl;
    images_ = device_.getSwapchainImagesKHR(swapChain_);
    imageViews_.resize(images_.size());
    raycastPool_ = utils_createDescriptorPool({vk::DescriptorType::eStorageBuffer, vk::DescriptorType::eStorageBuffer, vk::DescriptorType::eStorageImage});
    lightingPool_ = utils_createDescriptorPool({vk::DescriptorType::eStorageBuffer, vk::DescriptorType::eStorageBuffer});
    renderPool_ = utils_createDescriptorPool({vk::DescriptorType::eStorageBuffer, vk::DescriptorType::eStorageImage});

    for (size_t i = 0; i < images_.size(); i++)
    {
        vk::ImageViewCreateInfo createInfo{};
        createInfo.image = images_[i];
        createInfo.viewType = vk::ImageViewType::e2D;
        createInfo.format = swapChainSupportDetails.format.format;
        createInfo.components.r = vk::ComponentSwizzle::eIdentity;
        createInfo.components.g = vk::ComponentSwizzle::eIdentity;
        createInfo.components.b = vk::ComponentSwizzle::eIdentity;
        createInfo.components.a = vk::ComponentSwizzle::eIdentity;
        createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if ((result = device_.createImageView(&createInfo, nullptr, &imageViews_[i])) != vk::Result::eSuccess)
        {
            throw EXCEPTION("Failed to create image views", result);
        }
    }
    LOGGING->info() << "Created image views" << std::endl;
    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerInfo.anisotropyEnable = false;
    samplerInfo.maxAnisotropy = 16.0f;
    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo.unnormalizedCoordinates = false;
    samplerInfo.compareEnable = false;
    samplerInfo.compareOp = vk::CompareOp::eAlways;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if ((result = device_.createSampler(&samplerInfo, nullptr, &imageSampler_)) != vk::Result::eSuccess)
    {
        throw EXCEPTION("Failed to create the sampler", result);
    }
    LOGGING->info() << "Created sampler" << std::endl;
    raycastSetLayout_ = utils_createDescriptorSetLayout({{0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute, nullptr}, {1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute, nullptr}, {2, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute, &imageSampler_}});
    lightingSetLayout_ = utils_createDescriptorSetLayout({{0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute, nullptr}, {1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute, nullptr}});
    renderSetLayout_ = utils_createDescriptorSetLayout({{0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute, nullptr}, {1, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute, &imageSampler_}});
    LOGGING->info() << "Created descriptor set layouts" << std::endl;
    raycastDescriptorSets_ = utils_allocateDescriptorSets(raycastPool_, raycastSetLayout_);
    lightingDescriptorSets_ = utils_allocateDescriptorSets(lightingPool_, lightingSetLayout_);
    renderDescriptorSets_ = utils_allocateDescriptorSets(renderPool_, renderSetLayout_);
    LOGGING->info() << "Created descriptor sets" << std::endl;
    for (size_t i = 0; i < images_.size(); i++)
    {
        vk::DescriptorBufferInfo octreeBufferInfo{};
        octreeBufferInfo.buffer = octreeBuffer_.buffer;
        octreeBufferInfo.range = VK_WHOLE_SIZE;
        vk::DescriptorBufferInfo lightingBufferInfo{};
        lightingBufferInfo.buffer = lightingBuffer_.buffer;
        lightingBufferInfo.range = VK_WHOLE_SIZE;
        vk::DescriptorImageInfo imageInfo{};
        imageInfo.imageView = imageViews_[i];
        imageInfo.imageLayout = vk::ImageLayout::eGeneral;
        imageInfo.sampler = imageSampler_;

        std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
        writeDescriptorSets.push_back(vk::WriteDescriptorSet{raycastDescriptorSets_[i], 0, 0, vk::DescriptorType::eStorageBuffer, nullptr, octreeBufferInfo, nullptr, nullptr});
        writeDescriptorSets.push_back(vk::WriteDescriptorSet{raycastDescriptorSets_[i], 1, 0, vk::DescriptorType::eStorageBuffer, nullptr, lightingBufferInfo, nullptr, nullptr});
        writeDescriptorSets.push_back(vk::WriteDescriptorSet{raycastDescriptorSets_[i], 2, 0, vk::DescriptorType::eStorageImage, imageInfo, nullptr, nullptr, nullptr});

        device_.updateDescriptorSets(writeDescriptorSets, nullptr);
    }

    for (size_t i = 0; i < images_.size(); i++)
    {
        vk::DescriptorBufferInfo octreeBufferInfo{};
        octreeBufferInfo.buffer = octreeBuffer_.buffer;
        octreeBufferInfo.range = VK_WHOLE_SIZE;
        vk::DescriptorBufferInfo lightingBufferInfo{};
        lightingBufferInfo.buffer = lightingBuffer_.buffer;
        lightingBufferInfo.range = VK_WHOLE_SIZE;
        std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
        writeDescriptorSets.push_back(vk::WriteDescriptorSet{lightingDescriptorSets_[i], 0, 0, vk::DescriptorType::eStorageBuffer, nullptr, octreeBufferInfo, nullptr, nullptr});
        writeDescriptorSets.push_back(vk::WriteDescriptorSet{lightingDescriptorSets_[i], 1, 0, vk::DescriptorType::eStorageBuffer, nullptr, lightingBufferInfo, nullptr, nullptr});

        device_.updateDescriptorSets(writeDescriptorSets, nullptr);
    }

    for (size_t i = 0; i < images_.size(); i++)
    {
        vk::DescriptorBufferInfo lightingBufferInfo{};
        lightingBufferInfo.buffer = lightingBuffer_.buffer;
        lightingBufferInfo.range = VK_WHOLE_SIZE;
        vk::DescriptorImageInfo imageInfo{};
        imageInfo.imageView = imageViews_[i];
        imageInfo.imageLayout = vk::ImageLayout::eGeneral;
        imageInfo.sampler = imageSampler_;

        std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
        writeDescriptorSets.push_back(vk::WriteDescriptorSet{renderDescriptorSets_[i], 0, 0, vk::DescriptorType::eStorageBuffer, nullptr, lightingBufferInfo, nullptr, nullptr});
        writeDescriptorSets.push_back(vk::WriteDescriptorSet{renderDescriptorSets_[i], 1, 0, vk::DescriptorType::eStorageImage, imageInfo, nullptr, nullptr, nullptr});

        device_.updateDescriptorSets(writeDescriptorSets, nullptr);
    }
    LOGGING->info() << "Updated descriptor sets" << std::endl;
    SpecializationConstants constants;
    vk::PipelineLayout raycastPipelineLayout_, lightingPipelineLayout_, renderPipelineLayout_;

    vk::PipelineShaderStageCreateInfo raycastShaderInfo{};
    raycastShaderInfo.stage = vk::ShaderStageFlagBits::eCompute;
    raycastShaderInfo.module = utils_createShaderModule("shaders/raycast.spv");
    raycastShaderInfo.pName = "main";

    vk::SpecializationInfo raycastSpecializationInfo;
    std::vector<vk::SpecializationMapEntry> raycastSpecializationEntries;
    raycastSpecializationEntries.push_back(vk::SpecializationMapEntry{0, offsetof(RaycastSpecialization, window_width), sizeof(uint32_t)});
    raycastSpecializationEntries.push_back(vk::SpecializationMapEntry{1, offsetof(RaycastSpecialization, window_height), sizeof(uint32_t)});
    raycastSpecializationInfo.dataSize = sizeof(constants.raycast);
    raycastSpecializationInfo.mapEntryCount = static_cast<uint32_t>(raycastSpecializationEntries.size());
    raycastSpecializationInfo.pMapEntries = raycastSpecializationEntries.data();
    raycastSpecializationInfo.pData = &constants.raycast;
    raycastShaderInfo.pSpecializationInfo = &raycastSpecializationInfo;

    vk::PipelineLayoutCreateInfo raycastLayoutCreateInfo{};
    raycastLayoutCreateInfo.setLayoutCount = 1;
    raycastLayoutCreateInfo.pSetLayouts = &raycastSetLayout_;

    raycastPipelineLayout_ = device_.createPipelineLayout(raycastLayoutCreateInfo);

    vk::ComputePipelineCreateInfo raycastPipelineCreateInfo{};
    raycastPipelineCreateInfo.layout = raycastPipelineLayout_;
    raycastPipelineCreateInfo.basePipelineIndex = -1;
    raycastPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    raycastPipelineCreateInfo.stage = raycastShaderInfo;

    vk::ResultValue<vk::Pipeline> pipelineResult = device_.createComputePipeline(VK_NULL_HANDLE, raycastPipelineCreateInfo);

    if (pipelineResult.result != vk::Result::eSuccess)
        throw EXCEPTION("Failed to create compute pipeline", pipelineResult.result);
    raycastPipeline_ = pipelineResult.value;

    vk::PipelineShaderStageCreateInfo lightingShaderInfo{};
    lightingShaderInfo.stage = vk::ShaderStageFlagBits::eCompute;
    lightingShaderInfo.module = utils_createShaderModule("shaders/lighting.spv");
    lightingShaderInfo.pName = "main";

    vk::SpecializationInfo lightingSpecializationInfo;
    std::vector<vk::SpecializationMapEntry> lightingSpecializationEntries;
    lightingSpecializationInfo.dataSize = sizeof(constants.lighting);
    lightingSpecializationInfo.mapEntryCount = static_cast<uint32_t>(lightingSpecializationEntries.size());
    lightingSpecializationInfo.pMapEntries = lightingSpecializationEntries.data();
    lightingSpecializationInfo.pData = &constants.lighting;
    lightingShaderInfo.pSpecializationInfo = &lightingSpecializationInfo;

    vk::PipelineLayoutCreateInfo lightingLayoutCreateInfo{};
    lightingLayoutCreateInfo.setLayoutCount = 1;
    lightingLayoutCreateInfo.pSetLayouts = &lightingSetLayout_;

    lightingPipelineLayout_ = device_.createPipelineLayout(lightingLayoutCreateInfo);

    vk::ComputePipelineCreateInfo lightingPipelineCreateInfo{};
    lightingPipelineCreateInfo.layout = lightingPipelineLayout_;
    lightingPipelineCreateInfo.basePipelineIndex = -1;
    lightingPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    lightingPipelineCreateInfo.stage = lightingShaderInfo;

    pipelineResult = device_.createComputePipeline(VK_NULL_HANDLE, lightingPipelineCreateInfo);

    if (pipelineResult.result != vk::Result::eSuccess)
        throw EXCEPTION("Failed to create compute pipeline", pipelineResult.result);
    lightingPipeline_ = pipelineResult.value;

    vk::PipelineShaderStageCreateInfo renderShaderInfo{};
    renderShaderInfo.stage = vk::ShaderStageFlagBits::eCompute;
    renderShaderInfo.module = utils_createShaderModule("shaders/render.spv");
    renderShaderInfo.pName = "main";

    vk::SpecializationInfo renderSpecializationInfo;
    std::vector<vk::SpecializationMapEntry> renderSpecializationEntries;
    renderSpecializationInfo.dataSize = sizeof(constants.render);
    renderSpecializationInfo.mapEntryCount = static_cast<uint32_t>(renderSpecializationEntries.size());
    renderSpecializationInfo.pMapEntries = renderSpecializationEntries.data();
    renderSpecializationInfo.pData = &constants.render;
    renderShaderInfo.pSpecializationInfo = &renderSpecializationInfo;

    vk::PipelineLayoutCreateInfo renderLayoutCreateInfo{};
    renderLayoutCreateInfo.sType = vk::StructureType::ePipelineLayoutCreateInfo;
    renderLayoutCreateInfo.setLayoutCount = 1;
    renderLayoutCreateInfo.pSetLayouts = &renderSetLayout_;

    renderPipelineLayout_ = device_.createPipelineLayout(renderLayoutCreateInfo);

    vk::ComputePipelineCreateInfo renderPipelineCreateInfo{};
    renderPipelineCreateInfo.layout = renderPipelineLayout_;
    renderPipelineCreateInfo.basePipelineIndex = -1;
    renderPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    renderPipelineCreateInfo.stage = renderShaderInfo;

    pipelineResult = device_.createComputePipeline(VK_NULL_HANDLE, renderPipelineCreateInfo);

    if (pipelineResult.result != vk::Result::eSuccess)
        throw EXCEPTION("Failed to create compute pipeline", pipelineResult.result);
    renderPipeline_ = pipelineResult.value;
    LOGGING->info() << "Created pipelines" << std::endl;
    commandBuffers_.resize(images_.size() * 2);
    copyCommandBuffers_.resize(images_.size());

    vk::CommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.commandPool = commandPool_;
    commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
    commandBufferAllocateInfo.commandBufferCount = (uint32_t)commandBuffers_.size();
    commandBuffers_ = device_.allocateCommandBuffers(commandBufferAllocateInfo);
    for (size_t i = 0; i < commandBuffers_.size() / 2; i++)
    {
        vk::CommandBufferBeginInfo beginInfo{};

        commandBuffers_[i].begin(beginInfo);

        commandBuffers_[i].pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlags{}, {},
                                           std::vector<vk::BufferMemoryBarrier>{
                                               {vk::AccessFlagBits::eMemoryWrite, vk::AccessFlagBits::eMemoryRead, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, octreeBuffer_.buffer, 0, 65536}},
                                           std::vector<vk::ImageMemoryBarrier>{
                                               {vk::AccessFlagBits::eMemoryWrite, vk::AccessFlagBits::eMemoryRead, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, images_[i], {vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_MIP_LEVELS}}});
        commandBuffers_[i].bindPipeline(vk::PipelineBindPoint::eCompute, raycastPipeline_);
        commandBuffers_[i].bindDescriptorSets(vk::PipelineBindPoint::eCompute, raycastPipelineLayout_, 0, 1, &raycastDescriptorSets_[i], 0, nullptr);
        commandBuffers_[i].dispatch(engine_->config.window_width / 16 + 1, engine_->config.window_height / 16 + 1, 1); // TODO

        commandBuffers_[i].pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eBottomOfPipe, vk::DependencyFlags{}, {},
                                           std::vector<vk::BufferMemoryBarrier>{
                                               {vk::AccessFlagBits::eMemoryWrite, vk::AccessFlagBits::eMemoryRead, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, lightingBuffer_.buffer, 0, 65536}},
                                           std::vector<vk::ImageMemoryBarrier>{
                                               {vk::AccessFlagBits::eMemoryWrite, vk::AccessFlagBits::eMemoryRead, vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, images_[i], {vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_MIP_LEVELS}}});
        commandBuffers_[i].bindPipeline(vk::PipelineBindPoint::eCompute, lightingPipeline_);
        commandBuffers_[i].bindDescriptorSets(vk::PipelineBindPoint::eCompute, lightingPipelineLayout_, 0, 1, &lightingDescriptorSets_[i], 0, nullptr);
        commandBuffers_[i].dispatch(65536, 1, 1); // TODO

        commandBuffers_[i].pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eBottomOfPipe, vk::DependencyFlags{}, {},
                                           std::vector<vk::BufferMemoryBarrier>{
                                               {vk::AccessFlagBits::eMemoryWrite, vk::AccessFlagBits::eMemoryRead, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, lightingBuffer_.buffer, 0, 65536}},
                                           std::vector<vk::ImageMemoryBarrier>{
                                               {vk::AccessFlagBits::eMemoryWrite, vk::AccessFlagBits::eMemoryRead, vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, images_[i], {vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_MIP_LEVELS}}});

        commandBuffers_[i].bindPipeline(vk::PipelineBindPoint::eCompute, renderPipeline_);
        commandBuffers_[i].bindDescriptorSets(vk::PipelineBindPoint::eCompute, renderPipelineLayout_, 0, 1, &renderDescriptorSets_[i], 0, nullptr);
        commandBuffers_[i].dispatch(engine_->config.window_width / 16 + 1, engine_->config.window_height / 16 + 1, 1); // TODO

        commandBuffers_[i].pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eBottomOfPipe, vk::DependencyFlags{}, {},
                                           std::vector<vk::BufferMemoryBarrier>{}, std::vector<vk::ImageMemoryBarrier>{{vk::AccessFlagBits::eMemoryWrite, vk::AccessFlagBits::eMemoryRead, vk::ImageLayout::eGeneral, vk::ImageLayout::ePresentSrcKHR, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, images_[i], {vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_MIP_LEVELS}}});

        commandBuffers_[i].end();
    }
    LOGGING->info() << "First command buffers recorded";
    for (size_t i = commandBuffers_.size() / 2; i < commandBuffers_.size(); i++)
    {
        vk::CommandBufferBeginInfo beginInfo{};

        commandBuffers_[i].begin(beginInfo);
        commandBuffers_[i].copyBuffer(stagingBuffer_.buffer, octreeBuffer_.buffer, vk::BufferCopy{0, 0, 65536});
        commandBuffers_[i].pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlags{}, {},
                                           std::vector<vk::BufferMemoryBarrier>{
                                               {vk::AccessFlagBits::eMemoryWrite, vk::AccessFlagBits::eMemoryRead, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, octreeBuffer_.buffer, 0, 65536}},
                                           std::vector<vk::ImageMemoryBarrier>{
                                               {vk::AccessFlagBits::eMemoryWrite, vk::AccessFlagBits::eMemoryRead, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, images_[i-images_.size()], {vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_MIP_LEVELS}}});
        commandBuffers_[i].bindPipeline(vk::PipelineBindPoint::eCompute, raycastPipeline_);
        commandBuffers_[i].bindDescriptorSets(vk::PipelineBindPoint::eCompute, raycastPipelineLayout_, 0, 1, &raycastDescriptorSets_[i-images_.size()], 0, nullptr);
        commandBuffers_[i].dispatch(engine_->config.window_width / 16 + 1, engine_->config.window_height / 16 + 1, 1); // TODO

        commandBuffers_[i].pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eBottomOfPipe, vk::DependencyFlags{}, {},
                                           std::vector<vk::BufferMemoryBarrier>{
                                               {vk::AccessFlagBits::eMemoryWrite, vk::AccessFlagBits::eMemoryRead, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, lightingBuffer_.buffer, 0, 65536}},
                                           std::vector<vk::ImageMemoryBarrier>{
                                               {vk::AccessFlagBits::eMemoryWrite, vk::AccessFlagBits::eMemoryRead, vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, images_[i-images_.size()], {vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_MIP_LEVELS}}});
        commandBuffers_[i].bindPipeline(vk::PipelineBindPoint::eCompute, lightingPipeline_);
        commandBuffers_[i].bindDescriptorSets(vk::PipelineBindPoint::eCompute, lightingPipelineLayout_, 0, 1, &lightingDescriptorSets_[i-images_.size()], 0, nullptr);
        commandBuffers_[i].dispatch(65536, 1, 1); // TODO

        commandBuffers_[i].pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eBottomOfPipe, vk::DependencyFlags{}, {},
                                           std::vector<vk::BufferMemoryBarrier>{
                                               {vk::AccessFlagBits::eMemoryWrite, vk::AccessFlagBits::eMemoryRead, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, lightingBuffer_.buffer, 0, 65536}},
                                           std::vector<vk::ImageMemoryBarrier>{
                                               {vk::AccessFlagBits::eMemoryWrite, vk::AccessFlagBits::eMemoryRead, vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, images_[i-images_.size()], {vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_MIP_LEVELS}}});

        commandBuffers_[i].bindPipeline(vk::PipelineBindPoint::eCompute, renderPipeline_);
        commandBuffers_[i].bindDescriptorSets(vk::PipelineBindPoint::eCompute, renderPipelineLayout_, 0, 1, &renderDescriptorSets_[i-images_.size()], 0, nullptr);
        commandBuffers_[i].dispatch(engine_->config.window_width / 16 + 1, engine_->config.window_height / 16 + 1, 1); // TODO

        commandBuffers_[i].pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eBottomOfPipe, vk::DependencyFlags{}, {},
                                           std::vector<vk::BufferMemoryBarrier>{}, std::vector<vk::ImageMemoryBarrier>{{vk::AccessFlagBits::eMemoryWrite, vk::AccessFlagBits::eMemoryRead, vk::ImageLayout::eGeneral, vk::ImageLayout::ePresentSrcKHR, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, images_[i-images_.size()], {vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_MIP_LEVELS}}});

        commandBuffers_[i].end();
    }
    LOGGING->info() << "Created command buffers" << std::endl;
    imageAvailableSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences_.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlightFences_.resize(images_.size(), vk::Fence{});

    vk::SemaphoreCreateInfo semaphoreCreateInfo{};

    vk::FenceCreateInfo fenceCreateInfo{vk::FenceCreateFlagBits::eSignaled};

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (device_.createSemaphore(&semaphoreCreateInfo, nullptr, &imageAvailableSemaphores_[i]) != vk::Result::eSuccess ||
            device_.createSemaphore(&semaphoreCreateInfo, nullptr, &renderFinishedSemaphores_[i]) != vk::Result::eSuccess ||
            device_.createFence(&fenceCreateInfo, nullptr, &inFlightFences_[i]) != vk::Result::eSuccess)
        {
            throw EXCEPTION("Failed to create synchronization objects for a frame");
        }
    }
}

VulkanInstance::QueueSupportDetails VulkanInstance::utils_getQueueSupportDetails()
{
    vk::Result result;
    QueueSupportDetails queueSupportDetails;
    std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice_.getQueueFamilyProperties();
    size_t i = 0;
    for (auto &queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute)
        {
            queueSupportDetails.computeFamily = i;
        }
        if (physicalDevice_.getSurfaceSupportKHR(i, surface_) && queueSupportDetails.computeFamily == i)
        {

            queueSupportDetails.presentFamily = i;
        }
        if (queueSupportDetails.hasValues())
            break;
    }
    return queueSupportDetails;
}

/**
 * @brief Creates a descriptor pool
 *
 * @param descriptorTypes
 * @return vk::DescriptorPool
 */
vk::DescriptorPool VulkanInstance::utils_createDescriptorPool(std::vector<vk::DescriptorType> descriptorTypes)
{
    std::vector<vk::DescriptorPoolSize> pool_sizes;
    for (vk::DescriptorType type : descriptorTypes)
    {
        pool_sizes.push_back(vk::DescriptorPoolSize{type, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)});
    }
    vk::DescriptorPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.poolSizeCount = pool_sizes.size();
    poolCreateInfo.pPoolSizes = pool_sizes.data();
    poolCreateInfo.maxSets = static_cast<uint32_t>(images_.size());
    vk::DescriptorPool descriptorPool;
    if (device_.createDescriptorPool(&poolCreateInfo, nullptr, &descriptorPool) != vk::Result::eSuccess)
    {
        throw EXCEPTION("Failed to create descriptor pool");
    }
    return descriptorPool;
}

/**
 * @brief Creates a buffer
 *
 * @param size
 * @param bufferUsage
 * @param memoryUsage
 * @param flags
 * @return VulkanInstance::VmaBuffer
 */
VulkanInstance::VmaBuffer VulkanInstance::utils_createBuffer(vk::DeviceSize size, vk::BufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags)
{
    VmaBuffer buffer;
    vk::Result result;

    vk::BufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = bufferUsage;
    VmaAllocationCreateInfo allocationInfo{};
    allocationInfo.usage = memoryUsage;
    allocationInfo.flags = flags;
    if ((result = (vk::Result)vmaCreateBuffer(allocator_, reinterpret_cast<VkBufferCreateInfo *>(&bufferCreateInfo), &allocationInfo, reinterpret_cast<VkBuffer *>(&buffer.buffer), &buffer.allocation, nullptr)) != vk::Result::eSuccess)
        throw EXCEPTION("Failed to create buffer", result);
    return buffer;
}

/**
 * @brief Get swapchain details
 *
 * @return VulkanInstance::SwapChainSupportDetails
 */
VulkanInstance::SwapChainSupportDetails VulkanInstance::utils_getSwapChainSupportDetails()
{
    vk::Result result;
    SwapChainSupportDetails supportDetails;
    supportDetails.capabilities = physicalDevice_.getSurfaceCapabilitiesKHR(surface_);
    for (auto &format : physicalDevice_.getSurfaceFormatsKHR(surface_))
    {
        if ((physicalDevice_.getFormatProperties(format.format).optimalTilingFeatures & (vk::FormatFeatureFlagBits::eStorageImage | vk::FormatFeatureFlagBits::eColorAttachment)))
        {
            supportDetails.format = format;
            break;
        }
    }
    if (supportDetails.format.format == vk::Format::eUndefined)
        throw EXCEPTION("Failed to find storage bit swapchain format.");

    for (auto &presentMode : physicalDevice_.getSurfacePresentModesKHR(surface_))
    {
        if (presentMode == vk::PresentModeKHR::eMailbox)
        {
            supportDetails.presentMode = presentMode;
            break;
        }
    }
    if (supportDetails.presentMode != vk::PresentModeKHR::eMailbox)
        supportDetails.presentMode = vk::PresentModeKHR::eFifo;

    supportDetails.extent = vk::Extent2D{
        engine_->config.window_width,
        engine_->config.window_height};

    supportDetails.extent.width = std::clamp(supportDetails.extent.width, supportDetails.capabilities.minImageExtent.width, supportDetails.capabilities.maxImageExtent.width);
    supportDetails.extent.height = std::clamp(supportDetails.extent.height, supportDetails.capabilities.minImageExtent.height, supportDetails.capabilities.maxImageExtent.height);
    return supportDetails;
}

vk::DescriptorSetLayout VulkanInstance::utils_createDescriptorSetLayout(std::vector<vk::DescriptorSetLayoutBinding> bindings)
{
    vk::DescriptorSetLayout descriptorSetLayout;
    vk::Result result;
    return device_.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo{{}, bindings});
}

std::vector<vk::DescriptorSet> VulkanInstance::utils_allocateDescriptorSets(vk::DescriptorPool pool, vk::DescriptorSetLayout layout)
{
    std::vector<vk::DescriptorSetLayout> layouts(images_.size(), layout);
    vk::DescriptorSetAllocateInfo allocateInfo;
    allocateInfo.descriptorPool = pool;
    allocateInfo.descriptorSetCount = static_cast<uint32_t>(images_.size());
    allocateInfo.pSetLayouts = layouts.data();

    return device_.allocateDescriptorSets(allocateInfo);
}

vk::ShaderModule VulkanInstance::utils_createShaderModule(std::string path)
{
    vk::Result result;
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        throw EXCEPTION("Failed to open file " + path);
    }
    size_t size = file.tellg();
    std::vector<char> data(size);

    file.seekg(0);
    file.read(data.data(), size);
    file.close();

    vk::ShaderModuleCreateInfo createInfo{{}, size, (uint32_t *)data.data()}; // VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO

    return device_.createShaderModule(createInfo);
}