#include "flpch.h"
#include "VulkanDescriptorSet.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanSampler.h"
#include "VulkanCommon.h"

namespace Flux {

    static VkDescriptorType ToVkDescriptorType(DescriptorType type)
    {
        switch (type)
        {
        case DescriptorType::UniformBuffer:        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case DescriptorType::StorageBuffer:        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case DescriptorType::CombinedImageSampler: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case DescriptorType::SampledImage:         return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case DescriptorType::Sampler:              return VK_DESCRIPTOR_TYPE_SAMPLER;
        case DescriptorType::StorageImage:         return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        }
        FL_CORE_ASSERT(false, "Unknown DescriptorType");
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }

    // -------------------------------------------------------------------------
    // Layout
    // -------------------------------------------------------------------------

    VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VkDevice device, const DescriptorSetLayoutDesc& desc)
        : m_Device(device), m_Desc(desc)
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        bindings.reserve(desc.Bindings.size());

        for (const auto& b : desc.Bindings)
        {
            VkDescriptorSetLayoutBinding vkb{};
            vkb.binding = b.Binding;
            vkb.descriptorType = ToVkDescriptorType(b.Type);
            vkb.descriptorCount = b.Count;
            vkb.stageFlags = GetShaderStageFlags(b.Stage);
            vkb.pImmutableSamplers = nullptr;
            bindings.push_back(vkb);
        }

        VkDescriptorSetLayoutCreateInfo info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
        info.bindingCount = static_cast<uint32_t>(bindings.size());
        info.pBindings = bindings.data();

        FL_CORE_ASSERT(vkCreateDescriptorSetLayout(m_Device, &info, nullptr, &m_Layout) == VK_SUCCESS,
            "Failed to create DescriptorSetLayout");

        FL_CORE_INFO("Created Vulkan DescriptorSetLayout");
    }

    VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
    {
        vkDestroyDescriptorSetLayout(m_Device, m_Layout, nullptr);
    }

    // -------------------------------------------------------------------------
    // Descriptor set
    // -------------------------------------------------------------------------

    VulkanDescriptorSet::VulkanDescriptorSet(VkDevice device, VkDescriptorPool pool,
        VkDescriptorSetLayout layout,
        const DescriptorSetLayoutDesc& layoutDesc)
        : m_Device(device), m_Pool(pool), m_LayoutDesc(layoutDesc)
    {
        VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
        allocInfo.descriptorPool = pool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout;

        FL_CORE_ASSERT(vkAllocateDescriptorSets(m_Device, &allocInfo, &m_DescriptorSet) == VK_SUCCESS,
            "Failed to allocate DescriptorSet");

        FL_CORE_INFO("Created Vulkan DescriptorSet");
    }

    VulkanDescriptorSet::~VulkanDescriptorSet()
    {
        vkFreeDescriptorSets(m_Device, m_Pool, 1, &m_DescriptorSet);
    }

    // -------------------------------------------------------------------------
    // Bind — каждый pending write хранит свои info inline, указатели не протухают
    // -------------------------------------------------------------------------

    void VulkanDescriptorSet::BindBuffer(uint32_t binding, const RHIBuffer* buffer)
    {
        // Определяем реальный тип по layout — не хардкодим Uniform
        DescriptorType descType = DescriptorType::UniformBuffer;
        for (const auto& b : m_LayoutDesc.Bindings)
        {
            if (b.Binding == binding) { descType = b.Type; break; }
        }

        const auto* vkBuf = static_cast<const VulkanBuffer*>(buffer);

        PendingWrite pw{};
        pw.BufferInfo.buffer = vkBuf->GetHandle<VkBuffer>();
        pw.BufferInfo.offset = 0;
        pw.BufferInfo.range = vkBuf->GetSize();

        pw.Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        pw.Write.dstSet = m_DescriptorSet;
        pw.Write.dstBinding = binding;
        pw.Write.dstArrayElement = 0;
        pw.Write.descriptorCount = 1;
        pw.Write.descriptorType = ToVkDescriptorType(descType);
        pw.Write.pBufferInfo = &pw.BufferInfo; // указывает внутрь struct — безопасно после push_back если reserve

        m_Pending.push_back(pw);
        // Обновляем указатель после push_back (вектор мог переаллоцироваться)
        m_Pending.back().Write.pBufferInfo = &m_Pending.back().BufferInfo;
    }

    void VulkanDescriptorSet::BindTexture(uint32_t binding, const RHITexture* texture)
    {
        DescriptorType descType = DescriptorType::CombinedImageSampler;
        for (const auto& b : m_LayoutDesc.Bindings)
        {
            if (b.Binding == binding) { descType = b.Type; break; }
        }

        const auto* vkTex = static_cast<const VulkanTexture*>(texture);

        // Layout текстуры в шейдере — зависит от типа дескриптора
        VkImageLayout layout = (descType == DescriptorType::StorageImage)
            ? VK_IMAGE_LAYOUT_GENERAL
            : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        PendingWrite pw{};
        pw.ImageInfo.imageView = vkTex->GetImageView();
        pw.ImageInfo.imageLayout = layout;
        pw.ImageInfo.sampler = VK_NULL_HANDLE; // самплер BindSampler-ом или отдельно

        pw.Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        pw.Write.dstSet = m_DescriptorSet;
        pw.Write.dstBinding = binding;
        pw.Write.dstArrayElement = 0;
        pw.Write.descriptorCount = 1;
        pw.Write.descriptorType = ToVkDescriptorType(descType);

        m_Pending.push_back(pw);
        m_Pending.back().Write.pImageInfo = &m_Pending.back().ImageInfo;
    }

    void VulkanDescriptorSet::BindTexture(uint32_t binding, const RHITexture* texture, const RHISampler* sampler)
    {
        const auto* vkTex = static_cast<const VulkanTexture*>(texture);
        const auto* vkSampler = static_cast<const VulkanSampler*>(sampler);

        PendingWrite pw{};
        pw.IsBuffer = false;
        pw.ImageInfo.imageView = vkTex->GetImageView();
        pw.ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        pw.ImageInfo.sampler = vkSampler ? vkSampler->GetHandle<VkSampler>() : VK_NULL_HANDLE;

        pw.Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        pw.Write.dstSet = m_DescriptorSet;
        pw.Write.dstBinding = binding;
        pw.Write.dstArrayElement = 0;
        pw.Write.descriptorCount = 1;
        pw.Write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

        m_Pending.push_back(pw);
    }

    void VulkanDescriptorSet::BindSampler(uint32_t binding, const RHISampler* sampler)
    {
        DescriptorType descType = DescriptorType::Sampler;
        for (const auto& b : m_LayoutDesc.Bindings)
        {
            if (b.Binding == binding) { descType = b.Type; break; }
        }

        const auto* vkSampler = static_cast<const VulkanSampler*>(sampler);

        PendingWrite pw{};
        pw.ImageInfo.sampler = vkSampler->GetHandle<VkSampler>();
        pw.ImageInfo.imageView = VK_NULL_HANDLE;
        pw.ImageInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        pw.Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        pw.Write.dstSet = m_DescriptorSet;
        pw.Write.dstBinding = binding;
        pw.Write.dstArrayElement = 0;
        pw.Write.descriptorCount = 1;
        pw.Write.descriptorType = ToVkDescriptorType(descType);

        m_Pending.push_back(pw);
        m_Pending.back().Write.pImageInfo = &m_Pending.back().ImageInfo;
    }

    // -------------------------------------------------------------------------
    // Update — указатели уже корректны внутри каждого PendingWrite
    // -------------------------------------------------------------------------

    void VulkanDescriptorSet::Update()
    {
        if (m_Pending.empty())
            return;

        // Пересобираем указатели прямо здесь — после всех push_back вектор уже не реаллоцируется
        std::vector<VkWriteDescriptorSet> writes;
        writes.reserve(m_Pending.size());

        for (auto& pw : m_Pending)
        {
            VkWriteDescriptorSet w = pw.Write;
            if (pw.IsBuffer)
                w.pBufferInfo = &pw.BufferInfo;
            else
                w.pImageInfo = &pw.ImageInfo;
            writes.push_back(w);
        }

        vkUpdateDescriptorSets(m_Device,
            static_cast<uint32_t>(writes.size()), writes.data(),
            0, nullptr);

        m_Pending.clear();
    }

} // namespace Flux
