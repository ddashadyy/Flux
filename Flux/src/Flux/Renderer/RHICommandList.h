#pragma once

#include "RHICommon.h"
#include "RHIBuffer.h"
#include "RHIPipeline.h"
#include "RHIFramebuffer.h"

#include <glm/glm.hpp>

namespace Flux {

    // Регион текстуры для copy/blit операций
    struct TextureRegion
    {
        uint32_t X      = 0;
        uint32_t Y      = 0;
        uint32_t Z      = 0;
        uint32_t Width  = 0;
        uint32_t Height = 0;
        uint32_t Depth  = 1;
        uint32_t MipLevel    = 0;
        uint32_t ArrayLayer  = 0;
    };


    class RHICommandList
    {
    public:
        virtual ~RHICommandList() = default;

        template<typename T>
        T GetHandle() const { return reinterpret_cast<T>(GetHandleImpl()); }

        // -----------------------------------------------------------------
        // Recording
        // -----------------------------------------------------------------

        virtual void Begin() = 0;
        virtual void End()   = 0;

        // Submit убран отсюда — это ответственность RHIDevice::Submit()

        // -----------------------------------------------------------------
        // Render pass
        // -----------------------------------------------------------------

        virtual void BeginRenderPass(RHIRenderPass*  renderPass,
                                     RHIFramebuffer* framebuffer,
                                     glm::vec4       clearColor = { 0.0f, 0.0f, 0.0f, 1.0f },
                                     float           clearDepth = 1.0f,
                                     uint8_t         clearStencil = 0) = 0;
        virtual void EndRenderPass() = 0;

        // -----------------------------------------------------------------
        // Dynamic state
        // -----------------------------------------------------------------

        virtual void SetViewport(float x, float y, float width, float height,
                                 float minDepth = 0.0f, float maxDepth = 1.0f) = 0;
        virtual void SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height) = 0;

        // -----------------------------------------------------------------
        // Pipeline & resources
        // -----------------------------------------------------------------

        virtual void SetPipeline(RHIPipeline* pipeline) = 0;

        virtual void PushConstants(RHIPipeline* pipeline,
                                   ShaderStage  stage,
                                   uint32_t     offset,
                                   uint32_t     size,
                                   const void*  data) = 0;  

        virtual void BindVertexBuffer(RHIBuffer* buffer, uint64_t offset = 0) = 0;
        virtual void BindIndexBuffer(RHIBuffer*  buffer,
                                     IndexType   indexType = IndexType::Uint32,
                                     uint64_t    offset    = 0) = 0;

        virtual void BindDescriptorSet(uint32_t          setIndex,
                                       RHIDescriptorSet* descriptorSet,
                                       RHIPipeline*      pipeline) = 0; // pipeline нужен для layout

        // -----------------------------------------------------------------
        // Draw calls
        // -----------------------------------------------------------------

        virtual void Draw(uint32_t vertexCount,
                          uint32_t instanceCount = 1,
                          uint32_t firstVertex   = 0,
                          uint32_t firstInstance = 0) = 0;

        virtual void DrawIndexed(uint32_t indexCount,
                                 uint32_t instanceCount = 1,
                                 uint32_t firstIndex    = 0,
                                 int32_t  vertexOffset  = 0,
                                 uint32_t firstInstance = 0) = 0;

        virtual void DrawIndirect(RHIBuffer* argsBuffer,
                                  uint64_t   offset,
                                  uint32_t   drawCount) = 0;

        virtual void DrawIndexedIndirect(RHIBuffer* argsBuffer,
                                         uint64_t   offset,
                                         uint32_t   drawCount) = 0;

        // -----------------------------------------------------------------
        // Compute
        // -----------------------------------------------------------------

        virtual void Dispatch(uint32_t gx, uint32_t gy, uint32_t gz) = 0;

        virtual void DispatchIndirect(RHIBuffer* argsBuffer, uint64_t offset) = 0;

        // -----------------------------------------------------------------
        // Transfers
        // -----------------------------------------------------------------

        virtual void CopyBuffer(RHIBuffer* src, RHIBuffer* dst,
                                 uint64_t size      = 0,
                                 uint64_t srcOffset = 0,
                                 uint64_t dstOffset = 0) = 0;

        // Загрузить данные из staging buffer в текстуру (mip/layer)
        virtual void CopyBufferToTexture(RHIBuffer*          src,
                                         RHITexture*         dst,
                                         const TextureRegion& region) = 0;

        // Blit (с масштабированием / фильтрацией) — нужен для генерации mip-ов
        virtual void BlitTexture(RHITexture*   src,
                                 RHITexture*   dst,
                                 TextureRegion srcRegion,
                                 TextureRegion dstRegion,
                                 FilterMode    filter = FilterMode::Linear) = 0;

        // -----------------------------------------------------------------
        // Barriers
        // -----------------------------------------------------------------

        virtual void ResourceBarrier(RHITexture*   texture,
                                     ResourceState oldState,
                                     ResourceState newState) = 0;

        virtual void BufferBarrier(RHIBuffer*    buffer,
                                   ResourceState oldState,
                                   ResourceState newState) = 0; // нужен для storage buffer в compute

    protected:
        virtual void* GetHandleImpl() const = 0;
    };

} // namespace Flux
