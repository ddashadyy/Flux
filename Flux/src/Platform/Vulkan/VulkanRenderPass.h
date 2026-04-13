#pragma once


#include "Flux/Renderer/RHIRenderPass.h"

#include <vulkan/vulkan.h>

namespace Flux {

    class VulkanRenderPass final : public RHIRenderPass
    {
    public:
        VulkanRenderPass(VkDevice device, const RenderPassDesc& desc);
        ~VulkanRenderPass();

        uint32_t GetColorAttachmentCount() const override { return static_cast<uint32_t>(m_Desc.ColorFormats.size()); }
        bool     HasDepthAttachment()      const override { return m_Desc.HasDepth; }

        inline const RenderPassDesc& GetDesc() const override { return m_Desc; }

        inline VkRenderPass GetHandle() const { return m_RenderPass; }

    private:
        VkDevice       m_Device     = VK_NULL_HANDLE;
        RenderPassDesc m_Desc{};

        VkRenderPass   m_RenderPass = VK_NULL_HANDLE;
    };
}