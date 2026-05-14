#include "flpch.h"

#include "VulkanDevice.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanShader.h"
#include "VulkanPipeline.h"
#include "VulkanRenderPass.h"
#include "VulkanFence.h"
#include "VulkanSemaphore.h"
#include "VulkanDescriptorSet.h"
#include "VulkanFramebuffer.h"
#include "VulkanSampler.h"

#include <vk_mem_alloc.h>
#include <GLFW/glfw3.h>

#include <set>

namespace Flux {

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT* data,
        void* userData)
    {
        if (severity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            return VK_FALSE;

        auto objectTypeName = [](VkObjectType t) -> const char* {
            switch (t) {
            case VK_OBJECT_TYPE_INSTANCE:              return "Instance";
            case VK_OBJECT_TYPE_PHYSICAL_DEVICE:       return "PhysicalDevice";
            case VK_OBJECT_TYPE_DEVICE:                return "Device";
            case VK_OBJECT_TYPE_QUEUE:                 return "Queue";
            case VK_OBJECT_TYPE_SEMAPHORE:             return "Semaphore";
            case VK_OBJECT_TYPE_COMMAND_BUFFER:        return "CommandBuffer";
            case VK_OBJECT_TYPE_FENCE:                 return "Fence";
            case VK_OBJECT_TYPE_BUFFER:                return "Buffer";
            case VK_OBJECT_TYPE_IMAGE:                 return "Image";
            case VK_OBJECT_TYPE_IMAGE_VIEW:            return "ImageView";
            case VK_OBJECT_TYPE_SHADER_MODULE:         return "ShaderModule";
            case VK_OBJECT_TYPE_PIPELINE_LAYOUT:       return "PipelineLayout";
            case VK_OBJECT_TYPE_RENDER_PASS:           return "RenderPass";
            case VK_OBJECT_TYPE_PIPELINE:              return "Pipeline";
            case VK_OBJECT_TYPE_DESCRIPTOR_SET:        return "DescriptorSet";
            case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT: return "DescriptorSetLayout";
            case VK_OBJECT_TYPE_FRAMEBUFFER:           return "Framebuffer";
            case VK_OBJECT_TYPE_COMMAND_POOL:          return "CommandPool";
            case VK_OBJECT_TYPE_SAMPLER:               return "Sampler";
            default:                                   return "Unknown";
            }
            };

        bool isError = severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        if (isError) FL_CORE_ERROR("[Vulkan] {}", data->pMessage);
        else         FL_CORE_WARN("[Vulkan] {}", data->pMessage);

        if (data->pMessageIdName)
        {
            if (isError) FL_CORE_ERROR("  ^ {}", data->pMessageIdName);
            else         FL_CORE_WARN("  ^ {}", data->pMessageIdName);
        }

        for (uint32_t i = 0; i < data->objectCount; i++)
        {
            const auto& obj = data->pObjects[i];
            const char* name = (obj.pObjectName && obj.pObjectName[0]) ? obj.pObjectName : "unnamed";
            if (isError) FL_CORE_ERROR("  [{}] {} ({})", i, objectTypeName(obj.objectType), name);
            else         FL_CORE_WARN("  [{}] {} ({})", i, objectTypeName(obj.objectType), name);
        }

        return VK_FALSE;
    }

    // =========================================================================
    //  Constructor / Destructor
    // =========================================================================

    VulkanDevice::VulkanDevice(void* windowHandle, uint32_t width, uint32_t height)
    {
        CreateInstance();
        CreateDebugMessenger();
        CreateSurface(windowHandle);
        PickPhysicalDevice();
        CreateLogicalDevice();
        CreateAllocator();
        CreateDescriptorPool();

        m_Swapchain = CreateScope<VulkanSwapchain>(
            m_Device, m_PhysicalDevice, m_Surface, m_PresentQueue, width, height);

        CreateCommandLists();

        VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        allocInfo.commandPool = m_TransferCommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;
        vkAllocateCommandBuffers(m_Device, &allocInfo, &m_ImmediateCmd);

        VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        vkCreateFence(m_Device, &fenceInfo, nullptr, &m_ImmediateFence);

        FL_CORE_INFO("Created Vulkan Device");
    }

    VulkanDevice::~VulkanDevice()
    {
        vkDeviceWaitIdle(m_Device);

        vkDestroyFence(m_Device, m_ImmediateFence, nullptr);
        vkFreeCommandBuffers(m_Device, m_TransferCommandPool, 1, &m_ImmediateCmd);

        m_CommandLists.clear();
        m_Swapchain.reset();

        vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
        vkDestroyCommandPool(m_Device, m_GraphicsCommandPool, nullptr);
        vkDestroyCommandPool(m_Device, m_TransferCommandPool, nullptr);

        vmaDestroyAllocator(m_Allocator);
        vkDestroyDevice(m_Device, nullptr);
    }

    // =========================================================================
    //  Resource creation
    // =========================================================================

    Scope<RHIBuffer> VulkanDevice::CreateBuffer(const BufferSpec& spec)
    {
        return CreateScope<VulkanBuffer>(m_Allocator, spec);
    }

    Scope<RHITexture> VulkanDevice::CreateTexture(const TextureSpec& spec)
    {
        auto texture = CreateScope<VulkanTexture>(m_Device, m_Allocator, spec);
        texture->SetupForSetData(m_GraphicsQueue, m_TransferCommandPool);
        return texture;
    }

    Scope<RHISampler> VulkanDevice::CreateSampler(const SamplerSpec& spec)
    {
        return CreateScope<VulkanSampler>(m_Device, spec);
    }

    Scope<RHIFramebuffer> VulkanDevice::CreateFramebuffer(const FramebufferSpec& spec)
    {
        return CreateScope<VulkanFramebuffer>(m_Device, spec);
    }

    Scope<RHIPipeline> VulkanDevice::CreatePipeline(const PipelineDesc& desc)
    {
        return CreateScope<VulkanPipeline>(m_Device, desc);
    }

    Scope<RHIRenderPass> VulkanDevice::CreateRenderPass(const RenderPassDesc& desc)
    {
        return CreateScope<VulkanRenderPass>(m_Device, desc);
    }

    Scope<RHIShader> VulkanDevice::CreateShader(ShaderStage stage, const std::vector<uint32_t>& spirv)
    {
        return CreateScope<VulkanShader>(m_Device, stage, spirv);
    }

    Scope<RHIFence> VulkanDevice::CreateFence(bool signaled)
    {
        return CreateScope<VulkanFence>(m_Device, signaled);
    }

    Scope<RHISemaphore> VulkanDevice::CreateSemaphore()
    {
        return CreateScope<VulkanSemaphore>(m_Device);
    }

    Scope<RHIDescriptorSetLayout> VulkanDevice::CreateDescriptorSetLayout(const DescriptorSetLayoutDesc& desc)
    {
        return CreateScope<VulkanDescriptorSetLayout>(m_Device, desc);
    }

    Scope<RHIDescriptorSet> VulkanDevice::CreateDescriptorSet(const RHIDescriptorSetLayout* layout)
    {
        // Передаём layoutDesc — VulkanDescriptorSet знает типы биндингов при BindBuffer
        return CreateScope<VulkanDescriptorSet>(
            m_Device,
            m_DescriptorPool,
            static_cast<VkDescriptorSetLayout>(layout->GetHandle()),
            layout->GetDesc());
    }

    // =========================================================================
    //  Submit
    // =========================================================================

    void VulkanDevice::Submit(const SubmitDesc& desc)
    {
        FL_CORE_ASSERT(desc.CommandList, "Submit: CommandList is null!");

        VkCommandBuffer cmd = static_cast<VkCommandBuffer>(desc.CommandList->GetHandle());
        VkFence         fence = desc.SignalFence ? static_cast<VkFence>(desc.SignalFence->GetHandle()) : VK_NULL_HANDLE;
        VkSemaphore     waitSem = desc.WaitSemaphore ? static_cast<VkSemaphore>(desc.WaitSemaphore->GetHandle()) : VK_NULL_HANDLE;
        VkSemaphore     signalSem = desc.SignalSemaphore ? static_cast<VkSemaphore>(desc.SignalSemaphore->GetHandle()) : VK_NULL_HANDLE;

        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;
        submitInfo.waitSemaphoreCount = waitSem != VK_NULL_HANDLE ? 1 : 0;
        submitInfo.pWaitSemaphores = waitSem != VK_NULL_HANDLE ? &waitSem : nullptr;
        submitInfo.pWaitDstStageMask = waitSem != VK_NULL_HANDLE ? &waitStage : nullptr;
        submitInfo.signalSemaphoreCount = signalSem != VK_NULL_HANDLE ? 1 : 0;
        submitInfo.pSignalSemaphores = signalSem != VK_NULL_HANDLE ? &signalSem : nullptr;

        FL_CORE_ASSERT(vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, fence) == VK_SUCCESS,
            "Failed to submit Command Buffer");
    }

    void VulkanDevice::ImmediateSubmit(std::function<void(RHICommandList*)>&& fn)
    {
        vkResetFences(m_Device, 1, &m_ImmediateFence);
        vkResetCommandBuffer(m_ImmediateCmd, 0);

        VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(m_ImmediateCmd, &beginInfo);

        // Минимальный адаптер: вызывающий пишет команды через cmd->GetHandle<VkCommandBuffer>()
        struct ImmediateAdapter final : RHICommandList
        {
            VkCommandBuffer Cmd = VK_NULL_HANDLE;

            void Begin() override {}
            void End()   override {}
            void* GetHandle() const override { return Cmd; }
            void BeginRenderPass(RHIRenderPass*, RHIFramebuffer*, glm::vec4, float, uint8_t) override {}
            void EndRenderPass()                                                              override {}
            void SetViewport(float, float, float, float, float, float)                            override {}
            void SetScissor(int32_t, int32_t, uint32_t, uint32_t)                              override {}
            void SetPipeline(RHIPipeline*)                                                   override {}
            void PushConstants(RHIPipeline*, ShaderStage, uint32_t, uint32_t, const void*)      override {}
            void BindVertexBuffer(RHIBuffer*, uint64_t)                                      override {}
            void BindIndexBuffer(RHIBuffer*, IndexType, uint64_t)                            override {}
            void BindDescriptorSet(uint32_t, RHIDescriptorSet*, RHIPipeline*)               override {}
            void Draw(uint32_t, uint32_t, uint32_t, uint32_t)                                   override {}
            void DrawIndexed(uint32_t, uint32_t, uint32_t, int32_t, uint32_t)                   override {}
            void DrawIndirect(RHIBuffer*, uint64_t, uint32_t)                                  override {}
            void DrawIndexedIndirect(RHIBuffer*, uint64_t, uint32_t)                          override {}
            void Dispatch(uint32_t, uint32_t, uint32_t)                                        override {}
            void DispatchIndirect(RHIBuffer*, uint64_t)                                       override {}
            void CopyBuffer(RHIBuffer*, RHIBuffer*, uint64_t, uint64_t, uint64_t)               override {}
            void CopyBufferToTexture(RHIBuffer*, RHITexture*, const TextureRegion&)            override {}
            void BlitTexture(RHITexture*, RHITexture*, TextureRegion, TextureRegion, FilterMode) override {}
            void ResourceBarrier(RHITexture*, ResourceState, ResourceState)                    override {}
            void BufferBarrier(RHIBuffer*, ResourceState, ResourceState)                       override {}
        };

        ImmediateAdapter adapter;
        adapter.Cmd = m_ImmediateCmd;

        fn(&adapter);

        vkEndCommandBuffer(m_ImmediateCmd);

        VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_ImmediateCmd;
        vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_ImmediateFence);
        vkWaitForFences(m_Device, 1, &m_ImmediateFence, VK_TRUE, UINT64_MAX);
    }

    // =========================================================================
    //  Utils
    // =========================================================================

    void VulkanDevice::CopyBuffer(RHIBuffer* src, RHIBuffer* dst,
        uint64_t size, uint64_t srcOffset, uint64_t dstOffset) const
    {
        VkBuffer     srcBuf = static_cast<VkBuffer>(src->GetHandle());
        VkBuffer     dstBuf = static_cast<VkBuffer>(dst->GetHandle());
        VkDeviceSize copySize = (size > 0) ? size : src->GetSize();

        VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        allocInfo.commandPool = m_TransferCommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer cmd;
        vkAllocateCommandBuffers(m_Device, &allocInfo, &cmd);

        VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmd, &beginInfo);

        VkBufferCopy region{};
        region.srcOffset = srcOffset;
        region.dstOffset = dstOffset;
        region.size = copySize;
        vkCmdCopyBuffer(cmd, srcBuf, dstBuf, 1, &region);

        vkEndCommandBuffer(cmd);

        VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;
        vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_GraphicsQueue);

        vkFreeCommandBuffers(m_Device, m_TransferCommandPool, 1, &cmd);
    }

    DeviceMemoryStats VulkanDevice::GetMemoryStatistics() const
    {
        VmaBudget budgets[VK_MAX_MEMORY_HEAPS];
        vmaGetHeapBudgets(m_Allocator, budgets);

        DeviceMemoryStats result{};
        result.AllocationBytes = budgets[0].statistics.allocationBytes;
        result.AllocationCount = budgets[0].statistics.allocationCount;
        result.BlockBytes = budgets[0].statistics.blockBytes;
        result.BlockCount = budgets[0].statistics.blockCount;
        result.Usage = budgets[0].usage;
        result.Budget = budgets[0].budget;
        return result;
    }

    void VulkanDevice::WaitIdle() const
    {
        vkDeviceWaitIdle(m_Device);
    }

    // =========================================================================
    //  Private init
    // =========================================================================

    void VulkanDevice::CreateInstance()
    {
        static VkValidationFeaturesEXT validationFeatures{};
        validationFeatures.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;

        static std::array<VkValidationFeatureEnableEXT, 1> enables = {
            VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT
        };
        validationFeatures.enabledValidationFeatureCount = static_cast<uint32_t>(enables.size());
        validationFeatures.pEnabledValidationFeatures = enables.data();

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Flux";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Flux Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        std::vector<const char*> extensions = {
            VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef _WIN32
            "VK_KHR_win32_surface",
#elif __APPLE__
            "VK_MVK_macos_surface",
#endif
#ifdef FL_DEBUG
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
        };

        std::vector<const char*> layers;
