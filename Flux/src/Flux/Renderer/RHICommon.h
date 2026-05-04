#pragma once

#include "Flux/Core/Base.h"

namespace Flux {

    // -------------------------------------------------------------------------
    // Sample count
    // -------------------------------------------------------------------------

    enum class SampleCount : uint8_t
    {
        x1  = 1,
        x2  = 2,
        x4  = 4,
        x8  = 8,
    };

    // -------------------------------------------------------------------------
    // Shader stages
    // -------------------------------------------------------------------------

    enum class ShaderStage : uint8_t
    {
        Vertex   = BIT(0),
        Fragment = BIT(1),
        Compute  = BIT(2),
    };
    FL_ENABLE_BITWISE_OPERATORS(ShaderStage);

    // -------------------------------------------------------------------------
    // Pixel / vertex formats
    // -------------------------------------------------------------------------

    enum class Format : uint8_t
    {
        Undefined           = 0,

        // Color
        R8_UNORM            = 1,
        R8G8B8A8_UNORM      = 2,
        R8G8B8A8_SRGB       = 3,
        B8G8R8A8_UNORM      = 4,
        B8G8R8A8_SRGB       = 5,
        R16G16B16A16_SFLOAT = 6,
        R32G32_SFLOAT       = 7,
        R32G32B32_SFLOAT    = 8,
        R32G32B32A32_SFLOAT = 9,

        // Depth
        D16_UNORM           = 10,
        D32_SFLOAT          = 11,
        D24_UNORM_S8_UINT   = 12,
    };

    // -------------------------------------------------------------------------
    // Resource / image states
    // -------------------------------------------------------------------------

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
        IndirectArgument       = 9,  // для indirect draw / compute
    };

    // -------------------------------------------------------------------------
    // Index type
    // -------------------------------------------------------------------------

    enum class IndexType : uint8_t
    {
        None   = 0,
        Uint16 = 1,
        Uint32 = 2,
    };

    // -------------------------------------------------------------------------
    // Rasterizer
    // -------------------------------------------------------------------------

    enum class CullMode : uint8_t
    {
        None  = 0,
        Front = 1,
        Back  = 2,
    };

    enum class FillMode : uint8_t
    {
        Solid     = 0,
        Wireframe = 1,
    };

    enum class FrontFace : uint8_t
    {
        CounterClockwise = 0,
        Clockwise        = 1,
    };

    // -------------------------------------------------------------------------
    // Depth / stencil compare
    // -------------------------------------------------------------------------

    enum class CompareOp : uint8_t
    {
        Never          = 0,
        Less           = 1,
        Equal          = 2,
        LessOrEqual    = 3,
        Greater        = 4,
        NotEqual       = 5,
        GreaterOrEqual = 6,
        Always         = 7,
    };

    // -------------------------------------------------------------------------
    // Blend
    // -------------------------------------------------------------------------

    enum class BlendFactor : uint16_t
    {
        Zero             = 0,
        One              = 1,
        SrcColor         = 2,
        OneMinusSrcColor = 3,
        DstColor         = 4,
        OneMinusDstColor = 5,
        SrcAlpha         = 6,
        OneMinusSrcAlpha = 7,
        DstAlpha         = 8,
        OneMinusDstAlpha = 9,
    };

    enum class BlendOp : uint8_t
    {
        Add             = 0,
        Subtract        = 1,
        ReverseSubtract = 2,
        Min             = 3,
        Max             = 4,
    };

    // -------------------------------------------------------------------------
    // Sampler
    // -------------------------------------------------------------------------

    enum class FilterMode : uint8_t
    {
        Nearest = 0,
        Linear  = 1,
    };

    enum class MipMapMode : uint8_t
    {
        Nearest = 0,
        Linear  = 1,
    };

    enum class AddressMode : uint8_t
    {
        Repeat            = 0,
        MirroredRepeat    = 1,
        ClampToEdge       = 2,
        ClampToBorder     = 3,
    };

    enum class BorderColor : uint8_t
    {
        TransparentBlack = 0,
        OpaqueBlack      = 1,
        OpaqueWhite      = 2,
    };

} // namespace Flux
