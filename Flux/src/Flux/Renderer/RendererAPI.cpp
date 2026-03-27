#include "flpch.h"
#include "RendererAPI.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"


namespace Flux {

    RendererAPI::API RendererAPI::s_API = RendererAPI::API::Vulkan;

    Scope<RendererAPI> RendererAPI::Create()
    {
        switch (s_API)
        {
        case API::None:   return nullptr;
        case API::Vulkan: return CreateScope<VulkanRendererAPI>();
        }

        FL_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

}