#pragma once

#include "Flux/Renderer/RHIDescriptorSet.h"
#include <vulkan/vulkan.h>

namespace Flux {

    class VulkanDescriptorSetLayout final : public RHIDescriptorSetLayout
    {
    public:
        VulkanDescriptorSetLayout(VkDevice device, const DescriptorSetLayoutDesc& desc);
        ~VulkanDescriptorSetLayout();

        uint32_t GetBindingCount() const override { return static_cast<uint32_t>(m_Desc.Bindings.size()); }
        const DescriptorSetLayoutDesc& GetDesc() const override { return m_Desc; }

    private:
        VkDevice                m_Device = VK_NULL_HANDLE;
        VkDescriptorSetLayout   m_Layout = VK_NULL_HANDLE;
        DescriptorSetLayoutDesc m_Desc{};

    protected:
        void* GetHandleImpl() const override { return m_Layout; }
    };


    class VulkanDescriptorSet final : public RHIDescriptorSet
    {
    public:
        VulkanDescriptorSet(VkDevice device, VkDescriptorPool pool,
            VkDescriptorSetLayout layout,
            const DescriptorSetLayoutDesc& layoutDesc);
        ~VulkanDescriptorSet();

        void BindBuffer(uint32_t binding, const RHIBuffer* buffer)  override;
        void BindTexture(uint32_t binding, const RHITexture* texture) override;
        void BindTexture(uint32_t binding, const RHITexture* texture, const RHISampler* sampler) override;
        void BindSampler(uint32_t binding, const RHISampler* sampler) override;
        void Update() override;

    private:
        VkDevice         m_Device = VK_NULL_HANDLE;
        VkDescriptorPool m_Pool = VK_NULL_HANDLE;
        VkDescriptorSet  m_DescriptorSet = VK_NULL_HANDLE;

        DescriptorSetLayoutDesc m_LayoutDesc{};

        struct PendingWrite
        {
            VkWriteDescriptorSet       Write{};
            VkDescriptorBufferInfo     BufferInfo{};
            VkDescriptorImageInfo      ImageInfo{};
            bool                       IsBuffer = false;
        };
        std::vector<PendingWrite> m_Pending;

    protected:
        void* GetHandleImpl() const override { return m_DescriptorSet; }
    };

} // namespace Flux
