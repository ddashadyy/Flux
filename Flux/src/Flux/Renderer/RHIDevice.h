#pragma once


#include "Flux/Core.h"

#include "RHIBuffer.h"
#include "RHITexture.h"
#include "RHIPipeline.h"
#include "RHIRenderPass.h"
#include "RHIShader.h"
#include "RHIFence.h"
#include "RHISemaphore.h"
#include "RHICommandList.h"
#include "RHISwapchain.h"
#include "RHIDescriptorSet.h"


namespace Flux {

    class RHIDevice 
    {
    public:
        virtual ~RHIDevice() = default;

        virtual Scope<RHIBuffer>              CreateBuffer(const BufferSpec& spec) = 0;
        virtual Scope<RHITexture>             CreateTexture(const TextureSpec& spec) = 0;
        virtual Scope<RHIPipeline>            CreatePipeline(const PipelineDesc& desc) = 0;
        virtual Scope<RHIRenderPass>          CreateRenderPass(const RenderPassDesc& desc) = 0;
        virtual Scope<RHIShader>              CreateShader(ShaderStage stage, const std::vector<uint32_t>& spirv) = 0;
        virtual Scope<RHIFence>               CreateFence(bool signaled = false) = 0;
        virtual Scope<RHISemaphore>           CreateSemaphore() = 0;
        virtual Scope<RHIDescriptorSetLayout> CreateDescriptorSetLayout(const DescriptorSetLayoutDesc& desc) = 0;
        virtual Scope<RHIDescriptorSet>       CreateDescriptorSet(const RHIDescriptorSetLayout* layout) = 0;

        virtual RHICommandList* GetCommandList(uint32_t index = 0) = 0;
        virtual RHISwapchain* GetSwapchain() = 0;
    };

}