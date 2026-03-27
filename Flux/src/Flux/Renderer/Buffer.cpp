#include "flpch.h"
#include "Buffer.h"
#include "RendererAPI.h"
#include "Platform/Vulkan/VulkanBuffer.h"

namespace Flux {

	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None:   return nullptr;
		case RendererAPI::API::Vulkan: return Flux::CreateScope<VulkanVertexBuffer>(size);
		}

		FL_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None:   return nullptr;
		case RendererAPI::API::Vulkan: return Flux::CreateScope<VulkanVertexBuffer>(vertices, size);
		}

		FL_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None:   return nullptr;
		case RendererAPI::API::Vulkan: return Flux::CreateScope<VulkanIndexBuffer>(indices, count);
		}

		FL_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}