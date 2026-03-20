#pragma once


#include "Flux/Renderer/Pipeline.h"

#include <vulkan/vulkan.h>

namespace Flux {

	class VulkanPipeline final : public Pipeline
	{
	public: 
        VulkanPipeline(const Ref<Shader>& shader);
        ~VulkanPipeline();

        FL_NON_COPYABLE(VulkanPipeline);

        void Bind() const override;

        inline VkPipeline GetPipeline() const { return m_Pipeline; }

    private:
        void CreatePipelineLayout();
        void CreatePipeline(const Ref<Shader>& shader);

    private:
        VkPipeline       m_Pipeline       = VK_NULL_HANDLE;
        VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
	};
}