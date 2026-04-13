#pragma once


#include "Flux/Core.h"

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

namespace Flux {

    struct DeviceMemoryStats
    {
        uint32_t AllocationCount;
        uint64_t AllocationBytes;
        
        uint32_t BlockCount;
        uint64_t BlockBytes;

        uint64_t Usage;
        uint64_t Budget;
    };

    class RHIDevice 
    {
    public:
        virtual ~RHIDevice() = default;

        template<typename T>
        T GetHandle() const { return reinterpret_cast<T>(GetHandleImpl()); }

        virtual Scope<RHIBuffer>              CreateBuffer(const BufferSpec& spec) = 0;
        virtual Scope<RHITexture>             CreateTexture(const TextureSpec& spec) = 0;
        virtual Scope<RHIFramebuffer>         CreateFramebuffer(const FramebufferSpec& spec) = 0;
        virtual Scope<RHIPipeline>            CreatePipeline(const PipelineDesc& desc) = 0;
        virtual Scope<RHIRenderPass>          CreateRenderPass(const RenderPassDesc& desc) = 0;
        virtual Scope<RHIShader>              CreateShader(ShaderStage stage, const std::vector<uint32_t>& spirv) = 0;
        virtual Scope<RHIFence>               CreateFence(bool signaled = false) = 0;
        virtual Scope<RHISemaphore>           CreateSemaphore() = 0;
        virtual Scope<RHIDescriptorSetLayout> CreateDescriptorSetLayout(const DescriptorSetLayoutDesc& desc) = 0;
        virtual Scope<RHIDescriptorSet>       CreateDescriptorSet(const RHIDescriptorSetLayout* layout) = 0;

        virtual RHICommandList* GetCommandList(uint32_t index = 0) = 0;
        virtual RHISwapchain* GetSwapchain() = 0;

        virtual DeviceMemoryStats GetMemoryStatistics() const = 0;

    protected:
        virtual void* GetHandleImpl() const = 0;
    };

}