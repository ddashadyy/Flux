#include "flpch.h"
#include "VulkanCommandList.h"
#include "VulkanFence.h"
#include "VulkanSemaphore.h"
#include "VulkanRenderPass.h"
#include "VulkanPipeline.h"
#include "VulkanBuffer.h"
#include "VulkanDescriptorSet.h"
#include "VulkanTexture.h"
#include "VulkanFramebuffer.h"

namespace Flux {


    static VkImageLayout GetImageLayout(ResourceState state)
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
        }

        FL_CORE_ASSERT(false, "Unknown Resource State!");
        return VK_IMAGE_LAYOUT_UNDEFINED;
    }

    static VkAccessFlags GetAccessMask(ResourceState state)
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
        case ResourceState::DepthStencilWrite: return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        case ResourceState::DepthStencilRead:  return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        }

        FL_CORE_ASSERT(false, "Unknown Resource State!");
        return 0;
    }

    static VkPipelineStageFlags GetPipelineStage(ResourceState state)
    {
        switch (state)
        {
        case ResourceState::Undefined:         return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        case ResourceState::RenderTarget:      return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        case ResourceState::ShaderResource:    return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        case ResourceState::UnorderedAccess:   return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        case ResourceState::TransferSrc:       return VK_PIPELINE_STAGE_TRANSFER_BIT;
        case ResourceState::TransferDst:       return VK_PIPELINE_STAGE_TRANSFER_BIT;
        case ResourceState::Present:           return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        case ResourceState::DepthStencilWrite: return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        case ResourceState::DepthStencilRead:  return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }

        FL_CORE_ASSERT(false, "Unknown Resource State!");
        return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }

    static VkShaderStageFlags GetShaderStageFlags(ShaderStage stage)
    {
        VkShaderStageFlags flags = 0;

        if (HasFlag(stage, ShaderStage::Vertex))
            flags |= VK_SHADER_STAGE_VERTEX_BIT;

        if (HasFlag(stage, ShaderStage::Fragment))
            flags |= VK_SHADER_STAGE_FRAGMENT_BIT;

        if (HasFlag(stage, ShaderStage::Compute))
            flags |= VK_SHADER_STAGE_COMPUTE_BIT;

        if (flags == 0)
        {
            FL_CORE_ASSERT(false, "No Shader Stage specified!");
        }

        return flags;

    }

    VulkanCommandList::VulkanCommandList(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VulkanSwapchain* swapchain)
        : m_Device(device), m_CommandPool(commandPool), m_GraphicsQueue(graphicsQueue)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
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

    void VulkanCommandList::Begin()
    {
        FL_CORE_ASSERT(vkResetCommandBuffer(m_CommandBuffer, 0) == VK_SUCCESS, "Failed to reset Command Buffer!");

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        FL_CORE_ASSERT(vkBeginCommandBuffer(m_CommandBuffer, &beginInfo) == VK_SUCCESS,
            "Failed to begin Command Buffer");
    }

    void VulkanCommandList::End()
    {
        FL_CORE_ASSERT(vkEndCommandBuffer(m_CommandBuffer) == VK_SUCCESS,
            "Failed to end Command Buffer");
    }

    void VulkanCommandList::Submit(RHIFence* fence, RHISemaphore* waitSemaphore, RHISemaphore* signalSemaphore)
    {
        VkSemaphore waitSem   = waitSemaphore   ? waitSemaphore->GetHandle<VkSemaphore>() : VK_NULL_HANDLE;
        VkSemaphore signalSem = signalSemaphore ? signalSemaphore->GetHandle<VkSemaphore>() : VK_NULL_HANDLE;
        VkFence     vkFence   = fence           ? fence->GetHandle<VkFence>() : VK_NULL_HANDLE;

        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = waitSem != VK_NULL_HANDLE ? 1 : 0;
        submitInfo.pWaitSemaphores = waitSem != VK_NULL_HANDLE ? &waitSem : nullptr;
        submitInfo.pWaitDstStageMask = waitSem != VK_NULL_HANDLE ? &waitStage : nullptr;
        submitInfo.signalSemaphoreCount = signalSem != VK_NULL_HANDLE ? 1 : 0;
        submitInfo.pSignalSemaphores = signalSem != VK_NULL_HANDLE ? &signalSem : nullptr;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_CommandBuffer;

        FL_CORE_ASSERT(vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, vkFence) == VK_SUCCESS,
            "Failed to submit Command Buffer");
    }

    
    void VulkanCommandList::BeginRenderPass(RHIRenderPass* renderPass, RHIFramebuffer* framebuffer, glm::vec4 clearColor)
    {
        auto* vkPass = static_cast<VulkanRenderPass*>(renderPass);
        auto* vkFb = static_cast<VulkanFramebuffer*>(framebuffer);

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { clearColor.r, clearColor.g, clearColor.b, clearColor.a };
        clearValues[1].depthStencil = { 1.0f, 0 };

        VkRenderPassBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        beginInfo.renderPass = vkPass->GetHandle<VkRenderPass>();
        beginInfo.framebuffer = vkFb->GetHandle<VkFramebuffer>();
        beginInfo.renderArea.offset = { 0, 0 };
        beginInfo.renderArea.extent = { vkFb->GetWidth(), vkFb->GetHeight() };
        beginInfo.clearValueCount = vkPass->HasDepthAttachment() ? 2 : 1;
        beginInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(m_CommandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VulkanCommandList::EndRenderPass()
    {
        vkCmdEndRenderPass(m_CommandBuffer);
    }

    void VulkanCommandList::SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
    {
        VkViewport viewport{};
        viewport.x = x;
		viewport.y = y;
        viewport.width = width;
		viewport.height = height;
		viewport.minDepth = minDepth;
		viewport.maxDepth = maxDepth;

		vkCmdSetViewport(m_CommandBuffer, 0, 1, &viewport);
    }

    void VulkanCommandList::SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height)
    {
		VkRect2D scissor{};
        scissor.offset = { x, y };
        scissor.extent = { width, height };

		vkCmdSetScissor(m_CommandBuffer, 0, 1, &scissor);
    }


    void VulkanCommandList::SetPipeline(RHIPipeline* pipeline)
    {
        auto* vkPipeline = static_cast<VulkanPipeline*>(pipeline);
        m_CurrentPipelineLayout = vkPipeline->GetLayout<VkPipelineLayout>();

        vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline->GetHandle<VkPipeline>());
    }

    void VulkanCommandList::PushConstants(RHIPipeline* pipeline, const void* pushConstants)
    {
        auto* vkPipeline = static_cast<VulkanPipeline*>(pipeline);
        auto pipelineLayoutDesc = vkPipeline->GetLayoutDesc();

        vkCmdPushConstants(
            m_CommandBuffer,
            vkPipeline->GetLayout<VkPipelineLayout>(),
            GetShaderStageFlags(pipelineLayoutDesc.Stage),
            pipelineLayoutDesc.Offset,
            pipelineLayoutDesc.Size,
            pushConstants
        );
    }

    void VulkanCommandList::BindVertexBuffer(RHIBuffer* buffer)
    {
        auto vkBuffer = buffer->GetHandle<VkBuffer>();
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(m_CommandBuffer, 0, 1, &vkBuffer, &offset);
    }

    void VulkanCommandList::BindIndexBuffer(RHIBuffer* buffer)
    {
        auto vkBuffer = buffer->GetHandle<VkBuffer>();
        vkCmdBindIndexBuffer(m_CommandBuffer, vkBuffer, 0, VK_INDEX_TYPE_UINT32);
    }

    void VulkanCommandList::BindDescriptorSet(RHIDescriptorSet* descriptorSet)
    {
        auto set = descriptorSet->GetHandle<VkDescriptorSet>();
        vkCmdBindDescriptorSets(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_CurrentPipelineLayout, 0, 1, &set, 0, nullptr);
    }

    void VulkanCommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount)
    {
        vkCmdDrawIndexed(m_CommandBuffer, indexCount, instanceCount, 0, 0, 0);
    }

    void VulkanCommandList::Dispatch(uint32_t gx, uint32_t gy, uint32_t gz)
    {
        vkCmdDispatch(m_CommandBuffer, gx, gy, gz);
    }

    void VulkanCommandList::ResourceBarrier(RHITexture* texture, ResourceState oldState, ResourceState newState)
    {
        auto* vkTexture = static_cast<VulkanTexture*>(texture);

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = GetImageLayout(oldState);
        barrier.newLayout = GetImageLayout(newState);
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = vkTexture->GetImage();
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        barrier.srcAccessMask = GetAccessMask(oldState);
        barrier.dstAccessMask = GetAccessMask(newState);

        vkCmdPipelineBarrier(m_CommandBuffer,
            GetPipelineStage(oldState),
            GetPipelineStage(newState),
            0, 0, nullptr, 0, nullptr,
            1, &barrier);
    }
}