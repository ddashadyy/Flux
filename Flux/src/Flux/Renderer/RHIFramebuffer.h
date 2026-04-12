#pragma once
#include "RHIRenderPass.h"
#include "RHITexture.h"
#include <vector>

namespace Flux {

    struct FramebufferSpec
    {
        RHIRenderPass* RenderPass = nullptr;
        std::vector<RHITexture*> ColorTargets = {}; // по одному на каждый swapchain image
        RHITexture* DepthTarget = nullptr;
        uint32_t                 Width = 0;
        uint32_t                 Height = 0;
    };

    class RHIFramebuffer
    {
    public:
        virtual ~RHIFramebuffer() = default;

        virtual uint32_t GetWidth()  const = 0;
        virtual uint32_t GetHeight() const = 0;

        virtual const FramebufferSpec& GetSpec() const = 0;
    };
}