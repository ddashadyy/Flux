#pragma once

#include "Flux/Renderer/RHIPipeline.h"

#include <vulkan/vulkan.h>

namespace Flux {

    class VulkanPipeline final : public RHIPipeline
    {
    public:
        VulkanPipeline(VkDevice device, const PipelineDesc& desc);
        ~VulkanPipeline() override;

		void* GetHandle() const override { return m_Pipeline; }
		void* GetLayout() const override { return m_PipelineLayout; }

        PipelineType        GetType()  const override { return m_Desc.Type; }
        bool                IsValid()  const override { return m_Pipeline != VK_NULL_HANDLE; }
        const PipelineDesc& GetDesc()  const override { return m_Desc; }

    private:
        void CreateGraphicsPipeline();
        void CreateComputePipeline();
        void CreatePipelineLayout();

    private:
        VkDevice         m_Device = VK_NULL_HANDLE;
        VkPipeline       m_Pipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
        PipelineDesc     m_Desc{};
    };

} // namespace Flux
