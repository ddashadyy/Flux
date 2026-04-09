#pragma once
#include "FLux/Renderer/RHISwapchain.h"
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

        uint32_t AcquireNextImage(RHISemaphore* semaphore) override;
        void     Present(RHISemaphore* semaphore)          override;
        void     Resize(uint32_t w, uint32_t h)            override;
        inline uint32_t GetCurrentImageIndex()              const override { return m_CurrentImageIndex; }
        Format   GetFormat()                         const override;

        inline VkFramebuffer GetCurrentFramebuffer() const { return m_Framebuffers[m_CurrentImageIndex]; }
        inline VkExtent2D    GetExtent()             const { return m_Extent; }
        inline VkRenderPass  GetRenderPass()         const { return m_RenderPass; }

        inline uint32_t GetImageCount() const { return static_cast<uint32_t>(m_Images.size()); }

    private:
        void CreateSwapchain(uint32_t width, uint32_t height);
        void CreateImageViews();
        void CreateFramebuffers();
        void Cleanup();

    private:
        VkDevice         m_Device = VK_NULL_HANDLE;
        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
        VkSurfaceKHR     m_Surface = VK_NULL_HANDLE;
        VkQueue          m_PresentQueue = VK_NULL_HANDLE;

        VkSwapchainKHR             m_Swapchain = VK_NULL_HANDLE;
        VkRenderPass               m_RenderPass = VK_NULL_HANDLE;
        VkExtent2D                 m_Extent{};
        VkFormat                   m_Format{};
        uint32_t                   m_CurrentImageIndex = 0;

        std::vector<VkImage>       m_Images       = {};
        std::vector<VkImageView>   m_ImageViews   = {};
        std::vector<VkFramebuffer> m_Framebuffers = {};
    };

}