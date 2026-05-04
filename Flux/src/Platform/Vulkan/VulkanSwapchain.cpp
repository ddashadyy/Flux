#include "flpch.h"
#include "VulkanSwapchain.h"
#include "VulkanSemaphore.h"

namespace Flux {

    static VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
    {
        for (const auto& f : formats)
            if (f.format == VK_FORMAT_B8G8R8A8_UNORM && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return f;
        return formats[0];
    }

    static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& modes)
    {
        for (const auto& m : modes)
            if (m == VK_PRESENT_MODE_MAILBOX_KHR) return m;
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    static VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& caps, uint32_t w, uint32_t h)
    {
        if (caps.currentExtent.width != UINT32_MAX)
            return caps.currentExtent;
        return {
            std::clamp(w, caps.minImageExtent.width,  caps.maxImageExtent.width),
            std::clamp(h, caps.minImageExtent.height, caps.maxImageExtent.height)
        };
    }

    VulkanSwapchain::VulkanSwapchain(VkDevice device, VkPhysicalDevice physicalDevice,
        VkSurfaceKHR surface, VkQueue presentQueue,
        uint32_t width, uint32_t height)
        : m_Device(device), m_PhysicalDevice(physicalDevice),
        m_Surface(surface), m_PresentQueue(presentQueue)
    {
        CreateSwapchain(width, height);
        CreateImageViews();
        FL_CORE_INFO("Created Vulkan Swapchain {}x{}", width, height);
    }

    VulkanSwapchain::~VulkanSwapchain()
    {
        Cleanup();
    }

    // -------------------------------------------------------------------------
    // AcquireNextImage — обрабатываем OUT_OF_DATE
    // -------------------------------------------------------------------------

    uint32_t VulkanSwapchain::AcquireNextImage(RHISemaphore* semaphore)
    {
        VkSemaphore vkSem = semaphore->GetHandle<VkSemaphore>();

        VkResult result = vkAcquireNextImageKHR(
            m_Device, m_Swapchain, UINT64_MAX,
            vkSem, VK_NULL_HANDLE, &m_CurrentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            // Swapchain устарел (resize произошёл до AcquireNextImage)
            // Сигнализируем наверх — Renderer должен Resize
            m_NeedsResize = true;
            return UINT32_MAX;
        }

        // SUBOPTIMAL — продолжаем этот кадр, resize после Present
        if (result == VK_SUBOPTIMAL_KHR)
            m_NeedsResize = true;

        FL_CORE_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR,
            "vkAcquireNextImageKHR failed");

        return m_CurrentImageIndex;
    }

    // -------------------------------------------------------------------------
    // Present — обрабатываем OUT_OF_DATE / SUBOPTIMAL
    // -------------------------------------------------------------------------

    void VulkanSwapchain::Present(RHISemaphore* semaphore, uint32_t imageIndex)
    {
        VkSemaphore vkSem = semaphore->GetHandle<VkSemaphore>();

        VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &vkSem;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_Swapchain;
        presentInfo.pImageIndices = &imageIndex;

        VkResult result = vkQueuePresentKHR(m_PresentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
            m_NeedsResize = true;
        else
            FL_CORE_ASSERT(result == VK_SUCCESS, "vkQueuePresentKHR failed");
    }

    void VulkanSwapchain::Resize(uint32_t w, uint32_t h)
    {
        vkDeviceWaitIdle(m_Device);
        Cleanup();
        CreateSwapchain(w, h);
        CreateImageViews();
        m_NeedsResize = false;
        FL_CORE_INFO("Recreated Vulkan Swapchain {}x{}", w, h);
    }

    Format VulkanSwapchain::GetFormat() const
    {
        switch (m_Format)
        {
        case VK_FORMAT_B8G8R8A8_UNORM: return Format::B8G8R8A8_UNORM;
        case VK_FORMAT_R8G8B8A8_UNORM: return Format::R8G8B8A8_UNORM;
        default:                        return Format::B8G8R8A8_UNORM;
        }
    }

    void VulkanSwapchain::CreateSwapchain(uint32_t width, uint32_t height)
    {
        VkSurfaceCapabilitiesKHR caps;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, m_Surface, &caps);

        uint32_t fmtCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &fmtCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(fmtCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &fmtCount, formats.data());

        uint32_t pmCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &pmCount, nullptr);
        std::vector<VkPresentModeKHR> presentModes(pmCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &pmCount, presentModes.data());

        auto surfaceFmt = ChooseSurfaceFormat(formats);
        auto presentMode = ChoosePresentMode(presentModes);
        auto extent = ChooseExtent(caps, width, height);

        uint32_t imageCount = caps.minImageCount + 1;
        if (caps.maxImageCount > 0)
            imageCount = std::min(imageCount, caps.maxImageCount);

        VkSwapchainCreateInfoKHR info{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
        info.surface = m_Surface;
        info.minImageCount = imageCount;
        info.imageFormat = surfaceFmt.format;
        info.imageColorSpace = surfaceFmt.colorSpace;
        info.imageExtent = extent;
        info.imageArrayLayers = 1;
        info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.preTransform = caps.currentTransform;
        info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        info.presentMode = presentMode;
        info.clipped = VK_TRUE;
        info.oldSwapchain = VK_NULL_HANDLE;

        FL_CORE_ASSERT(vkCreateSwapchainKHR(m_Device, &info, nullptr, &m_Swapchain) == VK_SUCCESS,
            "Failed to create Swapchain");

        vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, nullptr);
        m_Images.resize(imageCount);
        vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, m_Images.data());

        m_Format = surfaceFmt.format;
        m_Extent = extent;
    }

    void VulkanSwapchain::CreateImageViews()
    {
        m_ImageViews.resize(m_Images.size());
        m_ColorTargets.resize(m_Images.size());

        for (size_t i = 0; i < m_Images.size(); i++)
        {
            VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
            viewInfo.image = m_Images[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = m_Format;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            FL_CORE_ASSERT(vkCreateImageView(m_Device, &viewInfo, nullptr, &m_ImageViews[i]) == VK_SUCCESS,
                "Failed to create Swapchain ImageView");

            m_ColorTargets[i] = CreateScope<VulkanSwapchainTexture>(
                m_Images[i], m_ImageViews[i], GetFormat(), m_Extent.width, m_Extent.height);
        }
    }

    void VulkanSwapchain::Cleanup()
    {
        // Сначала очищаем texture wrappers — они не владеют image/view, но держат указатели
        m_ColorTargets.clear();

        for (auto& iv : m_ImageViews)
            vkDestroyImageView(m_Device, iv, nullptr);
        m_ImageViews.clear();
        m_Images.clear();

        vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
        m_Swapchain = VK_NULL_HANDLE;
    }

} // namespace Flux
