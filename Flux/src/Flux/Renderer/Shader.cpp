#include "flpch.h"
#include "Shader.h"
#include "RendererAPI.h"
#include "Platform/Vulkan/VulkanShader.h"

namespace Flux {

	Ref<Shader> Shader::Create(const std::string& filePath)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::Vulkan: 
			return Flux::CreateRef<VulkanShader>(filePath);
				
		case RendererAPI::API::None:
			FL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;	
		}
	}
}