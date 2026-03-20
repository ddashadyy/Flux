#include "flpch.h"
#include "Pipeline.h"
#include "RendererAPI.h"
#include "Platform/Vulkan/VulkanPipeline.h"

namespace Flux {

	Ref<Pipeline> Pipeline::Create(const Ref<Shader>& shader)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::Vulkan:
			return Flux::CreateRef<VulkanPipeline>(shader);
		
		case RendererAPI::API::None:
			FL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		}
	}
}