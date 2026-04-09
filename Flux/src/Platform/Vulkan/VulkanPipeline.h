#pragma once


#include "Flux/Renderer/RHIPipeline.h"
#include "VulkanShader.h"

#include <vulkan/vulkan.h>

namespace Flux {

    class VulkanPipeline : public RHIPipeline 
    {
    public:
        VulkanPipeline(VkDevice device, const PipelineDesc& desc);
        ~VulkanPipeline() override;

        inline PipelineType GetType()  const override { return m_Desc.Type; }
		inline bool         IsValid()  const override { return m_Pipeline != VK_NULL_HANDLE; }

        VkPipeline            GetHandle()       const { return m_Pipeline; }
        VkPipelineLayout      GetLayout()       const { return m_PipelineLayout; }

    private:
        VkDevice         m_Device = VK_NULL_HANDLE;
        VkPipeline       m_Pipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
        PipelineDesc     m_Desc;
    };
}