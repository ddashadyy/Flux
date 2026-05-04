#pragma once

#include "Flux/Renderer/RHITexture.h"
#include <vulkan/vulkan.h>

namespace Flux {

    class VulkanSampler final : public RHISampler
    {
    public:
        VulkanSampler(VkDevice device, const SamplerSpec& spec);
        ~VulkanSampler();

        const SamplerSpec& GetSpec() const override { return m_Spec; }

    private:
        VkDevice     m_Device = VK_NULL_HANDLE;
        VkSampler    m_Sampler = VK_NULL_HANDLE;
        SamplerSpec  m_Spec{};

    protected:
        void* GetHandleImpl() const override { return m_Sampler; }
    };

} // namespace Flux
