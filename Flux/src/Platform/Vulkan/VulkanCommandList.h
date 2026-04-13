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

        void BeginRenderPass(RHIRenderPass* renderPass, RHIFramebuffer* framebuffer,
            glm::vec4 clearColor = { 0.0f, 0.0f, 0.0f, 1.0f }) override;
        void EndRenderPass() override;

        void SetViewport(float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f) override;
        void SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height) override;

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

    };
}