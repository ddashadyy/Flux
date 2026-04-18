#include "flpch.h"
#include "FrameSync.h"


namespace Flux {

    FrameSync::FrameSync(RHIDevice& device, uint32_t frameCount)
        : m_FrameCount(frameCount)
    {
        m_FrameFences.resize(frameCount);
        m_ImageAvailable.resize(frameCount);
        m_RenderFinished.resize(frameCount);

        for (uint32_t i = 0; i < frameCount; i++)
        {
            m_FrameFences[i] = device.CreateFence(true);
            m_ImageAvailable[i] = device.CreateSemaphore();
            m_RenderFinished[i] = device.CreateSemaphore();
        }
    }

    RHIFence& FrameSync::GetFrameFence(uint32_t frame)
    {
        FL_CORE_ASSERT(frame < m_FrameCount, "Frame index out of range!");
        return *m_FrameFences[frame];
    }

    RHISemaphore& FrameSync::GetImageAvailable(uint32_t frame)
    {
        FL_CORE_ASSERT(frame < m_FrameCount, "Frame index out of range!");
        return *m_ImageAvailable[frame];
    }

    RHISemaphore& FrameSync::GetRenderFinished(uint32_t frame)
    {
        FL_CORE_ASSERT(frame < m_FrameCount, "Frame index out of range!");
        return *m_RenderFinished[frame];
    }

}