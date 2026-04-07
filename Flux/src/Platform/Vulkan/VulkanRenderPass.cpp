#include "flpch.h"
#include "VulkanRenderPass.h"


namespace Flux {

    static VkFormat GetFormat(Format format)
    {
        switch (format)
        {
        case Format::R8G8B8A8_UNORM:      return VK_FORMAT_R8G8B8A8_UNORM;
        case Format::B8G8R8A8_UNORM:      return VK_FORMAT_B8G8R8A8_UNORM;
        case Format::D32_SFLOAT:          return VK_FORMAT_D32_SFLOAT;
        case Format::R32G32_SFLOAT:       return VK_FORMAT_R32G32_SFLOAT;
        case Format::R32G32B32_SFLOAT:    return VK_FORMAT_R32G32B32_SFLOAT;
        case Format::R32G32B32A32_SFLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
        }

        FL_CORE_ASSERT(false, "Unknown Format!");
        return VK_FORMAT_UNDEFINED;
    }

    static VkAttachmentLoadOp GetLoadOp(AttachmentLoadOp loadOp)
    {
        switch (loadOp)
        {
        case AttachmentLoadOp::Load:     return VK_ATTACHMENT_LOAD_OP_LOAD;
        case AttachmentLoadOp::Clear:    return VK_ATTACHMENT_LOAD_OP_CLEAR;
        case AttachmentLoadOp::DontCare: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        }

        FL_CORE_ASSERT(false, "Unknown Attachment load op!");
        return VK_ATTACHMENT_LOAD_OP_NONE;
    }

    static VkAttachmentStoreOp GetStoreOp(AttachmentStoreOp storeOp)
    {
        switch (storeOp)
        {
        case AttachmentStoreOp::Store:    return VK_ATTACHMENT_STORE_OP_STORE;
        case AttachmentStoreOp::DontCare: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        }

        FL_CORE_ASSERT(false, "Unknown Attachment store op!");
        return VK_ATTACHMENT_STORE_OP_NONE;
    }


    VulkanRenderPass::VulkanRenderPass(VkDevice device, const RenderPassDesc& desc)
        : m_Device(device), m_Desc(desc)
    {
        std::vector<VkAttachmentDescription> attachments{};

        for (auto& format : m_Desc.ColorFormats)
        {
            VkAttachmentDescription color{};
            color.format = GetFormat(format);
            color.samples = VK_SAMPLE_COUNT_1_BIT;
            color.loadOp = GetLoadOp(m_Desc.ColorLoadOp);
            color.storeOp = GetStoreOp(m_Desc.ColorStoreOp);
            color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            attachments.emplace_back(color);
        }

        if (m_Desc.HasDepth)
        {
            VkAttachmentDescription depth{};
            depth.format = GetFormat(m_Desc.DepthFormat);
            depth.samples = VK_SAMPLE_COUNT_1_BIT;
            depth.loadOp = GetLoadOp(m_Desc.DepthLoadOp);
            depth.storeOp = GetStoreOp(m_Desc.DepthStoreOp);
            depth.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depth.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depth.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            depth.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            attachments.emplace_back(depth);
        }

        std::vector<VkAttachmentReference> colorRefs{};
        for (uint32_t i = 0; i < m_Desc.ColorFormats.size(); i++)
        {
            VkAttachmentReference ref{};
            ref.attachment = i;
            ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorRefs.emplace_back(ref);
        }

        VkAttachmentReference depthRef{};
        depthRef.attachment = static_cast<uint32_t>(m_Desc.ColorFormats.size());
        depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = static_cast<uint32_t>(colorRefs.size());
        subpass.pColorAttachments = colorRefs.data();
        subpass.pDepthStencilAttachment = m_Desc.HasDepth ? &depthRef : nullptr;

        std::array<VkSubpassDependency, 2> dependencies{};

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = 0;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = 0;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.pNext = nullptr;
        renderPassInfo.flags = 0;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();

        FL_CORE_ASSERT(vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &m_RenderPass) == VK_SUCCESS,
            "Failed to create Render Pass");

        FL_CORE_INFO("Created Vulkan RenderPass");
    }

    VulkanRenderPass::~VulkanRenderPass()
    {
        vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
    }
}