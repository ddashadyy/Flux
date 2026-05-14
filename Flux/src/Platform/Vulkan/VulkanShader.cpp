#include "flpch.h"
#include "VulkanShader.h"
#include "VulkanCommon.h"

namespace Flux {

	VulkanShader::VulkanShader(VkDevice device, ShaderStage stage, const std::vector<uint32_t>& spirv, const std::string& entryPoint)
		: m_Device(device), m_Stage(stage), m_EntryPoint(entryPoint)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = spirv.size() * sizeof(uint32_t);
		createInfo.pCode = spirv.data();

		vkCreateShaderModule(m_Device, &createInfo, nullptr, &m_Module);

		FL_CORE_INFO("Created Vulkan shader module, stage: {0}", ShaderStageToString(stage));
	}

	VulkanShader::~VulkanShader()
	{
		vkDestroyShaderModule(m_Device, m_Module , nullptr);
	}

}