#include "flpch.h"
#include "VulkanFramebuffer.h"
#include "VulkanRenderPass.h"
#include "VulkanSwapchainTexture.h"

namespace Flux {

    VulkanFramebuffer::VulkanFramebuffer(VkDevice device, const FramebufferSpec& spec)
        : m_Device(device), m_Spec(spec)
    {
        std::vector<VkImageView> attachments;

        for (auto* colorTarget : m_Spec.ColorTargets)
            attachments.emplace_back(static_cast<VkImageView>(colorTarget->GetNativeImageView()));

        if (m_Spec.DepthTarget)
            attachments.emplace_back(static_cast<VkImageView>(m_Spec.DepthTarget->GetNativeImageView()));

        if (m_Spec.ResolveTarget)  
            attachments.emplace_back(static_cast<VkImageView>(m_Spec.ResolveTarget->GetNativeImageView()));

        auto* vkPass = static_cast<VulkanRenderPass*>(m_Spec.RenderPass);

        VkFramebufferCreateInfo fbInfo{};
        fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbInfo.renderPass = vkPass->GetHandle<VkRenderPass>();
        fbInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        fbInfo.pAttachments = attachments.data();
        fbInfo.width = m_Spec.Width;
        fbInfo.height = m_Spec.Height;
        fbInfo.layers = 1;

        FL_CORE_ASSERT(vkCreateFramebuffer(m_Device, &fbInfo, nullptr, &m_Framebuffer) == VK_SUCCESS,
            "Failed to create VulkanFramebuffer");

        FL_CORE_INFO("Created VulkanFramebuffer {}x{}", m_Spec.Width, m_Spec.Height);
    }

    VulkanFramebuffer::~VulkanFramebuffer()
    {
        vkDestroyFramebuffer(m_Device, m_Framebuffer, nullptr);
    }
}