#pragma once


#include "Flux/Core/Base.h"

namespace Flux {

    enum class ShaderStage : uint8_t
    {
        Vertex   = BIT(0),
        Fragment = BIT(1),
        Compute  = BIT(2),
    };

    constexpr ShaderStage operator | (ShaderStage a, ShaderStage b)
    {
        return static_cast<ShaderStage>(
            static_cast<uint8_t>(a) | static_cast<uint8_t>(b)
            );
    }

    constexpr ShaderStage operator & (ShaderStage a, ShaderStage b)
    {
        return static_cast<ShaderStage>(
            static_cast<uint8_t>(a) & static_cast<uint8_t>(b)
            );
    }

    constexpr ShaderStage operator ~ (ShaderStage a)
    {
        return static_cast<ShaderStage>(~static_cast<uint8_t>(a));
    }

    constexpr ShaderStage& operator |= (ShaderStage& a, ShaderStage b)
    {
        a = a | b;
        return a;
    }

    constexpr ShaderStage& operator &= (ShaderStage& a, ShaderStage b)
    {
        a = a & b;
        return a;
    }

    constexpr bool HasFlag(ShaderStage value, ShaderStage flag)
    {
        return (static_cast<uint8_t>(value) & static_cast<uint8_t>(flag)) != 0;
    }

    

    enum class Format : uint8_t
    {
        R8G8B8A8_UNORM      = 0,
        B8G8R8A8_UNORM      = 1,
        D32_SFLOAT          = 2,
        R32G32_SFLOAT       = 3,
        R32G32B32_SFLOAT    = 4,
        R32G32B32A32_SFLOAT = 5,
    };

    enum class ResourceState : uint8_t 
    {
        Undefined              = 0,
        RenderTarget           = 1,
        ShaderResource         = 2,
        UnorderedAccess        = 3,
        TransferSrc            = 4,
        TransferDst            = 5,
        Present                = 6,
        DepthStencilWrite      = 7,
        DepthStencilRead       = 8,
    };

}