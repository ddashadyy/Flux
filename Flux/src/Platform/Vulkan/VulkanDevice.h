#pragma once

#include "Flux/Renderer/RHIDevice.h"

#include "VulkanSwapchain.h"
#include "VulkanCommandList.h"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace Flux {

    class VulkanDevice final : public RHIDevice
    {
    public:
        VulkanDevice(void* windowHandle, uint32_t width, uint32_t height);
        ~VulkanDevice() override;

		void* GetHandle() const override { return m_Device; }

        // -----------------------------------------------------------------
        // Resource creation
        // -----------------------------------------------------------------
        Scope<RHIBuffer>              CreateBuffer(const BufferSpec& spec)                              override;
        Scope<RHITexture>             CreateTexture(const TextureSpec& spec)                            override;
        Scope<RHISampler>             CreateSampler(const SamplerSpec& spec)                            override;
        Scope<RHIFramebuffer>         CreateFramebuffer(const FramebufferSpec& spec)                    override;
        Scope<RHIPipeline>            CreatePipeline(const PipelineDesc& desc)                          override;
        Scope<RHIRenderPass>          CreateRenderPass(const RenderPassDesc& desc)                      override;
        Scope<RHIShader>              CreateShader(ShaderStage stage, const std::vector<uint32_t>& spirv) override;
        Scope<RHIFence>               CreateFence(bool signaled = false)                                override;
        Scope<RHISemaphore>           CreateSemaphore()                                                 override;
        Scope<RHIDescriptorSetLayout> CreateDescriptorSetLayout(const DescriptorSetLayoutDesc& desc)    override;
        Scope<RHIDescriptorSet>       CreateDescriptorSet(const RHIDescriptorSetLayout* layout)         override;

        // -----------------------------------------------------------------
        // Command lists
        // -----------------------------------------------------------------
        RHICommandList* GetCommandList(uint32_t index = 0) override { return m_CommandLists[index].get(); }
        RHISwapchain* GetSwapchain()                     override { return m_Swapchain.get(); }

        // -----------------------------------------------------------------
        // Submit — перенесён из CommandList
        // -----------------------------------------------------------------
        void Submit(const SubmitDesc& desc)                                    override;
        void ImmediateSubmit(std::function<void(RHICommandList*)>&& fn)        override;

        // -----------------------------------------------------------------
        // Utils
        // -----------------------------------------------------------------
        void CopyBuffer(RHIBuffer* src, RHIBuffer* dst,
            uint64_t size = 0, uint64_t srcOffset = 0, uint64_t dstOffset = 0) const override;

        DeviceMemoryStats GetMemoryStatistics() const override;
        void              WaitIdle()            const override;

        // -----------------------------------------------------------------
        // Vulkan-specific accessors (для внутреннего использования)
        // -----------------------------------------------------------------
        VkPhysicalDevice GetPhysicalDevice()   const { return m_PhysicalDevice; }
        VmaAllocator     GetAllocator()        const { return m_Allocator; }
        VkDescriptorPool GetDescriptorPool()   const { return m_DescriptorPool; }
        VkInstance       GetInstance()         const { return m_Instance; }
        uint32_t         GetGraphicsFamily()   const { return m_GraphicsQueueFamilyIndex; }
        VkQueue          GetGraphicsQueue()    const { return m_GraphicsQueue; }
        VkCommandPool    GetTransferPool()     const { return m_TransferCommandPool; }

    private:
        void CreateInstance();
        void CreateDebugMessenger();
        void CreateSurface(void* windowHandle);
        void PickPhysicalDevice();
        void CreateLogicalDevice();
        void CreateAllocator();
        void CreateDescriptorPool();
        void CreateCommandLists();

    private:
        VkInstance               m_Instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
        VkSurfaceKHR             m_Surface = VK_NULL_HANDLE;
        VkPhysicalDevice         m_PhysicalDevice = VK_NULL_HANDLE;
        VkDevice                 m_Device = VK_NULL_HANDLE;

        VkQueue  m_GraphicsQueue = VK_NULL_HANDLE;
        VkQueue  m_PresentQueue = VK_NULL_HANDLE;
        VkQueue  m_ComputeQueue = VK_NULL_HANDLE;
        uint32_t m_GraphicsQueueFamilyIndex = 0;

        VmaAllocator     m_Allocator = nullptr;
        VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
        VkCommandPool    m_TransferCommandPool = VK_NULL_HANDLE;
        VkCommandPool    m_GraphicsCommandPool = VK_NULL_HANDLE;

        Scope<VulkanSwapchain>                m_Swapchain;
        std::vector<Scope<VulkanCommandList>> m_CommandLists;

        VkCommandBuffer m_ImmediateCmd = VK_NULL_HANDLE;
        VkFence         m_ImmediateFence = VK_NULL_HANDLE;

    };

} // namespace Flux
