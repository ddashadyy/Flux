#pragma once


namespace Flux {

    enum class RendererBackend : uint8_t
    {
        Auto   = 0,
        Vulkan = 1,
        D3D12  = 2,
        Metal  = 3,
    };

}