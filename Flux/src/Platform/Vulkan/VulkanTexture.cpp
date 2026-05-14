#include "flpch.h"
#include "VulkanTexture.h"
#include "VulkanBuffer.h"
#include "VulkanCommon.h"

#include <functional>

namespace {

    using Flux::TextureUsage;

    constexpr VkImageUsageFlags GetImageUsageFlags(TextureUsage usage)
    {
        using enum TextureUsage;

        VkImageUsageFlags flags = 0;

        if (HasFlag(usage, Sampled))
            flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
        if (HasFlag(usage, RenderTarget))
            flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if (HasFlag(usage, DepthStencil))
            flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        if (HasFlag(usage, Storage))
            flags |= VK_IMAGE_USAGE_STORAGE_BIT;
        if (HasFlag(usage, TransferSrc))
            flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        if (HasFlag(usage, TransferDst))
            flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        FL_CONSTEXPR_ASSERT(flags != 0, "TextureUsage resulted in empty VkImageUsageFlags!");

        return flags;
    }

    constexpr VkImageAspectFlags GetImageAspect(TextureUsage usage)
    {
        using enum TextureUsage;

        if (HasFlag(usage, DepthStencil))
            return VK_IMAGE_ASPECT_DEPTH_BIT;

        return VK_IMAGE_ASPECT_COLOR_BIT;
    }

} // anonymous namespace

namespace Flux {

    // -------------------------------------------------------------------------
    // VulkanTexture
    // -------------------------------------------------------------------------

    VulkanTexture::VulkanTexture(VkDevice device, VmaAllocator allocator, const TextureSpec& spec)
        : m_Device(device), m_Allocator(allocator), m_Spec(spec)
    {
        if (m_Spec.GenerateMipmaps)
        {
            m_Spec.MipLevels = static_cast<uint32_t>(
                std::floor(std::log2(std::max(m_Spec.Width, m_Spec.Height)))) + 1;
            m_Spec.Usage = m_Spec.Usage | TextureUsage::TransferSrc | TextureUsage::TransferDst;
        }

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = GetVkFormat(m_Spec.ImageFormat);
        imageInfo.extent = { m_Spec.Width, m_Spec.Height, m_Spec.Depth };
        imageInfo.mipLevels = m_Spec.MipLevels;
        imageInfo.arrayLayers = m_Spec.ArrayLayers;
        imageInfo.samples = GetSampleCount(m_Spec.Samples);
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = GetImageUsageFlags(m_Spec.Usage);
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        FL_CORE_ASSERT(vmaCreateImage(m_Allocator, &imageInfo, &allocInfo,
            &m_Image, &m_Allocation, nullptr) == VK_SUCCESS,
            "Failed to create Vulkan Image");

        // Image view
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_Image;
        viewInfo.viewType = (m_Spec.ArrayLayers > 1) ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = GetVkFormat(m_Spec.ImageFormat);
        viewInfo.subresourceRange.aspectMask = GetImageAspect(m_Spec.Usage);
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = m_Spec.MipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = m_Spec.ArrayLayers;

        FL_CORE_ASSERT(vkCreateImageView(m_Device, &viewInfo, nullptr, &m_ImageView) == VK_SUCCESS,
            "Failed to create Image View");

        FL_CORE_INFO("Created Vulkan Texture {}x{} mips={}", m_Spec.Width, m_Spec.Height, m_Spec.MipLevels);
    }

    VulkanTexture::~VulkanTexture()
    {
        vkDestroyImageView(m_Device, m_ImageView, nullptr);
        vmaDestroyImage(m_Allocator, m_Image, m_Allocation);
    }

    // -------------------------------------------------------------------------
    // Upload (staging → image + mip generation)
    // -------------------------------------------------------------------------

