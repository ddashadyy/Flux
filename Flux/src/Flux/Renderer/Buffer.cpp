#include "flpch.h"
#include "Buffer.h"
#include "RendererAPI.h"
#include "Platform/Vulkan/VulkanBuffer.h"

namespace Flux {

	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::Vulkan: return Flux::CreateScope<VulkanVertexBuffer>(size);
		case RendererAPI::API::None:   FL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		}
	}

	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::Vulkan: return Flux::CreateScope<VulkanVertexBuffer>(vertices, size);
		case RendererAPI::API::None:   FL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		}
	}
	
	Ref<IndexBuffer> IndexBuffer::Create(uint32_t size)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::Vulkan: return Flux::CreateScope<VulkanIndexBuffer>(size);
		case RendererAPI::API::None:   FL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		}
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t size)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::Vulkan: return Flux::CreateScope<VulkanIndexBuffer>(indices, size);
		case RendererAPI::API::None:   FL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		}
	}
}