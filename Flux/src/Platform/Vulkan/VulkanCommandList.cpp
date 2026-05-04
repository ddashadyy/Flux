#include "flpch.h"
#include "VulkanCommandList.h"
#include "VulkanRenderPass.h"
#include "VulkanPipeline.h"
#include "VulkanBuffer.h"
#include "VulkanDescriptorSet.h"
#include "VulkanTexture.h"
#include "VulkanFramebuffer.h"
#include "VulkanCommon.h"

namespace Flux {

    // -------------------------------------------------------------------------
    // Barrier helpers
    // -------------------------------------------------------------------------

    static VkImageLayout ToVkLayout(ResourceState state)
    {
        switch (state)
        {
        case ResourceState::Undefined:         return VK_IMAGE_LAYOUT_UNDEFINED;
        case ResourceState::RenderTarget:      return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case ResourceState::ShaderResource:    return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        case ResourceState::UnorderedAccess:   return VK_IMAGE_LAYOUT_GENERAL;
        case ResourceState::TransferSrc:       return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case ResourceState::TransferDst:       return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        case ResourceState::Present:           return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        case ResourceState::DepthStencilWrite: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        case ResourceState::DepthStencilRead:  return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        default: FL_CORE_ASSERT(false, "Unknown ResourceState"); return VK_IMAGE_LAYOUT_UNDEFINED;
        }
    }

    static VkAccessFlags ToVkAccess(ResourceState state)
    {
        switch (state)
        {
        case ResourceState::Undefined:         return 0;
        case ResourceState::RenderTarget:      return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        case ResourceState::ShaderResource:    return VK_ACCESS_SHADER_READ_BIT;
        case ResourceState::UnorderedAccess:   return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        case ResourceState::TransferSrc:       return VK_ACCESS_TRANSFER_READ_BIT;
        case ResourceState::TransferDst:       return VK_ACCESS_TRANSFER_WRITE_BIT;
        case ResourceState::Present:           return 0;
        case ResourceState::DepthStencilWrite: return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        case ResourceState::DepthStencilRead:  return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        case ResourceState::IndirectArgument:  return VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
        default: return 0;
        }
    }

    static VkPipelineStageFlags ToVkStage(ResourceState state)
    {
        switch (state)
        {
        case ResourceState::Undefined:         return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        case ResourceState::RenderTarget:      return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        case ResourceState::ShaderResource:    return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; 
        case ResourceState::UnorderedAccess:   return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        case ResourceState::TransferSrc:
        case ResourceState::TransferDst:       return VK_PIPELINE_STAGE_TRANSFER_BIT;
        case ResourceState::Present:           return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        case ResourceState::DepthStencilWrite: return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        case ResourceState::DepthStencilRead:  return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        case ResourceState::IndirectArgument:  return VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
        default: return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        }
    }

    static bool IsDepthState(ResourceState state)
    {
        return state == ResourceState::DepthStencilWrite || state == ResourceState::DepthStencilRead;
    }

    // -------------------------------------------------------------------------
    // Constructor / Destructor
    // -------------------------------------------------------------------------

    VulkanCommandList::VulkanCommandList(VkDevice device, VkCommandPool commandPool,
        VkQueue graphicsQueue, VulkanSwapchain* /*swapchain*/)
        : m_Device(device), m_CommandPool(commandPool), m_GraphicsQueue(graphicsQueue)
    {
        VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        allocInfo.commandPool = m_CommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        FL_CORE_ASSERT(vkAllocateCommandBuffers(m_Device, &allocInfo, &m_CommandBuffer) == VK_SUCCESS,
            "Failed to allocate Command Buffer");

        FL_CORE_INFO("Created Vulkan CommandList");
    }

