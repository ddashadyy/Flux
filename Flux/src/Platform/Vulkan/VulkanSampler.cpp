#include "flpch.h"
#include "VulkanSampler.h"
#include "VulkanCommon.h"

namespace {

    using Flux::FilterMode;
    using Flux::MipMapMode;
    using Flux::AddressMode;
    using Flux::BorderColor;
    using Flux::CompareOp;

    constexpr VkFilter GetVkFilter(FilterMode mode)
    {
        using enum FilterMode;

        switch (mode)
        {
        case Nearest: return VK_FILTER_NEAREST;
        case Linear:  return VK_FILTER_LINEAR;
        }

        FL_CONSTEXPR_ASSERT(false, "Unknown FilterMode");
        return VK_FILTER_LINEAR;
    }

    constexpr VkSamplerMipmapMode GetVkMipMode(MipMapMode mode)
    {
        using enum MipMapMode;

        switch (mode)
        {
        case Nearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case Linear:  return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        }

        FL_CONSTEXPR_ASSERT(false, "Unknown MipMapMode");
        return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    }

    constexpr VkSamplerAddressMode GetVkAddressMode(AddressMode mode)
    {
        using enum AddressMode;

        switch (mode)
        {
        case Repeat:         return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case MirroredRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case ClampToEdge:    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case ClampToBorder:  return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        }

        FL_CONSTEXPR_ASSERT(false, "Unknown AddressMode");
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }

    constexpr VkBorderColor GetVkBorderColor(BorderColor color)
    {
        using enum BorderColor;

        switch (color)
        {
        case TransparentBlack: return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        case OpaqueBlack:      return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        case OpaqueWhite:      return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        }

        FL_CONSTEXPR_ASSERT(false, "Unknown BorderColor");
        return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    }

    constexpr VkCompareOp GetVkCompareOp(CompareOp op)
    {
        using enum CompareOp;

        switch (op)
        {
        case Never:          return VK_COMPARE_OP_NEVER;
        case Less:           return VK_COMPARE_OP_LESS;
        case Equal:          return VK_COMPARE_OP_EQUAL;
        case LessOrEqual:    return VK_COMPARE_OP_LESS_OR_EQUAL;
        case Greater:        return VK_COMPARE_OP_GREATER;
        case NotEqual:       return VK_COMPARE_OP_NOT_EQUAL;
        case GreaterOrEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case Always:         return VK_COMPARE_OP_ALWAYS;
        }

        FL_CONSTEXPR_ASSERT(false, "Unknown CompareOp");
        return VK_COMPARE_OP_ALWAYS;
    }

} // anonymous namespace

namespace Flux {

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
