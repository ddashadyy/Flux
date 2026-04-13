#pragma once


#include "Flux/Renderer/RHIShader.h"

#include <vulkan/vulkan.h>

namespace Flux {

	class VulkanShader final : public RHIShader 
	{
	public:
		VulkanShader(VkDevice device, ShaderStage stage,
			const std::vector<uint32_t>& spirv,
			const std::string& entryPoint = "main"
		);
		~VulkanShader();

		ShaderStage        GetStage()      const override { return m_Stage; }
		const std::string& GetEntryPoint() const override { return m_EntryPoint; }

	private:
		VkDevice       m_Device     = VK_NULL_HANDLE;
		VkShaderModule m_Module     = VK_NULL_HANDLE;

		ShaderStage    m_Stage      = ShaderStage::Vertex;
		std::string    m_EntryPoint = "main";

	protected:
		void* GetHandleImpl() const override { return m_Module; }
	};
}