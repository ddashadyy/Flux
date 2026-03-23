#pragma once


#include "Flux/Renderer/Shader.h"

#include <vulkan/vulkan.h>

namespace Flux {

	class VulkanShader final : public Shader 
	{
	public:
		VulkanShader(const std::string& filePath);
		~VulkanShader();

		inline const std::string& GetName() const override { return m_Name; }

		inline VkShaderModule GetVertexModule() const { return m_VertModule; }
		inline VkShaderModule GetFragmentModule() const { return m_FragModule; }

	private: 
		VkShaderModule CreateShaderModule(const std::vector<char>& code);

	private:
		std::string    m_Name;

		VkShaderModule m_VertModule = VK_NULL_HANDLE;
		VkShaderModule m_FragModule = VK_NULL_HANDLE;
	};
}