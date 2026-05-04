#include "flpch.h"
#include "VulkanSampler.h"
#include "VulkanCommon.h"

namespace Flux {

    static VkFilter GetVkFilter(FilterMode mode)
    {
        switch (mode)
        {
        case FilterMode::Nearest: return VK_FILTER_NEAREST;
        case FilterMode::Linear:  return VK_FILTER_LINEAR;
        }
        return VK_FILTER_LINEAR;
    }

    static VkSamplerMipmapMode GetVkMipMode(MipMapMode mode)
    {
        switch (mode)
        {
        case MipMapMode::Nearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case MipMapMode::Linear:  return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        }
        return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    }

    static VkSamplerAddressMode GetVkAddressMode(AddressMode mode)
    {
        switch (mode)
        {
        case AddressMode::Repeat:         return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case AddressMode::MirroredRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case AddressMode::ClampToEdge:    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case AddressMode::ClampToBorder:  return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        }
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }

    static VkBorderColor GetVkBorderColor(BorderColor color)
    {
        switch (color)
        {
        case BorderColor::TransparentBlack: return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        case BorderColor::OpaqueBlack:      return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        case BorderColor::OpaqueWhite:      return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        }
        return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    }

    static VkCompareOp GetVkCompareOp(CompareOp op)
    {
        switch (op)
        {
        case CompareOp::Never:          return VK_COMPARE_OP_NEVER;
        case CompareOp::Less:           return VK_COMPARE_OP_LESS;
        case CompareOp::Equal:          return VK_COMPARE_OP_EQUAL;
        case CompareOp::LessOrEqual:    return VK_COMPARE_OP_LESS_OR_EQUAL;
        case CompareOp::Greater:        return VK_COMPARE_OP_GREATER;
        case CompareOp::NotEqual:       return VK_COMPARE_OP_NOT_EQUAL;
        case CompareOp::GreaterOrEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case CompareOp::Always:         return VK_COMPARE_OP_ALWAYS;
        }
        return VK_COMPARE_OP_ALWAYS;
    }

    VulkanSampler::VulkanSampler(VkDevice device, const SamplerSpec& spec)
        : m_Device(device), m_Spec(spec)
    {
        VkSamplerCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.magFilter = GetVkFilter(spec.MagFilter);
        info.minFilter = GetVkFilter(spec.MinFilter);
        info.mipmapMode = GetVkMipMode(spec.MipMode);
        info.addressModeU = GetVkAddressMode(spec.AddressU);
        info.addressModeV = GetVkAddressMode(spec.AddressV);
        info.addressModeW = GetVkAddressMode(spec.AddressW);
        info.mipLodBias = spec.MipLodBias;
        info.anisotropyEnable = spec.AnisotropyEnable ? VK_TRUE : VK_FALSE;
        info.maxAnisotropy = spec.MaxAnisotropy;
        info.compareEnable = spec.CompareEnable ? VK_TRUE : VK_FALSE;
        info.compareOp = GetVkCompareOp(spec.Compare);
        info.minLod = spec.MinLod;
        info.maxLod = spec.MaxLod;
        info.borderColor = GetVkBorderColor(spec.Border);
        info.unnormalizedCoordinates = VK_FALSE;

        FL_CORE_ASSERT(vkCreateSampler(m_Device, &info, nullptr, &m_Sampler) == VK_SUCCESS,
            "Failed to create Vulkan Sampler");

        FL_CORE_INFO("Created Vulkan Sampler");
    }

    VulkanSampler::~VulkanSampler()
    {
        vkDestroySampler(m_Device, m_Sampler, nullptr);
    }

} // namespace Flux
