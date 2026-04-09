#include "flpch.h"
#include "RHIFactory.h"

#include "Flux/Core.h"
#include "Platform/Vulkan/VulkanDevice.h"

namespace Flux {

    Scope<RHIDevice> RHIFactory::CreateDevice(RendererBackend backend,
        void* windowHandle,
        uint32_t width,
        uint32_t height)
    {
        if (backend == RendererBackend::Auto)
            backend = DetectBestBackend();

        switch (backend)
        {
        case RendererBackend::Vulkan:
            return CreateScope<VulkanDevice>(windowHandle, width, height);

        case RendererBackend::D3D12:
            FL_CORE_ASSERT(false, "D3D12 backend not implemented yet!");
            return nullptr;

        case RendererBackend::Metal:
            FL_CORE_ASSERT(false, "Metal backend not implemented yet!");
            return nullptr;
        }

        FL_CORE_ASSERT(false, "Unknown RendererBackend!");
        return nullptr;
    }

    // пока что нет Direct3d 12 для Windows, поэтому Vulkan
    RendererBackend RHIFactory::DetectBestBackend()
    {
    #if defined(FL_PLATFORM_WINDOWS)
        return RendererBackend::Vulkan;
    #elif defined(FL_PLATFORM_MACOS)
        return RendererBackend::Metal;
    #else
        return RendererBackend::Vulkan;
    #endif
    }

}