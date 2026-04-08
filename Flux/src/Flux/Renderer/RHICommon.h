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

}