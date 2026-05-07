#pragma once

#include "Flux/Core/Base.h"

#include "RHIBuffer.h"
#include "RHICommandList.h"
#include "RHIDescriptorSet.h"
#include "RHIFence.h"
#include "RHIFramebuffer.h"
#include "RHIPipeline.h"
#include "RHIRenderPass.h"
#include "RHISemaphore.h"
#include "RHIShader.h"
#include "RHISwapchain.h"
#include "RHITexture.h"

#include <functional>

namespace Flux {

    // -------------------------------------------------------------------------
    // Submit descriptor — собрана вся синхронизация в одном месте
    // -------------------------------------------------------------------------

    struct SubmitDesc
    {
        RHICommandList*  CommandList     = nullptr;
        RHIFence*        SignalFence     = nullptr; // сигналить по завершению (CPU-GPU sync)
        RHISemaphore*    WaitSemaphore   = nullptr; // ждать перед выполнением
        RHISemaphore*    SignalSemaphore = nullptr; // сигналить после (GPU-GPU sync)
    };

    // -------------------------------------------------------------------------
    // Memory stats
    // -------------------------------------------------------------------------

    struct DeviceMemoryStats
    {
        uint32_t AllocationCount = 0;
        uint64_t AllocationBytes = 0;

        uint32_t BlockCount      = 0;
        uint64_t BlockBytes      = 0;

        uint64_t Usage           = 0;
        uint64_t Budget          = 0;
    };

    // -------------------------------------------------------------------------
    // Device
    // -------------------------------------------------------------------------

    class RHIDevice
    {
    public:
        virtual ~RHIDevice() = default;

        virtual void* GetHandle() const = 0;

        // -----------------------------------------------------------------
        // Resource creation
        // -----------------------------------------------------------------

        virtual Scope<RHIBuffer>              CreateBuffer(const BufferSpec& spec)                         = 0;
        virtual Scope<RHITexture>             CreateTexture(const TextureSpec& spec)                       = 0;
        virtual Scope<RHISampler>             CreateSampler(const SamplerSpec& spec)                       = 0; 
        virtual Scope<RHIFramebuffer>         CreateFramebuffer(const FramebufferSpec& spec)               = 0;
        virtual Scope<RHIPipeline>            CreatePipeline(const PipelineDesc& desc)                     = 0;
        virtual Scope<RHIRenderPass>          CreateRenderPass(const RenderPassDesc& desc)                 = 0;
        virtual Scope<RHIShader>              CreateShader(ShaderStage stage,
                                                           const std::vector<uint32_t>& spirv)             = 0;
        virtual Scope<RHIFence>               CreateFence(bool signaled = false)                           = 0;
        virtual Scope<RHISemaphore>           CreateSemaphore()                                            = 0;
        virtual Scope<RHIDescriptorSetLayout> CreateDescriptorSetLayout(const DescriptorSetLayoutDesc& desc) = 0;
        virtual Scope<RHIDescriptorSet>       CreateDescriptorSet(const RHIDescriptorSetLayout* layout)   = 0;

        // -----------------------------------------------------------------
        // Command lists — девайс владеет пулом
        // -----------------------------------------------------------------

        virtual RHICommandList* GetCommandList(uint32_t index = 0) = 0;

        // -----------------------------------------------------------------
        // Submit — перенесён из RHICommandList (правильное место)
        // -----------------------------------------------------------------

        virtual void Submit(const SubmitDesc& desc) = 0;

        // Одноразовый синхронный submit для upload операций (staging copy и т.п.)
        // Лямбда получает command list, девайс сам Begin/End/Submit/Wait
        virtual void ImmediateSubmit(std::function<void(RHICommandList*)>&& fn) = 0;

        // -----------------------------------------------------------------
        // Swapchain
        // -----------------------------------------------------------------

        virtual RHISwapchain* GetSwapchain() = 0;

        // -----------------------------------------------------------------
        // Utils
        // -----------------------------------------------------------------

        // CopyBuffer через ImmediateSubmit — можно оставить как convenience метод
        virtual void CopyBuffer(RHIBuffer* src, RHIBuffer* dst,
                                uint64_t size      = 0,
                                uint64_t srcOffset = 0,
                                uint64_t dstOffset = 0) const = 0;

        virtual DeviceMemoryStats GetMemoryStatistics() const = 0;

        virtual void WaitIdle() const = 0;
    };

} // namespace Flux
