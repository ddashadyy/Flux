#include "flpch.h"
#include "VulkanSwapchain.h"
#include "VulkanSemaphore.h"


namespace Flux {
        
    
    static VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
    {
        for (auto& format : formats)
        {
            if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return format;
        }
        return formats[0];
    }

    static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& modes)
    {
        for (auto& mode : modes)
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) return mode;
        
        return VK_PRESENT_MODE_FIFO_KHR; 
    }

    static VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height)
    {
        if (capabilities.currentExtent.width != UINT32_MAX)
            return capabilities.currentExtent;

        VkExtent2D extent{ width, height };
        extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        return extent;
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

    uint32_t VulkanSwapchain::AcquireNextImage(RHISemaphore* semaphore)
    {
        VkSemaphore vkSemaphore = static_cast<VulkanSemaphore*>(semaphore)->GetHandle();

        vkAcquireNextImageKHR(m_Device, m_Swapchain, UINT64_MAX,
            vkSemaphore, VK_NULL_HANDLE, &m_CurrentImageIndex);

        return m_CurrentImageIndex;
    }

    void VulkanSwapchain::Present(RHISemaphore* semaphore, uint32_t imageIndex)
    {
        VkSemaphore vkSemaphore = static_cast<VulkanSemaphore*>(semaphore)->GetHandle();

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &vkSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_Swapchain;
        presentInfo.pImageIndices = &imageIndex;

        vkQueuePresentKHR(m_PresentQueue, &presentInfo);
    }

    void VulkanSwapchain::Resize(uint32_t w, uint32_t h)
    {
        vkDeviceWaitIdle(m_Device);
        Cleanup();
        CreateSwapchain(w, h);
        CreateImageViews();

        FL_CORE_INFO("Recreated Vulkan Swapchain {0}x{1}", w, h);
    }

    Format VulkanSwapchain::GetFormat() const
    {
        switch (m_Format)
        {
        case VK_FORMAT_B8G8R8A8_UNORM: return Format::B8G8R8A8_UNORM;
        case VK_FORMAT_R8G8B8A8_UNORM: return Format::R8G8B8A8_UNORM;
        default:                       return Format::B8G8R8A8_UNORM;
        }
    }

    void VulkanSwapchain::CreateSwapchain(uint32_t width, uint32_t height)
    {
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, m_Surface, &capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &formatCount, formats.data());

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &presentModeCount, nullptr);
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &presentModeCount, presentModes.data());

        auto surfaceFormat = ChooseSurfaceFormat(formats);
        auto presentMode = ChoosePresentMode(presentModes);
        auto extent = ChooseExtent(capabilities, width, height);

        uint32_t imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0)
            imageCount = std::min(imageCount, capabilities.maxImageCount);

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_Surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.preTransform = capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        FL_CORE_ASSERT(vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_Swapchain) == VK_SUCCESS,
            "Failed to create Swapchain");

        // images
        vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, nullptr);
        m_Images.resize(imageCount);
        vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, m_Images.data());

        m_Format = surfaceFormat.format;
        m_Extent = extent;
    }

    void VulkanSwapchain::CreateImageViews()
    {
        m_ImageViews.resize(m_Images.size());
        m_ColorTargets.resize(m_Images.size());

        for (size_t i = 0; i < m_Images.size(); i++)
        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
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
                m_Images[i], m_ImageViews[i], GetFormat(),
                m_Extent.width, m_Extent.height
            );
        }
    }

    void VulkanSwapchain::Cleanup()
    {
        for (auto& imageView : m_ImageViews)
            vkDestroyImageView(m_Device, imageView, nullptr);

        vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
    }


}