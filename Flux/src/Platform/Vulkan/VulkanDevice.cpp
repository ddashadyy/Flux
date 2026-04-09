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
        FL_CORE_ERROR("Vulkan Validation: {0}", data->pMessage);
        return VK_FALSE;
    }

    VulkanDevice::VulkanDevice(void* windowHandle, uint32_t width, uint32_t height)
    {
        CreateInstance();
        CreateDebugMessenger();
        CreateSurface(windowHandle);
        PickPhysicalDevice();
        CreateLogicalDevice();
        CreateAllocator();
        CreateDescriptorPool();

        m_Swapchain = CreateScope<VulkanSwapchain>(m_Device, m_PhysicalDevice,
            m_Surface, m_PresentQueue,
            width, height
        );

        CreateCommandList();

        FL_CORE_INFO("Vulkan Device initialized");
    }

    VulkanDevice::~VulkanDevice()
    {
        vkDeviceWaitIdle(m_Device);

        m_CommandList.reset();
        m_Swapchain.reset();

        vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
        vkDestroyCommandPool(m_Device, m_TransferCommandPool, nullptr);
        vmaDestroyAllocator(m_Allocator);
        vkDestroyDevice(m_Device, nullptr);
        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);

#ifdef FL_DEBUG
        auto vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT");
        if (vkDestroyDebugUtilsMessengerEXT)
            vkDestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
#endif

        vkDestroyInstance(m_Instance, nullptr);
    }

    void VulkanDevice::CreateInstance()
    {
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
        FL_CORE_ASSERT(glfwCreateWindowSurface(m_Instance, (GLFWwindow*)windowHandle, nullptr, &m_Surface) == VK_SUCCESS,
            "Failed to create GLFW Surface");
    }

    void VulkanDevice::PickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
        FL_CORE_ASSERT(deviceCount > 0, "No Vulkan-capable GPU found!");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

        // берём первый дискретный GPU
        for (auto& device : devices)
        {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(device, &props);

            if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                m_PhysicalDevice = device;
                FL_CORE_INFO("Selected GPU: {0}", props.deviceName);
                return;
            }
        }

        // если дискретного нет — берём первый доступный
        m_PhysicalDevice = devices[0];
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(m_PhysicalDevice, &props);
        FL_CORE_INFO("Selected GPU: {0}", props.deviceName);
    }

    void VulkanDevice::CreateLogicalDevice()
    {
        // находим graphics queue family
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, queueFamilies.data());

        uint32_t graphicsFamily = UINT32_MAX;
        uint32_t presentFamily = UINT32_MAX;
        uint32_t computeFamily = UINT32_MAX;

        for (uint32_t i = 0; i < queueFamilyCount; i++)
        {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                graphicsFamily = i;

            if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
                computeFamily = i;

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, i, m_Surface, &presentSupport);
            if (presentSupport)
                presentFamily = i;

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

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
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

    void VulkanDevice::CreateCommandList()
    {
        // создаём transfer command pool
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = m_GraphicsQueueFamilyIndex;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

        FL_CORE_ASSERT(vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_TransferCommandPool) == VK_SUCCESS,
            "Failed to create Transfer Command Pool");

        m_CommandList = CreateScope<VulkanCommandList>(m_Device, m_GraphicsQueue,
            m_GraphicsQueueFamilyIndex,
            m_Swapchain.get());
    }

    Scope<RHIBuffer> VulkanDevice::CreateBuffer(const BufferSpec& spec)
    {
        return CreateScope<VulkanBuffer>(m_Allocator, spec);
    }

    Scope<RHITexture> VulkanDevice::CreateTexture(const TextureSpec& spec)
    {
        return CreateScope<VulkanTexture>(m_Device, m_GraphicsQueue, m_TransferCommandPool, m_Allocator, spec);
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
        auto* vkLayout = static_cast<const VulkanDescriptorSetLayout*>(layout);
        return CreateScope<VulkanDescriptorSet>(m_Device, m_DescriptorPool, vkLayout->GetHandle());
    }

    VkCommandBuffer VulkanDevice::BeginSingleTimeCommands()
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_TransferCommandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer cmd;
        vkAllocateCommandBuffers(m_Device, &allocInfo, &cmd);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(cmd, &beginInfo);
        return cmd;
    }

    void VulkanDevice::EndSingleTimeCommands(VkCommandBuffer cmd)
    {
        vkEndCommandBuffer(cmd);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;

        vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_GraphicsQueue);

        vkFreeCommandBuffers(m_Device, m_TransferCommandPool, 1, &cmd);
    }

}