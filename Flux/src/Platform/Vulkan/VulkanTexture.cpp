#include "flpch.h"
#include "VulkanTexture.h"
#include "VulkanBuffer.h"


namespace Flux {

    static VkFormat GetImageFormat(Format format)
    {
        switch (format)
        {
        case Format::R8G8B8A8_UNORM:      return VK_FORMAT_R8G8B8A8_UNORM;
        case Format::B8G8R8A8_UNORM:      return VK_FORMAT_B8G8R8A8_UNORM;
        case Format::D32_SFLOAT:          return VK_FORMAT_D32_SFLOAT;
        case Format::R32G32_SFLOAT:       return VK_FORMAT_R32G32_SFLOAT;
        case Format::R32G32B32_SFLOAT:    return VK_FORMAT_R32G32B32_SFLOAT;
        case Format::R32G32B32A32_SFLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
        }

        FL_CORE_ASSERT(false, "Unknown Format!");
        return VK_FORMAT_UNDEFINED;
    }

    static VkImageUsageFlags GetImageUsageFlags(TextureUsage usage)
    {
        switch (usage)
        {
        case TextureUsage::Sampled:      return VK_IMAGE_USAGE_SAMPLED_BIT |
                                                VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        case TextureUsage::RenderTarget: return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                                VK_IMAGE_USAGE_SAMPLED_BIT;
        case TextureUsage::DepthStencil: return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        case TextureUsage::Storage:      return VK_IMAGE_USAGE_STORAGE_BIT |
                                                VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }

        FL_CORE_ASSERT(false, "Unknown Texture Usage!");
        return VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
    }

    static VkImageAspectFlags GetImageAspect(TextureUsage usage)
    {
        switch (usage)
        {
        case TextureUsage::DepthStencil: return VK_IMAGE_ASPECT_DEPTH_BIT;
        default:                         return VK_IMAGE_ASPECT_COLOR_BIT;
        }
    }

	VulkanTexture::VulkanTexture(VkDevice device, VkQueue graphicsQueue,
        VkCommandPool commandPool, VmaAllocator allocator,
        const TextureSpec& spec
    )
		: m_Device(device), m_GraphicsQueue(graphicsQueue), m_CommandPool(commandPool), m_Allocator(allocator), m_Spec(spec)
	{
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = GetImageFormat(m_Spec.ImageFormat);
        imageInfo.extent.width = m_Spec.Width;
        imageInfo.extent.height = m_Spec.Height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = m_Spec.MipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = GetImageUsageFlags(m_Spec.Usage);
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        FL_CORE_ASSERT(vmaCreateImage(m_Allocator, &imageInfo, &allocInfo,
            &m_Image, &m_Allocation, nullptr) == VK_SUCCESS, "Failed to create Vulkan image!");


        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_Image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = GetImageFormat(m_Spec.ImageFormat);

        viewInfo.subresourceRange.aspectMask = GetImageAspect(m_Spec.Usage);
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = m_Spec.MipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        FL_CORE_ASSERT(vkCreateImageView(m_Device, &viewInfo, nullptr, &m_ImageView) == VK_SUCCESS,
            "Failed to create Image View");

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = 16.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(m_Spec.MipLevels);

        FL_CORE_ASSERT(vkCreateSampler(m_Device, &samplerInfo, nullptr, &m_Sampler) == VK_SUCCESS,
            "Failed to create Sampler");
	}

	VulkanTexture::~VulkanTexture()
	{
        vkDestroySampler(m_Device, m_Sampler, nullptr);
        vkDestroyImageView(m_Device, m_ImageView, nullptr);
        vmaDestroyImage(m_Allocator, m_Image, m_Allocation);
	}

	void VulkanTexture::SetData(const void* data, uint32_t size)
	{

		BufferSpec stagingSpec{};
		stagingSpec.Size = size;
		stagingSpec.Usage = BufferUsage::Staging;
		stagingSpec.CpuVisible = true;

		VulkanBuffer stagingBuffer(m_Allocator, stagingSpec);

		void* ptr = stagingBuffer.Map();
        memcpy(ptr, data, size);
		stagingBuffer.Unmap();

        TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        CopyBufferToImage(stagingBuffer.GetHandle());

        TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	}

    void VulkanTexture::TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkCommandBuffer cmd = BeginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_Image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = m_Spec.MipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags srcStage, dstStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
            newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
            newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else
        {
            FL_CORE_ASSERT(false, "Unsupported layout transition!");
            return;
        }

        vkCmdPipelineBarrier(cmd,
            srcStage, dstStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        EndSingleTimeCommands(cmd);
    }

    void VulkanTexture::CopyBufferToImage(VkBuffer buffer)
    {
        VkCommandBuffer cmd = BeginSingleTimeCommands();

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

        vkCmdCopyBufferToImage(cmd, buffer, m_Image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &region);

        EndSingleTimeCommands(cmd);
    }

    VkCommandBuffer VulkanTexture::BeginSingleTimeCommands()
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
        return cmd;
    }

    void VulkanTexture::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_GraphicsQueue);

        vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &commandBuffer);
    }

}