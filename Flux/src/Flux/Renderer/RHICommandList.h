#pragma once


#include "RHICommon.h"
#include "RHIBuffer.h"
#include "RHIPipeline.h"
#include "RHIFence.h"
#include "RHISemaphore.h"
#include "RHIFramebuffer.h"

#include <glm/glm.hpp>

namespace Flux {


    class RHICommandList 
    {
    public:
        virtual ~RHICommandList() = default;

        virtual void Begin() = 0;
        virtual void End() = 0;

        virtual void Submit(RHIFence* fence,
            RHISemaphore* waitSemaphore = nullptr,
            RHISemaphore* signalSemaphore = nullptr) = 0;

        virtual void BeginRenderPass(RHIRenderPass* renderPass, RHIFramebuffer* framebuffer,
            glm::vec4 clearColor = {0.0f, 0.0f, 0.0f, 1.0f}) = 0;
        virtual void EndRenderPass() = 0;

        virtual void SetViewport(float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f) = 0;
        virtual void SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height) = 0;

        virtual void SetPipeline(RHIPipeline* pipeline)  = 0;
        virtual void PushConstants(RHIPipeline* pipeline, const void* pushConstants) = 0;

        virtual void BindVertexBuffer(RHIBuffer* buffer) = 0;
        virtual void BindIndexBuffer(RHIBuffer* buffer)  = 0;

        virtual void BindDescriptorSet(RHIDescriptorSet* descriptorSet) = 0;

        virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1) = 0;
        virtual void Dispatch(uint32_t gx, uint32_t gy, uint32_t gz) = 0;

        virtual void ResourceBarrier(RHITexture* texture,
            ResourceState oldState,
            ResourceState newState) = 0;
    };

}