#include "flpch.h"
#include "UniformBuffer.h"
#include "RendererAPI.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"

namespace Flux {

	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::Vulkan: return Flux::CreateRef<VulkanUniformBuffer>(size);
		case RendererAPI::API::None:   FL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		}

		FL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
		return nullptr;
	}

}