#pragma once

#include "Flux/Core/Base.h"
#include "RHICommon.h"

namespace Flux {

    // -------------------------------------------------------------------------
    // Texture
    // -------------------------------------------------------------------------

    enum class TextureType : uint8_t
    {
        Texture2D      = 0,
        Texture2DArray = 1,
        Texture3D      = 2,
        CubeMap        = 3,
    };

    // Битмаска — текстура может быть RenderTarget | Sampled (G-buffer)
    enum class TextureUsage : uint8_t
    {
        Sampled      = BIT(0),
        RenderTarget = BIT(1),
        DepthStencil = BIT(2),
        Storage      = BIT(3),  // read/write в compute shader
        TransferSrc  = BIT(4),  // источник копирования
        TransferDst  = BIT(5),  // назначение копирования
    };
    FL_ENABLE_BITWISE_OPERATORS(TextureUsage);

    struct TextureSpec
    {
        uint32_t     Width       = 1;
        uint32_t     Height      = 1;
        uint32_t     Depth       = 1;
        uint32_t     ArrayLayers = 1;

        Format       ImageFormat = Format::R8G8B8A8_UNORM;
        uint32_t     MipLevels   = 1;

        SampleCount  Samples     = SampleCount::x1;

        TextureUsage Usage       = TextureUsage::Sampled;
        TextureType  Type        = TextureType::Texture2D;

        ResourceState InitialState = ResourceState::Undefined; // начальный layout

        const char*  DebugName   = nullptr;
    };

    class RHITexture
    {
    public:
        virtual ~RHITexture() = default;

        virtual void* GetHandle() const = 0;

        virtual const TextureSpec& GetSpec()     const = 0;
        uint32_t GetWidth()     const { return GetSpec().Width; }
        uint32_t GetHeight()    const { return GetSpec().Height; }
        Format   GetFormat()    const { return GetSpec().ImageFormat; }
        uint32_t GetMipLevels() const { return GetSpec().MipLevels; }

        virtual void* GetNativeImageView() const = 0;

        // Загрузить данные с CPU (только для CPU-visible / staging текстур)
        virtual void SetData(const void* data, uint32_t size) = 0;
    };

    // -------------------------------------------------------------------------
    // Sampler 
    // -------------------------------------------------------------------------

    struct SamplerSpec
    {
        FilterMode  MagFilter    = FilterMode::Linear;
        FilterMode  MinFilter    = FilterMode::Linear;
        MipMapMode  MipMode      = MipMapMode::Linear;

        AddressMode AddressU     = AddressMode::Repeat;
        AddressMode AddressV     = AddressMode::Repeat;
        AddressMode AddressW     = AddressMode::Repeat;

        float       MipLodBias   = 0.0f;
        float       MinLod       = 0.0f;
        float       MaxLod       = 1000.0f; // VK_LOD_CLAMP_NONE аналог

        bool        AnisotropyEnable = false;
        float       MaxAnisotropy    = 1.0f;

        bool        CompareEnable = false;
        CompareOp   Compare       = CompareOp::Always;

        BorderColor Border        = BorderColor::OpaqueBlack;

        const char* DebugName     = nullptr;
    };

    class RHISampler
    {
    public:
        virtual ~RHISampler() = default;

        virtual void* GetHandle() const = 0;

        virtual const SamplerSpec& GetSpec() const = 0;
    };

} // namespace Flux
