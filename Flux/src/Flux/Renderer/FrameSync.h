#pragma once

#include "RHIFence.h"
#include "RHISemaphore.h"
#include "RHIDevice.h"

#include <vector>

namespace Flux {

    class FrameSync
    {
    public:
        FrameSync(RHIDevice& device, uint32_t frameCount);

        RHIFence& GetFrameFence(uint32_t frame);
        RHISemaphore& GetImageAvailable(uint32_t frame);
        RHISemaphore& GetRenderFinished(uint32_t frame);

        uint32_t GetFrameCount() const { return m_FrameCount; }

    private:
        std::vector<Scope<RHIFence>>     m_FrameFences;
        std::vector<Scope<RHISemaphore>> m_ImageAvailable;
        std::vector<Scope<RHISemaphore>> m_RenderFinished;

        uint32_t m_FrameCount = 0;
    };

}