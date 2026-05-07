#pragma once

#include "Flux/Renderer/RHITexture.h"
#include <vulkan/vulkan.h>

namespace Flux {

    class VulkanSampler final : public RHISampler
    {
    public:
        VulkanSampler(VkDevice device, const SamplerSpec& spec);
        ~VulkanSampler();

		void* GetHandle() const override { return m_Sampler; }

        const SamplerSpec& GetSpec() const override { return m_Spec; }

    private:
        VkDevice     m_Device = VK_NULL_HANDLE;
        VkSampler    m_Sampler = VK_NULL_HANDLE;
        SamplerSpec  m_Spec{};
    };

} // namespace Flux
