#pragma once


#include "FLux/Renderer/RHIDevice.h"

#include "VulkanSwapchain.h"
#include "VulkanCommandList.h"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace Flux {

    class VulkanDevice : public RHIDevice {
    public:
        VulkanDevice(void* windowHandle, uint32_t width, uint32_t height);
        ~VulkanDevice() override;

        Scope<RHIBuffer>              CreateBuffer(const BufferSpec& spec)                       override;
        Scope<RHITexture>             CreateTexture(const TextureSpec& spec)                     override;
        Scope<RHIFramebuffer>         CreateFramebuffer(const FramebufferSpec& spec)             override;
        Scope<RHIPipeline>            CreatePipeline(const PipelineDesc& spec)                   override;
        Scope<RHIRenderPass>          CreateRenderPass(const RenderPassDesc& spec)               override;
        Scope<RHIShader>              CreateShader(ShaderStage stage, const std::vector<uint32_t>& spirv) override;
        Scope<RHIFence>               CreateFence(bool signaled = false)                         override;
        Scope<RHISemaphore>           CreateSemaphore()                                          override;
        Scope<RHIDescriptorSetLayout> CreateDescriptorSetLayout(const DescriptorSetLayoutDesc& desc) override;
        Scope<RHIDescriptorSet>       CreateDescriptorSet(const RHIDescriptorSetLayout* layout)  override;

        inline RHICommandList* GetCommandList(uint32_t index = 0) override { return m_CommandLists[index].get(); }
        inline RHISwapchain* GetSwapchain()     override { return m_Swapchain.get(); }

        DeviceMemoryStats GetMemoryStatistics() const override;

        VkCommandBuffer BeginSingleTimeCommands();
        void            EndSingleTimeCommands(VkCommandBuffer cmd);

        inline VkDevice         GetHandle()          const { return m_Device; }
        inline VkPhysicalDevice GetPhysicalDevice()  const { return m_PhysicalDevice; }
        inline VmaAllocator     GetAllocator()       const { return m_Allocator; }
        inline VkDescriptorPool GetDescriptorPool()  const { return m_DescriptorPool; }

        inline VkInstance  GetInstance()            const { return m_Instance; }
        inline uint32_t    GetGraphicsQueueFamily() const { return m_GraphicsQueueFamilyIndex; }
        inline VkQueue     GetGraphicsQueue()       const { return m_GraphicsQueue; }

    private:
        void CreateInstance();
        void CreateDebugMessenger();
        void CreateSurface(void* windowHandle);
        void PickPhysicalDevice();
        void CreateLogicalDevice();
        void CreateAllocator();
        void CreateDescriptorPool();
        void CreateCommandList();

    private:
        VkInstance               m_Instance       = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
        VkSurfaceKHR             m_Surface        = VK_NULL_HANDLE;
        VkPhysicalDevice         m_PhysicalDevice = VK_NULL_HANDLE;
        VkDevice                 m_Device         = VK_NULL_HANDLE;

        VkQueue  m_GraphicsQueue            = VK_NULL_HANDLE;
        VkQueue  m_PresentQueue             = VK_NULL_HANDLE;
        VkQueue  m_ComputeQueue             = VK_NULL_HANDLE;
        uint32_t m_GraphicsQueueFamilyIndex = 0;

        VmaAllocator     m_Allocator           = nullptr;
        VkDescriptorPool m_DescriptorPool      = VK_NULL_HANDLE;
        VkCommandPool    m_TransferCommandPool = VK_NULL_HANDLE;
        VkCommandPool    m_GraphicsCommandPool = VK_NULL_HANDLE;

        Scope<VulkanSwapchain>   m_Swapchain;
        std::vector<Scope<VulkanCommandList>> m_CommandLists;
    };

}