#pragma once


#include "Flux/Renderer/RHICommon.h"

#include <vulkan/vulkan.h>

namespace Flux {
	VkShaderStageFlags    GetShaderStageFlags(ShaderStage stage);
	VkSampleCountFlagBits GetSampleCount(SampleCount samples);
	VkFormat              GetVkFormat(Format format);
}