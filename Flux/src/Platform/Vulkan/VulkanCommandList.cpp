#include "flpch.h"
#include "VulkanCommandList.h"
#include "VulkanFence.h"
#include "VulkanSemaphore.h"
#include "VulkanRenderPass.h"
#include "VulkanPipeline.h"
#include "VulkanBuffer.h"
#include "VulkanDescriptorSet.h"
#include "VulkanTexture.h"

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

    VulkanCommandList::VulkanCommandList(VkDevice device, VkQueue graphicsQueue, uint32_t queueFamilyIndex, VulkanSwapchain* swapchain)
        : m_Device(device), m_GraphicsQueue(graphicsQueue), m_Swapchain(swapchain)
    {
        // Command Pool
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndex;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        FL_CORE_ASSERT(vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool) == VK_SUCCESS,
            "Failed to create Command Pool");

        // Command Buffer
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
        vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
    }

    void VulkanCommandList::Begin()
    {
        vkResetCommandBuffer(m_CommandBuffer, 0);

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
        VkSemaphore waitSem   = waitSemaphore   ? static_cast<VulkanSemaphore*>(waitSemaphore)->GetHandle() : VK_NULL_HANDLE;
        VkSemaphore signalSem = signalSemaphore ? static_cast<VulkanSemaphore*>(signalSemaphore)->GetHandle() : VK_NULL_HANDLE;
        VkFence     vkFence   = fence           ? static_cast<VulkanFence*>(fence)->GetHandle() : VK_NULL_HANDLE;

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

    void VulkanCommandList::BeginRenderPass(RHIRenderPass* renderPass)
    {
        auto* vkRenderPass = static_cast<VulkanRenderPass*>(renderPass);

        VkRenderPassBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        beginInfo.renderPass = vkRenderPass->GetHandle();
        beginInfo.framebuffer = m_Swapchain->GetCurrentFramebuffer(); 
        beginInfo.renderArea.extent = m_Swapchain->GetExtent();          
        beginInfo.renderArea.offset = { 0, 0 };

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
        clearValues[1].depthStencil = { 1.0f, 0 };

        beginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        beginInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(m_CommandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_Swapchain->GetExtent().width);
        viewport.height = static_cast<float>(m_Swapchain->GetExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(m_CommandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = m_Swapchain->GetExtent();
        vkCmdSetScissor(m_CommandBuffer, 0, 1, &scissor);
    }

    void VulkanCommandList::EndRenderPass()
    {
        vkCmdEndRenderPass(m_CommandBuffer);
    }


    void VulkanCommandList::SetPipeline(RHIPipeline* pipeline)
    {
        auto* vkPipeline = static_cast<VulkanPipeline*>(pipeline);
        m_CurrentPipelineLayout = vkPipeline->GetLayout();

        vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline->GetHandle());
    }

    void VulkanCommandList::BindVertexBuffer(RHIBuffer* buffer)
    {
        VkBuffer     vkBuffer = static_cast<VulkanBuffer*>(buffer)->GetHandle();
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(m_CommandBuffer, 0, 1, &vkBuffer, &offset);
    }

    void VulkanCommandList::BindIndexBuffer(RHIBuffer* buffer)
    {
        VkBuffer vkBuffer = static_cast<VulkanBuffer*>(buffer)->GetHandle();
        vkCmdBindIndexBuffer(m_CommandBuffer, vkBuffer, 0, VK_INDEX_TYPE_UINT32);
    }

    void VulkanCommandList::BindDescriptorSet(RHIDescriptorSet* descriptorSet)
    {
        VkDescriptorSet set = static_cast<VulkanDescriptorSet*>(descriptorSet)->GetHandle();
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