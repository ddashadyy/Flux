#include "flpch.h"
#include "VulkanCommon.h"


namespace Flux {

	VkShaderStageFlags GetShaderStageFlags(ShaderStage stage)
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

	VkSampleCountFlagBits GetSampleCount(SampleCount samples)
	{
		switch (samples)
		{
		case SampleCount::x1: return VK_SAMPLE_COUNT_1_BIT;
		case SampleCount::x2: return VK_SAMPLE_COUNT_2_BIT;
		case SampleCount::x4: return VK_SAMPLE_COUNT_4_BIT;
		case SampleCount::x8: return VK_SAMPLE_COUNT_8_BIT;
		}
		return VK_SAMPLE_COUNT_1_BIT;
	}

	VkFormat GetVkFormat(Format format)
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
}