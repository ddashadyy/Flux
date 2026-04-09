#pragma once


#include <cstdint>

namespace Flux {

    enum class ShaderStage : uint8_t
    {
        Vertex = 0,
        Fragment = 1,
        Compute = 2,
    };

    enum class Format : uint8_t
    {
        R8G8B8A8_UNORM = 0,
        B8G8R8A8_UNORM = 1,
        D32_SFLOAT = 2,
        R32G32_SFLOAT = 3,
        R32G32B32_SFLOAT = 4,
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