#include "flpch.h"
#include "Pipeline.h"
#include "RendererAPI.h"
#include "Platform/Vulkan/VulkanPipeline.h"

namespace Flux {

	Ref<Pipeline> Pipeline::Create(const Ref<Shader>& shader, const BufferLayout& layout)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None:   return nullptr;
		case RendererAPI::API::Vulkan: return Flux::CreateRef<VulkanPipeline>(shader, layout);
		}

		FL_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;;
	}
}