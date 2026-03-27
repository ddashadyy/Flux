#include "flpch.h"
#include "GraphicsContext.h"
#include "RendererAPI.h"
#include "Platform/Vulkan/VulkanContext.h"

namespace Flux {

	Scope<GraphicsContext> GraphicsContext::Create(void* window)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None:   return nullptr;
		case RendererAPI::API::Vulkan: return CreateScope<VulkanContext>(static_cast<GLFWwindow*>(window));
		}

		FL_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}