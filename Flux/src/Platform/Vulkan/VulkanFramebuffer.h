#pragma once


#include "Flux/Renderer/RHIFramebuffer.h"
#include <vulkan/vulkan.h>

namespace Flux {

    class VulkanFramebuffer final : public RHIFramebuffer
    {
    public:
        VulkanFramebuffer(VkDevice device, const FramebufferSpec& spec);
        ~VulkanFramebuffer();

        uint32_t GetWidth()  const override { return m_Spec.Width; }
        uint32_t GetHeight() const override { return m_Spec.Height; }

        const FramebufferSpec& GetSpec() const override { return m_Spec; }

    private:
        VkDevice       m_Device = VK_NULL_HANDLE;
        VkFramebuffer  m_Framebuffer = VK_NULL_HANDLE;
        FramebufferSpec m_Spec{};

    protected:
        void* GetHandleImpl() const override { return m_Framebuffer; }
    };
}