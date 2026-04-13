#pragma once


#include "Flux/Renderer/RHICommandList.h"
#include "VulkanSwapchain.h"

#include <vulkan/vulkan.h>

namespace Flux {

    class VulkanCommandList final : public RHICommandList 
    {
    public:
        VulkanCommandList(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VulkanSwapchain* swapchain);
        ~VulkanCommandList();

        inline VkCommandBuffer GetHandle() const { return m_CommandBuffer; }

        void Begin() override;
        void End() override;

        void Submit(RHIFence* fence,
            RHISemaphore* waitSemaphore = nullptr,
            RHISemaphore* signalSemaphore = nullptr) override;

        void BeginRenderPass(RHIRenderPass* renderPass, uint32_t imageIndex) override;
        void BeginRenderPass(RHIRenderPass* renderPass, RHIFramebuffer* framebuffer) override;
        void EndRenderPass() override;

        void SetPipeline(RHIPipeline* pipeline) override;
        void PushConstants(RHIPipeline* pipeline, const void* pushConstants) override;

        void BindVertexBuffer(RHIBuffer* buffer) override;
        void BindIndexBuffer(RHIBuffer* buffer) override;

        void BindDescriptorSet(RHIDescriptorSet* descriptorSet) override;

        void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1) override;
        void Dispatch(uint32_t gx, uint32_t gy, uint32_t gz) override;

        void ResourceBarrier(RHITexture* texture,
            ResourceState oldState,
            ResourceState newState) override;

    private:
        VkDevice         m_Device = VK_NULL_HANDLE;
        VkQueue          m_GraphicsQueue = VK_NULL_HANDLE;
        VkCommandPool    m_CommandPool = VK_NULL_HANDLE;
        VkCommandBuffer  m_CommandBuffer = VK_NULL_HANDLE;
        VkPipelineLayout m_CurrentPipelineLayout = VK_NULL_HANDLE;

        VulkanSwapchain* m_Swapchain = nullptr;
    };
}