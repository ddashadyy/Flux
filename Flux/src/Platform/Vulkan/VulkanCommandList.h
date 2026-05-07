#pragma once

#include "Flux/Renderer/RHICommandList.h"
#include "VulkanSwapchain.h"

#include <vulkan/vulkan.h>

namespace Flux {

    class VulkanCommandList final : public RHICommandList
    {
    public:
        VulkanCommandList(VkDevice device, VkCommandPool commandPool,
            VkQueue graphicsQueue, VulkanSwapchain* swapchain);
        ~VulkanCommandList();

		void* GetHandle() const override { return m_CommandBuffer; }

        void Begin() override;
        void End()   override;

        void BeginRenderPass(RHIRenderPass* renderPass,
            RHIFramebuffer* framebuffer,
            glm::vec4       clearColor = { 0.0f, 0.0f, 0.0f, 1.0f },
            float           clearDepth = 1.0f,
            uint8_t         clearStencil = 0) override;
        void EndRenderPass() override;

        void SetViewport(float x, float y, float width, float height,
            float minDepth = 0.0f, float maxDepth = 1.0f) override;
        void SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height) override;

        void SetPipeline(RHIPipeline* pipeline) override;

        void PushConstants(RHIPipeline* pipeline,
            ShaderStage  stage,
            uint32_t     offset,
            uint32_t     size,
            const void* data) override;

        void BindVertexBuffer(RHIBuffer* buffer, uint64_t offset = 0) override;
        void BindIndexBuffer(RHIBuffer* buffer,
            IndexType   indexType = IndexType::Uint32,
            uint64_t    offset = 0) override;

        void BindDescriptorSet(uint32_t          setIndex,
            RHIDescriptorSet* descriptorSet,
            RHIPipeline* pipeline) override;

        void Draw(uint32_t vertexCount, uint32_t instanceCount = 1,
            uint32_t firstVertex = 0, uint32_t firstInstance = 0) override;

        void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1,
            uint32_t firstIndex = 0, int32_t vertexOffset = 0,
            uint32_t firstInstance = 0) override;

        void DrawIndirect(RHIBuffer* argsBuffer, uint64_t offset, uint32_t drawCount) override;
        void DrawIndexedIndirect(RHIBuffer* argsBuffer, uint64_t offset, uint32_t drawCount) override;

        void Dispatch(uint32_t gx, uint32_t gy, uint32_t gz) override;
        void DispatchIndirect(RHIBuffer* argsBuffer, uint64_t offset) override;

        void CopyBuffer(RHIBuffer* src, RHIBuffer* dst,
            uint64_t size = 0, uint64_t srcOffset = 0, uint64_t dstOffset = 0) override;

        void CopyBufferToTexture(RHIBuffer* src,
            RHITexture* dst,
            const TextureRegion& region) override;

        void BlitTexture(RHITexture* src, RHITexture* dst,
            TextureRegion srcRegion, TextureRegion dstRegion,
            FilterMode    filter = FilterMode::Linear) override;

        void ResourceBarrier(RHITexture* texture,
            ResourceState oldState,
            ResourceState newState) override;

        void BufferBarrier(RHIBuffer* buffer,
            ResourceState oldState,
            ResourceState newState) override;

    private:
        VkDevice         m_Device = VK_NULL_HANDLE;
        VkQueue          m_GraphicsQueue = VK_NULL_HANDLE;
        VkCommandPool    m_CommandPool = VK_NULL_HANDLE;
        VkCommandBuffer  m_CommandBuffer = VK_NULL_HANDLE;
        VkPipelineLayout m_CurrentPipelineLayout = VK_NULL_HANDLE;

    };

} // namespace Flux
