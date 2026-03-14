#include "flpch.h"
#include "GraphicsContext.h"

#include "RendererAPI.h"
#include "Flux/Core.h"
#include "Platform/Vulkan/VulkanContext.h"

namespace Flux {

	Scope<GraphicsContext> GraphicsContext::Create(void* window)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::Vulkan:
			return CreateScope<VulkanContext>(static_cast<GLFWwindow*>(window));
		case RendererAPI::API::None:
			FL_CORE_ASSERT(false, "RendererAPI::None is not supported!");
			return nullptr;
		}

		FL_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}