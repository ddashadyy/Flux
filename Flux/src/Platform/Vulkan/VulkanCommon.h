#pragma once

#include "Flux/Renderer/RHICommon.h"
#include <vulkan/vulkan.h>
#include <string_view>
#include <cassert> 

namespace Flux {

    constexpr VkShaderStageFlags GetShaderStageFlags(ShaderStage stage)
    {
        VkShaderStageFlags flags = 0;

        using enum ShaderStage;

        if (HasFlag(stage, Vertex))
            flags |= VK_SHADER_STAGE_VERTEX_BIT;

        if (HasFlag(stage, Fragment))
            flags |= VK_SHADER_STAGE_FRAGMENT_BIT;

        if (HasFlag(stage, Compute))
            flags |= VK_SHADER_STAGE_COMPUTE_BIT;

        FL_CONSTEXPR_ASSERT(flags != 0, "No Shader Stage specified!");

        return flags;
    }

    constexpr VkSampleCountFlagBits GetSampleCount(SampleCount samples)
    {
        using enum SampleCount;

        switch (samples)
        {
        case x1: return VK_SAMPLE_COUNT_1_BIT;
        case x2: return VK_SAMPLE_COUNT_2_BIT;
        case x4: return VK_SAMPLE_COUNT_4_BIT;
        case x8: return VK_SAMPLE_COUNT_8_BIT;
        }

        FL_CONSTEXPR_ASSERT(false, "Unknown SampleCount!");
        return VK_SAMPLE_COUNT_1_BIT;
    }

    constexpr VkFormat GetVkFormat(Format format)
    {
        using enum Format;

        switch (format)
        {
        case R8G8B8A8_UNORM:      return VK_FORMAT_R8G8B8A8_UNORM;
        case B8G8R8A8_UNORM:      return VK_FORMAT_B8G8R8A8_UNORM;
        case D32_SFLOAT:          return VK_FORMAT_D32_SFLOAT;
        case R32G32_SFLOAT:       return VK_FORMAT_R32G32_SFLOAT;
        case R32G32B32_SFLOAT:    return VK_FORMAT_R32G32B32_SFLOAT;
        case R32G32B32A32_SFLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
        }

        FL_CONSTEXPR_ASSERT(false, "Unknown Format!");
        return VK_FORMAT_UNDEFINED;
    }

    constexpr std::string_view ShaderStageToString(ShaderStage stage)
    {
        using enum ShaderStage;

        switch (stage)
        {
        case Vertex:   return "Vertex";
        case Fragment: return "Fragment";
        case Compute:  return "Compute";
        default:       return "Combined/Unknown";
        }
    }

} // namespace Flux