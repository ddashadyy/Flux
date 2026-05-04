#pragma once

#include "RHIRenderPass.h"
#include "RHITexture.h"

#include <vector>

namespace Flux {

    struct FramebufferSpec
    {
        RHIRenderPass* RenderPass = nullptr;

        // Один color target на фреймбуфер.
        // Если рендерить в swapchain — создавать по одному фреймбуферу на каждый
        std::vector<RHITexture*> ColorTargets  = {}; // один слот на каждый color attachment renderpass
        RHITexture*              DepthTarget   = nullptr;
        RHITexture*              ResolveTarget = nullptr; // MSAA resolve

        uint32_t Width  = 0;
        uint32_t Height = 0;

        const char* DebugName = nullptr;
    };

    class RHIFramebuffer
    {
    public:
        virtual ~RHIFramebuffer() = default;

        template<typename T>
        T GetHandle() const { return reinterpret_cast<T>(GetHandleImpl()); }

        virtual uint32_t GetWidth()  const = 0;
        virtual uint32_t GetHeight() const = 0;

        virtual const FramebufferSpec& GetSpec() const = 0;

    protected:
        virtual void* GetHandleImpl() const = 0;
    };

} // namespace Flux