    void VulkanTexture::RecordUpload(VkCommandBuffer cmd, VkBuffer stagingBuffer)
    {
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { m_Spec.Width, m_Spec.Height, 1 };

        vkCmdCopyBufferToImage(cmd, stagingBuffer, m_Image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    }

    void VulkanTexture::RecordBarrier(VkCommandBuffer cmd,
        VkImageLayout oldLayout, VkImageLayout newLayout,
        VkAccessFlags srcAccess, VkAccessFlags dstAccess,
        VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage)
    {
        const bool isDepth = static_cast<uint8_t>(m_Spec.Usage & TextureUsage::DepthStencil) != 0;

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_Image;
        barrier.srcAccessMask = srcAccess;
        barrier.dstAccessMask = dstAccess;
        barrier.subresourceRange.aspectMask = isDepth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = m_Spec.MipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = m_Spec.ArrayLayers;

        vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0,
            0, nullptr, 0, nullptr, 1, &barrier);
    }

    void VulkanTexture::RecordGenerateMipmaps(VkCommandBuffer cmd)
    {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = m_Image;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        int32_t mipW = static_cast<int32_t>(m_Spec.Width);
        int32_t mipH = static_cast<int32_t>(m_Spec.Height);

        for (uint32_t i = 1; i < m_Spec.MipLevels; i++)
        {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            vkCmdPipelineBarrier(cmd,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                0, 0, nullptr, 0, nullptr, 1, &barrier);

            VkImageBlit blit{};
            blit.srcOffsets[0] = { 0, 0, 0 };
            blit.srcOffsets[1] = { mipW, mipH, 1 };
            blit.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, i - 1, 0, 1 };
            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = { mipW > 1 ? mipW / 2 : 1, mipH > 1 ? mipH / 2 : 1, 1 };
            blit.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, i, 0, 1 };

            vkCmdBlitImage(cmd,
                m_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &blit, VK_FILTER_LINEAR);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            vkCmdPipelineBarrier(cmd,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0, 0, nullptr, 0, nullptr, 1, &barrier);

            if (mipW > 1) mipW /= 2;
            if (mipH > 1) mipH /= 2;
        }

        barrier.subresourceRange.baseMipLevel = m_Spec.MipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(cmd,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0, 0, nullptr, 0, nullptr, 1, &barrier);
    }

    // -------------------------------------------------------------------------
    // SetData — создаёт staging, submit через локальный fence
    // В идеале вызывающий код использует Device::ImmediateSubmit.
    // -------------------------------------------------------------------------
    void VulkanTexture::SetData(const void* data, uint32_t size)
    {
        FL_CORE_ASSERT(m_GraphicsQueue != VK_NULL_HANDLE && m_CommandPool != VK_NULL_HANDLE,
            "VulkanTexture::SetData requires GraphicsQueue and CommandPool — use Device::ImmediateSubmit instead!");

        BufferSpec stagingSpec{};
        stagingSpec.Size = size;
        stagingSpec.Usage = BufferUsage::Staging;
        stagingSpec.CpuVisible = true;

        VulkanBuffer staging(m_Allocator, stagingSpec);
        staging.SetData(data, size);

        SubmitOneTime([&](VkCommandBuffer cmd) {
            RecordBarrier(cmd,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                0, VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT);

            RecordUpload(cmd, static_cast<VkBuffer>(staging.GetHandle()));

            if (m_Spec.MipLevels > 1)
                RecordGenerateMipmaps(cmd);
            else
                RecordBarrier(cmd,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    VK_ACCESS_TRANSFER_WRITE_BIT,
                    VK_ACCESS_SHADER_READ_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
            });
    }

    void VulkanTexture::SubmitOneTime(std::function<void(VkCommandBuffer)>&& fn)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_CommandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer cmd;
        vkAllocateCommandBuffers(m_Device, &allocInfo, &cmd);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmd, &beginInfo);

        fn(cmd);

        vkEndCommandBuffer(cmd);

        VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        VkFence fence;
        vkCreateFence(m_Device, &fenceInfo, nullptr, &fence);

        VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;
        vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, fence);
        vkWaitForFences(m_Device, 1, &fence, VK_TRUE, UINT64_MAX);

        vkDestroyFence(m_Device, fence, nullptr);
        vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &cmd);
    }

    void VulkanTexture::SetupForSetData(VkQueue queue, VkCommandPool pool)
    {
		m_GraphicsQueue = queue;
        m_CommandPool = pool;
    }

} // namespace Flux
