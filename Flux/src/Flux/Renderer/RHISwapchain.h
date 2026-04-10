#pragma once


#include "RHICommon.h"
#include "RHISemaphore.h"
#include "RHIRenderPass.h"

#include <cstdint>


namespace Flux {

    class RHISwapchain 
    {
    public:
        virtual ~RHISwapchain() = default;

        virtual uint32_t AcquireNextImage(RHISemaphore* semaphore) = 0;
        virtual void     Present(RHISemaphore* semaphore, uint32_t imageIndex) = 0;
        virtual void     Resize(uint32_t w, uint32_t h) = 0;
        virtual uint32_t GetCurrentImageIndex() const = 0;
        virtual Format   GetFormat() const = 0;
        virtual uint32_t GetImageCount() const = 0;

        virtual RHIRenderPass* GetRenderPass() const = 0;
    };

}