#pragma once


#include "Flux/Renderer/RHITexture.h"
#include "Flux/Core/Base.h"

#include <vulkan/vulkan.h>

namespace Flux {

    // Тонкая обёртка вокруг swapchain image — не владеет ресурсами
    class VulkanSwapchainTexture final : public RHITexture
    {
    public:
        VulkanSwapchainTexture(VkImage image, VkImageView imageView,
            Format format, uint32_t width, uint32_t height)
            : m_Image(image), m_ImageView(imageView)
            , m_Format(format), m_Width(width), m_Height(height)
        {
        }

        uint32_t GetWidth()     const override { return m_Width; }
        uint32_t GetHeight()    const override { return m_Height; }
        Format   GetFormat()    const override { return m_Format; }
        uint32_t GetMipLevels() const override { return 1; }

        void SetData(const void*, uint32_t) override
        {
            FL_CORE_ASSERT(false, "Cannot SetData on swapchain texture!");
        }

        VkImage     GetImage()     const { return m_Image; }
        VkImageView GetImageView() const { return m_ImageView; }

        void* GetNativeImageView() const override { return static_cast<void*>(m_ImageView); }

    private:
        VkImage     m_Image = VK_NULL_HANDLE;
        VkImageView m_ImageView = VK_NULL_HANDLE;
        Format      m_Format = Format::B8G8R8A8_UNORM;
        uint32_t    m_Width = 0;
        uint32_t    m_Height = 0;
    };
}