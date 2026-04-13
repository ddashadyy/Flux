#pragma once


#include "Flux/Renderer/RHITexture.h"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace Flux {


	class VulkanTexture final : public RHITexture
	{
	public:
		VulkanTexture(VkDevice device, VkQueue graphicsQueue,
			VkCommandPool commandPool, VmaAllocator allocator,
			const TextureSpec& spec);
		~VulkanTexture();

		uint32_t GetWidth()     const override { return m_Spec.Width; }
		uint32_t GetHeight()    const override { return m_Spec.Height; }
		Format   GetFormat()    const override { return m_Spec.ImageFormat; }
		uint32_t GetMipLevels() const override { return m_Spec.MipLevels; }

		void SetData(const void* data, uint32_t size) override;

		VkImage     GetImage()     const { return m_Image; }
		VkImageView GetImageView() const { return m_ImageView; }
		VkSampler   GetSampler()   const { return m_Sampler; }

	private:
		void TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBufferToImage(VkBuffer buffer);
		
		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands(VkCommandBuffer commandBuffer);


	private:
		VkImage        m_Image      = VK_NULL_HANDLE;
		VkImageView    m_ImageView  = VK_NULL_HANDLE;
		VkSampler      m_Sampler    = VK_NULL_HANDLE;

		VmaAllocator   m_Allocator  = VK_NULL_HANDLE;
		VmaAllocation  m_Allocation = VK_NULL_HANDLE;

		VkDevice       m_Device        = VK_NULL_HANDLE;
		VkQueue		   m_GraphicsQueue = VK_NULL_HANDLE;
		VkCommandPool  m_CommandPool   = VK_NULL_HANDLE;

		TextureSpec    m_Spec{};
	};
}