    VulkanCommandList::~VulkanCommandList()
    {
        vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &m_CommandBuffer);
    }

    // -------------------------------------------------------------------------
    // Recording
    // -------------------------------------------------------------------------

    void VulkanCommandList::Begin()
    {
        FL_CORE_ASSERT(vkResetCommandBuffer(m_CommandBuffer, 0) == VK_SUCCESS,
            "Failed to reset Command Buffer");

        m_CurrentPipelineLayout = VK_NULL_HANDLE; 

        VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        FL_CORE_ASSERT(vkBeginCommandBuffer(m_CommandBuffer, &beginInfo) == VK_SUCCESS,
            "Failed to begin Command Buffer");
    }

    void VulkanCommandList::End()
    {
        FL_CORE_ASSERT(vkEndCommandBuffer(m_CommandBuffer) == VK_SUCCESS,
            "Failed to end Command Buffer");
    }

    // -------------------------------------------------------------------------
    // Render pass
    // -------------------------------------------------------------------------

    void VulkanCommandList::BeginRenderPass(RHIRenderPass* renderPass, RHIFramebuffer* framebuffer,
        glm::vec4 clearColor, float clearDepth, uint8_t clearStencil)
    {
        auto* vkPass = static_cast<VulkanRenderPass*>(renderPass);
        auto* vkFb = static_cast<VulkanFramebuffer*>(framebuffer);

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { clearColor.r, clearColor.g, clearColor.b, clearColor.a };
        clearValues[1].depthStencil = { clearDepth, clearStencil };

        const auto& desc = vkPass->GetDesc();

        uint32_t clearCount = 0;
        if (desc.ColorLoadOp == AttachmentLoadOp::Clear)
            clearCount += static_cast<uint32_t>(desc.ColorFormats.size());
        if (vkPass->HasDepthAttachment() && desc.DepthLoadOp == AttachmentLoadOp::Clear)
            clearCount += 1;

        VkRenderPassBeginInfo beginInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        beginInfo.renderPass = vkPass->GetHandle<VkRenderPass>();
        beginInfo.framebuffer = vkFb->GetHandle<VkFramebuffer>();
        beginInfo.renderArea = { { 0, 0 }, { vkFb->GetWidth(), vkFb->GetHeight() } };
        beginInfo.clearValueCount = clearCount;
        beginInfo.pClearValues = clearCount > 0 ? clearValues.data() : nullptr;

        vkCmdBeginRenderPass(m_CommandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VulkanCommandList::EndRenderPass()
    {
        vkCmdEndRenderPass(m_CommandBuffer);
    }

    // -------------------------------------------------------------------------
    // Dynamic state
    // -------------------------------------------------------------------------

    void VulkanCommandList::SetViewport(float x, float y, float width, float height,
        float minDepth, float maxDepth)
    {
        VkViewport vp{ x, y, width, height, minDepth, maxDepth };
        vkCmdSetViewport(m_CommandBuffer, 0, 1, &vp);
    }

    void VulkanCommandList::SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height)
    {
        VkRect2D scissor{ { x, y }, { width, height } };
        vkCmdSetScissor(m_CommandBuffer, 0, 1, &scissor);
    }

    // -------------------------------------------------------------------------
    // Pipeline & resources
    // -------------------------------------------------------------------------

    void VulkanCommandList::SetPipeline(RHIPipeline* pipeline)
    {
        auto* vkPipeline = static_cast<VulkanPipeline*>(pipeline);
        m_CurrentPipelineLayout = vkPipeline->GetLayout<VkPipelineLayout>();

        VkPipelineBindPoint bindPoint = (pipeline->GetType() == PipelineType::Compute)
            ? VK_PIPELINE_BIND_POINT_COMPUTE
            : VK_PIPELINE_BIND_POINT_GRAPHICS;

        vkCmdBindPipeline(m_CommandBuffer, bindPoint, vkPipeline->GetHandle<VkPipeline>());
    }

    void VulkanCommandList::PushConstants(RHIPipeline* pipeline,
        ShaderStage stage, uint32_t offset,
        uint32_t size, const void* data)
    {
        auto* vkPipeline = static_cast<VulkanPipeline*>(pipeline);
        vkCmdPushConstants(m_CommandBuffer,
            vkPipeline->GetLayout<VkPipelineLayout>(),
            GetShaderStageFlags(stage),
            offset, size, data);
    }

    void VulkanCommandList::BindVertexBuffer(RHIBuffer* buffer, uint64_t offset)
    {
        VkBuffer vkBuf = buffer->GetHandle<VkBuffer>();
        vkCmdBindVertexBuffers(m_CommandBuffer, 0, 1, &vkBuf, &offset);
    }

    void VulkanCommandList::BindIndexBuffer(RHIBuffer* buffer, IndexType indexType, uint64_t offset)
    {
        vkCmdBindIndexBuffer(m_CommandBuffer,
            buffer->GetHandle<VkBuffer>(), offset,
            indexType == IndexType::Uint16 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
    }

    void VulkanCommandList::BindDescriptorSet(uint32_t setIndex,
        RHIDescriptorSet* descriptorSet,
        RHIPipeline* pipeline)
    {
        auto* vkPipeline = static_cast<VulkanPipeline*>(pipeline);
        VkPipelineLayout layout = vkPipeline->GetLayout<VkPipelineLayout>();

        VkPipelineBindPoint bindPoint = (pipeline->GetType() == PipelineType::Compute)
            ? VK_PIPELINE_BIND_POINT_COMPUTE
            : VK_PIPELINE_BIND_POINT_GRAPHICS;

        auto set = descriptorSet->GetHandle<VkDescriptorSet>();
        vkCmdBindDescriptorSets(m_CommandBuffer, bindPoint, layout, setIndex, 1, &set, 0, nullptr);
    }

    // -------------------------------------------------------------------------
    // Draw calls
    // -------------------------------------------------------------------------

    void VulkanCommandList::Draw(uint32_t vertexCount, uint32_t instanceCount,
        uint32_t firstVertex, uint32_t firstInstance)
    {
        vkCmdDraw(m_CommandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void VulkanCommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount,
        uint32_t firstIndex, int32_t vertexOffset,
        uint32_t firstInstance)
    {
        vkCmdDrawIndexed(m_CommandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void VulkanCommandList::DrawIndirect(RHIBuffer* argsBuffer, uint64_t offset, uint32_t drawCount)
    {
        vkCmdDrawIndirect(m_CommandBuffer,
            argsBuffer->GetHandle<VkBuffer>(), offset, drawCount,
            sizeof(VkDrawIndirectCommand));
    }

    void VulkanCommandList::DrawIndexedIndirect(RHIBuffer* argsBuffer, uint64_t offset, uint32_t drawCount)
    {
        vkCmdDrawIndexedIndirect(m_CommandBuffer,
            argsBuffer->GetHandle<VkBuffer>(), offset, drawCount,
            sizeof(VkDrawIndexedIndirectCommand));
    }

    // -------------------------------------------------------------------------
    // Compute
    // -------------------------------------------------------------------------

    void VulkanCommandList::Dispatch(uint32_t gx, uint32_t gy, uint32_t gz)
    {
        vkCmdDispatch(m_CommandBuffer, gx, gy, gz);
    }

    void VulkanCommandList::DispatchIndirect(RHIBuffer* argsBuffer, uint64_t offset)
    {
        vkCmdDispatchIndirect(m_CommandBuffer, argsBuffer->GetHandle<VkBuffer>(), offset);
    }

    // -------------------------------------------------------------------------
    // Transfers
    // -------------------------------------------------------------------------

    void VulkanCommandList::CopyBuffer(RHIBuffer* src, RHIBuffer* dst,
        uint64_t size, uint64_t srcOffset, uint64_t dstOffset)
    {
        VkBufferCopy region{};
        region.srcOffset = srcOffset;
        region.dstOffset = dstOffset;
        region.size = size == 0 ? src->GetSize() : size;
        vkCmdCopyBuffer(m_CommandBuffer,
            src->GetHandle<VkBuffer>(), dst->GetHandle<VkBuffer>(), 1, &region);
    }

    void VulkanCommandList::CopyBufferToTexture(RHIBuffer* src, RHITexture* dst,
        const TextureRegion& region)
    {
        auto* vkTexture = static_cast<VulkanTexture*>(dst);

        VkBufferImageCopy copy{};
        copy.bufferOffset = 0;
        copy.bufferRowLength = 0;
        copy.bufferImageHeight = 0;
        copy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, region.MipLevel, region.ArrayLayer, 1 };
        copy.imageOffset = { (int32_t)region.X, (int32_t)region.Y, (int32_t)region.Z };
        copy.imageExtent = { region.Width, region.Height, region.Depth };

        vkCmdCopyBufferToImage(m_CommandBuffer,
            src->GetHandle<VkBuffer>(),
            vkTexture->GetImage(),
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &copy);
    }

    void VulkanCommandList::BlitTexture(RHITexture* src, RHITexture* dst,
        TextureRegion srcRegion, TextureRegion dstRegion,
        FilterMode filter)
    {
        auto* vkSrc = static_cast<VulkanTexture*>(src);
        auto* vkDst = static_cast<VulkanTexture*>(dst);

        VkImageBlit blit{};
        blit.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, srcRegion.MipLevel, srcRegion.ArrayLayer, 1 };
        blit.srcOffsets[0] = { (int32_t)srcRegion.X, (int32_t)srcRegion.Y, 0 };
        blit.srcOffsets[1] = { (int32_t)(srcRegion.X + srcRegion.Width), (int32_t)(srcRegion.Y + srcRegion.Height), 1 };
        blit.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, dstRegion.MipLevel, dstRegion.ArrayLayer, 1 };
        blit.dstOffsets[0] = { (int32_t)dstRegion.X, (int32_t)dstRegion.Y, 0 };
        blit.dstOffsets[1] = { (int32_t)(dstRegion.X + dstRegion.Width), (int32_t)(dstRegion.Y + dstRegion.Height), 1 };

        VkFilter vkFilter = (filter == FilterMode::Linear) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;

        vkCmdBlitImage(m_CommandBuffer,
            vkSrc->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            vkDst->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blit, vkFilter);
    }

    // -------------------------------------------------------------------------
    // Barriers
    // -------------------------------------------------------------------------

    void VulkanCommandList::ResourceBarrier(RHITexture* texture,
        ResourceState oldState, ResourceState newState)
    {
        auto* vkTexture = static_cast<VulkanTexture*>(texture);

        // Правильный aspect — depth или color в зависимости от состояния
        const bool isDepth = IsDepthState(oldState) || IsDepthState(newState);

        VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
        barrier.oldLayout = ToVkLayout(oldState);
        barrier.newLayout = ToVkLayout(newState);
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = vkTexture->GetImage();
        barrier.srcAccessMask = ToVkAccess(oldState);
        barrier.dstAccessMask = ToVkAccess(newState);
        barrier.subresourceRange.aspectMask = isDepth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = texture->GetMipLevels();
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        vkCmdPipelineBarrier(m_CommandBuffer,
            ToVkStage(oldState), ToVkStage(newState),
            0, 0, nullptr, 0, nullptr, 1, &barrier);
    }

    void VulkanCommandList::BufferBarrier(RHIBuffer* buffer,
        ResourceState /*oldState*/, ResourceState /*newState*/)
    {
        // Для storage buffer compute → graphics или graphics → compute
        VkBufferMemoryBarrier barrier{ VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER };
        barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.buffer = buffer->GetHandle<VkBuffer>();
        barrier.offset = 0;
        barrier.size = VK_WHOLE_SIZE;

        vkCmdPipelineBarrier(m_CommandBuffer,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0, 0, nullptr, 1, &barrier, 0, nullptr);
    }

} // namespace Flux
