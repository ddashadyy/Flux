#pragma once


#include "FLux/Renderer/RHISwapchain.h"

#include "VulkanRenderPass.h"
#include "VulkanSwapchainTexture.h"
#include "VulkanFramebuffer.h"

#include <vulkan/vulkan.h>

namespace Flux {

    class VulkanSwapchain : public RHISwapchain {
    public:
        VulkanSwapchain(VkDevice device,
            VkPhysicalDevice physicalDevice,
            VkSurfaceKHR     surface,
            VkQueue          presentQueue,
            uint32_t         width,
            uint32_t         height);
        ~VulkanSwapchain();

        uint32_t AcquireNextImage(RHISemaphore* semaphore)             override;
        void     Present(RHISemaphore* semaphore, uint32_t imageIndex) override;
        void     Resize(uint32_t w, uint32_t h)                        override;

        uint32_t   GetCurrentImageIndex() const override { return m_CurrentImageIndex; }
        uint32_t   GetImageCount()        const override { return static_cast<uint32_t>(m_Images.size()); }
        Format     GetFormat()            const override;
        VkExtent2D GetExtent()            const { return m_Extent; }

        RHITexture* GetColorTarget(uint32_t index) const override { return m_ColorTargets[index].get(); }


    private:
        void CreateSwapchain(uint32_t width, uint32_t height);
        void CreateImageViews();
        void Cleanup();

    private:
        VkDevice         m_Device = VK_NULL_HANDLE;
        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
        VkSurfaceKHR     m_Surface = VK_NULL_HANDLE;
        VkQueue          m_PresentQueue = VK_NULL_HANDLE;

        VkSwapchainKHR             m_Swapchain = VK_NULL_HANDLE;
        VkExtent2D                 m_Extent{};
        VkFormat                   m_Format{};
        uint32_t                   m_CurrentImageIndex = 0;

        std::vector<VkImage>       m_Images       = {};
        std::vector<VkImageView>   m_ImageViews   = {};

        std::vector<Scope<VulkanSwapchainTexture>> m_ColorTargets = {};
    };

}