#pragma once


#include "Flux/Renderer/RHIContext.h"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>


namespace Flux {


	// Контекст вулкана

	class VulkanRHI final : public RHIContext
	{
	public:
		VulkanRHI()  = default;
		~VulkanRHI() = default;

		void Init();
		void Shutdown();

		Scope<Buffer>         CreateBuffer(const BufferSpec& spec)               override;
		Scope<Texture>        CreateTexture(const TextureSpec& spec)             override;
		Scope<Shader>         CreateShader(const ShaderSpec& spec)               override;
		Scope<RenderPass>     CreateRenderPass(const RenderPassSpec& spec)       override;
		Scope<Pipeline>       CreatePipeline(const PipelineSpec& spec)           override;
		Scope<Sampler>        CreateSampler(const SamplerSpec& spec)             override;
		Scope<Texture>        CreateTexture(const TextureSpec& spec)             override;
		Scope<DescriptorSet>  CreateDescriptorSet(const DescriptorSetSpec& spec) override;

		void Submit(const CommandList& cmds) override;
		void Present() override;


	private:

	};
}



