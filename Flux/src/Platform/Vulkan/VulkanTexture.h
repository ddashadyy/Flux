#pragma once

#include "Flux/Renderer/RHITexture.h"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace Flux {

    // -------------------------------------------------------------------------
    // VulkanTexture — обычная GPU текстура
    // Самплер живёт отдельно (VulkanSampler), текстура его не знает.
    // Upload через Device::ImmediateSubmit — текстура сама не делает submit.
    // -------------------------------------------------------------------------
    class VulkanTexture final : public RHITexture
    {
    public:
        VulkanTexture(VkDevice device, VmaAllocator allocator, const TextureSpec& spec);
        ~VulkanTexture();

        const TextureSpec& GetSpec() const override { return m_Spec; }

        // Upload пикселей — вызывается снаружи через Device::ImmediateSubmit
        // cmd — уже начатый command buffer
        void RecordUpload(VkCommandBuffer cmd, VkBuffer stagingBuffer);

        // Запись barrier-а в текущий cmd buffer
        void RecordBarrier(VkCommandBuffer cmd,
            VkImageLayout   oldLayout,
            VkImageLayout   newLayout,
            VkAccessFlags   srcAccess,
            VkAccessFlags   dstAccess,
            VkPipelineStageFlags srcStage,
            VkPipelineStageFlags dstStage);

        // Генерация mip-ов — вызывается после upload в том же cmd
        void RecordGenerateMipmaps(VkCommandBuffer cmd);

        // SetData: создаёт staging buffer сама, но submit делает через колбэк
        // Принимает лямбду ImmediateSubmit-совместимого вида
        void SetData(const void* data, uint32_t size) override;

        VkImage     GetImage()     const { return m_Image; }
        VkImageView GetImageView() const { return m_ImageView; }

        void* GetNativeImageView() const override { return static_cast<void*>(m_ImageView); }

        void SetupForSetData(VkQueue queue, VkCommandPool pool);

    private:
        // Только для SetData — создаёт временный fence/submit
        // Нормальный путь — через Device::ImmediateSubmit
        void SubmitOneTime(std::function<void(VkCommandBuffer)>&& fn);

    private:
        VkImage       m_Image = VK_NULL_HANDLE;
        VkImageView   m_ImageView = VK_NULL_HANDLE;

        VmaAllocator  m_Allocator = VK_NULL_HANDLE;
        VmaAllocation m_Allocation = VK_NULL_HANDLE;

        VkDevice      m_Device = VK_NULL_HANDLE;
        VkQueue       m_GraphicsQueue = VK_NULL_HANDLE; 
        VkCommandPool m_CommandPool = VK_NULL_HANDLE; 

        TextureSpec   m_Spec{};

    protected:
        void* GetHandleImpl() const override { return m_Image; }
    };

    // -------------------------------------------------------------------------
    // VulkanSwapchainTexture — обёртка над swapchain image
    // Не владеет VkImage — им владеет swapchain.
    // -------------------------------------------------------------------------
    class VulkanSwapchainTexture final : public RHITexture
    {
    public:
        VulkanSwapchainTexture(VkImage image, VkImageView imageView,
            Format format, uint32_t width, uint32_t height)
            : m_Image(image), m_ImageView(imageView)
        {
            m_Spec.ImageFormat = format;
            m_Spec.Width = width;
            m_Spec.Height = height;
            m_Spec.Usage = TextureUsage::RenderTarget;
        }

        const TextureSpec& GetSpec() const override { return m_Spec; }

        VkImage     GetImage()     const { return m_Image; }
        VkImageView GetImageView() const { return m_ImageView; }

        void* GetNativeImageView() const override { return static_cast<void*>(m_ImageView); }
        void  SetData(const void*, uint32_t) override { FL_CORE_ASSERT(false, "Cannot SetData on swapchain texture!"); }

    private:
        VkImage     m_Image = VK_NULL_HANDLE;
        VkImageView m_ImageView = VK_NULL_HANDLE;
        TextureSpec m_Spec{};

    protected:
        void* GetHandleImpl() const override { return m_Image; }
    };

} // namespace Flux
