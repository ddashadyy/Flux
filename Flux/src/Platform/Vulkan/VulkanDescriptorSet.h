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
        VulkanDescriptorSet(VkDevice device, VkDescriptorPool pool, VkDescriptorSetLayout layout);
        ~VulkanDescriptorSet();

        void BindBuffer(uint32_t binding, const RHIBuffer* buffer) override;
        void BindTexture(uint32_t binding, const RHITexture* texture) override;
        void Update() override;


    private:
        VkDevice         m_Device        = VK_NULL_HANDLE;
        VkDescriptorPool m_Pool          = VK_NULL_HANDLE;
        VkDescriptorSet  m_DescriptorSet = VK_NULL_HANDLE;

        std::vector<VkWriteDescriptorSet>   m_PendingWrites;
        std::vector<VkDescriptorBufferInfo> m_BufferInfos;
        std::vector<VkDescriptorImageInfo>  m_ImageInfos;

    protected:
        void* GetHandleImpl() const override { return m_DescriptorSet; }
    };
}
