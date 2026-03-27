#include "flpch.h"
#include "UniformBuffer.h"
#include "RendererAPI.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"

namespace Flux {

	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None:   return nullptr;
		case RendererAPI::API::Vulkan: return Flux::CreateRef<VulkanUniformBuffer>(size);
		}

		FL_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}