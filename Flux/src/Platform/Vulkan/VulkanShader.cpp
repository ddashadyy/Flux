#include "flpch.h"
#include "VulkanShader.h"

#include "Platform/Vulkan/VulkanContext.h"

#include "Flux/Utils/PlatformUtils.h"

namespace Flux {

	VulkanShader::VulkanShader(const std::string& filePath)
	{
		auto vertexShaderCode = Utils::ReadFile(filePath + ".vert.spv");
		auto fragmentShaderCode = Utils::ReadFile(filePath + ".frag.spv");

		m_VertModule = CreateShaderModule(vertexShaderCode);
		m_FragModule = CreateShaderModule(fragmentShaderCode);

		FL_CORE_INFO("Vulkan Shader Modules created");

	}

	VulkanShader::~VulkanShader()
	{
		VkDevice device = VulkanContext::Get().GetDevice();

		vkDestroyShaderModule(device, m_FragModule, nullptr);
		vkDestroyShaderModule(device, m_VertModule, nullptr);
	
	}

	VkShaderModule VulkanShader::CreateShaderModule(const std::vector<char>& code)
	{
		VkDevice device = VulkanContext::Get().GetDevice();

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.pNext = VK_NULL_HANDLE;
		createInfo.flags = 0;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		FL_CORE_ASSERT(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) == VK_SUCCESS, "Failed to create Shader Module");

		return shaderModule;
	}

}