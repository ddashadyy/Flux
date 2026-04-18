#include "flpch.h"
#include "VulkanDescriptorSet.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanCommon.h"


namespace Flux {

	static VkDescriptorType GetDescriptorType(DescriptorType type)
	{
		switch (type)
		{
		case DescriptorType::UniformBuffer:        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case DescriptorType::StorageBuffer:        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case DescriptorType::CombinedImageSampler: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		case DescriptorType::StorageImage:         return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		}

		FL_CORE_ASSERT(false, "Unknown Descriptor Type!");
		return VK_DESCRIPTOR_TYPE_MAX_ENUM;
	}

	VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VkDevice device, const DescriptorSetLayoutDesc& desc)
		: m_Device(device), m_Desc(desc)
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings{};

		for (auto& binding : m_Desc.Bindings)
		{
			VkDescriptorSetLayoutBinding b{};
			b.binding = binding.Binding;
			b.descriptorType = GetDescriptorType(binding.Type);
			b.descriptorCount = binding.Count;
			b.stageFlags = GetShaderStageFlags(binding.Stage);
			b.pImmutableSamplers = nullptr;

			bindings.emplace_back(b);
		}

		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		createInfo.pBindings = bindings.data();

		FL_CORE_ASSERT(vkCreateDescriptorSetLayout(m_Device, &createInfo, nullptr, &m_Layout) == VK_SUCCESS,
			"Failed to create Descriptor Set Layout");

		FL_CORE_INFO("Created Vulkan DescriptorSetLayout");
	}

	VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(m_Device, m_Layout, nullptr);
	}

	VulkanDescriptorSet::VulkanDescriptorSet(VkDevice device, VkDescriptorPool pool, VkDescriptorSetLayout layout)
		: m_Device(device), m_Pool(pool)
	{
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = pool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &layout;

		FL_CORE_ASSERT(vkAllocateDescriptorSets(m_Device, &allocInfo, &m_DescriptorSet) == VK_SUCCESS,
			"Failed to allocate Descriptor Set");

		FL_CORE_INFO("Created Vulkan DescriptorSet");
	}

	VulkanDescriptorSet::~VulkanDescriptorSet()
	{
		vkFreeDescriptorSets(m_Device, m_Pool, 1, &m_DescriptorSet);
	}

	void VulkanDescriptorSet::BindBuffer(uint32_t binding, const RHIBuffer* buffer)
	{
		const auto* vulkanBuffer = static_cast<const VulkanBuffer*>(buffer);

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = vulkanBuffer->GetHandle<VkBuffer>();
		bufferInfo.offset = 0;
		bufferInfo.range = vulkanBuffer->GetSize();

		m_BufferInfos.emplace_back(bufferInfo);

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = m_DescriptorSet;
		write.dstBinding = binding;
		write.dstArrayElement = 0;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write.pBufferInfo = &m_BufferInfos.back();

		m_PendingWrites.emplace_back(write);
	}

	void VulkanDescriptorSet::BindTexture(uint32_t binding, const RHITexture* texture)
	{
		const auto* vulkanTexture = static_cast<const VulkanTexture*>(texture);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = vulkanTexture->GetImageView();
		imageInfo.sampler = vulkanTexture->GetSampler();

		m_ImageInfos.emplace_back(imageInfo);

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = m_DescriptorSet;
		write.dstBinding = binding;
		write.dstArrayElement = 0;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.pImageInfo = &m_ImageInfos.back();

		m_PendingWrites.emplace_back(write);
	}

	void VulkanDescriptorSet::Update()
	{
		std::vector<VkDescriptorBufferInfo> bufferInfosCopy = std::move(m_BufferInfos);
		std::vector<VkDescriptorImageInfo> imageInfosCopy = std::move(m_ImageInfos);

		size_t bufIdx = 0, imgIdx = 0;
		for (auto& write : m_PendingWrites)
		{
			if (write.pBufferInfo) write.pBufferInfo = &bufferInfosCopy[bufIdx++];
			if (write.pImageInfo) write.pImageInfo = &imageInfosCopy[imgIdx++];
		}

		vkUpdateDescriptorSets(m_Device,
			static_cast<uint32_t>(m_PendingWrites.size()),
			m_PendingWrites.data(),
			0, nullptr
		);

		m_PendingWrites.clear();
	}

}