#ifdef FL_DEBUG
        layers.emplace_back("VK_LAYER_KHRONOS_validation");
#endif

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        createInfo.ppEnabledLayerNames = layers.data();
#ifdef FL_DEBUG
        createInfo.pNext = &validationFeatures;
#endif

        FL_CORE_ASSERT(vkCreateInstance(&createInfo, nullptr, &m_Instance) == VK_SUCCESS,
            "Failed to create Vulkan Instance");
    }

    void VulkanDevice::CreateDebugMessenger()
    {
#ifdef FL_DEBUG
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        createInfo.pfnUserCallback = DebugCallback;

        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT");
        FL_CORE_ASSERT(func, "Failed to load vkCreateDebugUtilsMessengerEXT");
        func(m_Instance, &createInfo, nullptr, &m_DebugMessenger);
#endif
    }

    void VulkanDevice::CreateSurface(void* windowHandle)
    {
        FL_CORE_ASSERT(
            glfwCreateWindowSurface(m_Instance, (GLFWwindow*)windowHandle, nullptr, &m_Surface) == VK_SUCCESS,
            "Failed to create GLFW Surface");
    }

    void VulkanDevice::PickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
        FL_CORE_ASSERT(deviceCount > 0, "No Vulkan-capable GPU found!");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

        for (auto& device : devices)
        {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(device, &props);
            if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                m_PhysicalDevice = device;
                FL_CORE_INFO("Selected GPU: {}", props.deviceName);
                return;
            }
        }

        m_PhysicalDevice = devices[0];
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(m_PhysicalDevice, &props);
        FL_CORE_INFO("Selected GPU: {}", props.deviceName);
    }

    void VulkanDevice::CreateLogicalDevice()
    {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, queueFamilies.data());

        uint32_t graphicsFamily = UINT32_MAX;
        uint32_t presentFamily = UINT32_MAX;
        uint32_t computeFamily = UINT32_MAX;

        for (uint32_t i = 0; i < queueFamilyCount; i++)
        {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) graphicsFamily = i;
            if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT)  computeFamily = i;

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, i, m_Surface, &presentSupport);
            if (presentSupport) presentFamily = i;

            if (graphicsFamily != UINT32_MAX && presentFamily != UINT32_MAX && computeFamily != UINT32_MAX)
                break;
        }

        FL_CORE_ASSERT(graphicsFamily != UINT32_MAX, "No graphics queue family found!");
        m_GraphicsQueueFamilyIndex = graphicsFamily;

        std::set<uint32_t> uniqueFamilies = { graphicsFamily, presentFamily, computeFamily };
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        float queuePriority = 1.0f;

        for (uint32_t family : uniqueFamilies)
        {
            VkDeviceQueueCreateInfo queueInfo{};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueFamilyIndex = family;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.emplace_back(queueInfo);
        }

        VkPhysicalDeviceVulkan12Features vulkan12Features{};
        vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;

        VkPhysicalDeviceFeatures2 features2{};
        features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        features2.pNext = &vulkan12Features;
        vkGetPhysicalDeviceFeatures2(m_PhysicalDevice, &features2);

        features2.features.samplerAnisotropy = VK_TRUE;
        if (vulkan12Features.timelineSemaphore)   vulkan12Features.timelineSemaphore = VK_TRUE;
        if (vulkan12Features.bufferDeviceAddress) vulkan12Features.bufferDeviceAddress = VK_TRUE;

        std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = &features2;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = nullptr;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        FL_CORE_ASSERT(vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) == VK_SUCCESS,
            "Failed to create Logical Device");

        vkGetDeviceQueue(m_Device, graphicsFamily, 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_Device, presentFamily, 0, &m_PresentQueue);
        vkGetDeviceQueue(m_Device, computeFamily, 0, &m_ComputeQueue);
    }

    void VulkanDevice::CreateAllocator()
    {
        VmaAllocatorCreateInfo allocatorInfo{};
        allocatorInfo.physicalDevice = m_PhysicalDevice;
        allocatorInfo.device = m_Device;
        allocatorInfo.instance = m_Instance;
        allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;

        FL_CORE_ASSERT(vmaCreateAllocator(&allocatorInfo, &m_Allocator) == VK_SUCCESS,
            "Failed to create VMA Allocator");
    }

    void VulkanDevice::CreateDescriptorPool()
    {
        std::array<VkDescriptorPoolSize, 4> poolSizes{};
        poolSizes[0] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000 };
        poolSizes[1] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 };
        poolSizes[2] = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1000 };
        poolSizes[3] = { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1000 };

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets = 4000;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();

        FL_CORE_ASSERT(vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_DescriptorPool) == VK_SUCCESS,
            "Failed to create Descriptor Pool");
    }

    void VulkanDevice::CreateCommandLists()
    {
        VkCommandPoolCreateInfo transferPoolInfo{};
        transferPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        transferPoolInfo.queueFamilyIndex = m_GraphicsQueueFamilyIndex;
        transferPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

        FL_CORE_ASSERT(vkCreateCommandPool(m_Device, &transferPoolInfo, nullptr, &m_TransferCommandPool) == VK_SUCCESS,
            "Failed to create Transfer Command Pool");

        VkCommandPoolCreateInfo graphicsPoolInfo{};
        graphicsPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        graphicsPoolInfo.queueFamilyIndex = m_GraphicsQueueFamilyIndex;
        graphicsPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        FL_CORE_ASSERT(vkCreateCommandPool(m_Device, &graphicsPoolInfo, nullptr, &m_GraphicsCommandPool) == VK_SUCCESS,
            "Failed to create Graphics Command Pool");

        uint32_t imageCount = m_Swapchain->GetImageCount();
        m_CommandLists.reserve(imageCount);
        for (uint32_t i = 0; i < imageCount; i++)
        {
            m_CommandLists.emplace_back(
                CreateScope<VulkanCommandList>(m_Device, m_GraphicsCommandPool, m_GraphicsQueue, m_Swapchain.get()));
        }
    }

} // namespace Flux