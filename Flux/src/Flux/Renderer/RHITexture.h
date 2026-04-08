#pragma once

#include "Flux/Core.h"
#include "RHICommon.h"

namespace Flux {

    enum class TextureType : uint8_t
    {
        Texture2D = 0,
        Texture3D = 1,
        CubeMap = 2
    };

    enum class TextureUsage : uint8_t
    {
        Sampled      = 0, // читается в шейдере
        RenderTarget = 1, // color attachment
        DepthStencil = 2, // depth attachment
        Storage      = 3, // read/write в compute
    };

    struct TextureSpec 
    {
        uint32_t     Width       = 1;
        uint32_t     Height      = 1;
        uint32_t     Depth       = 1;

        Format       ImageFormat = Format::R8G8B8A8_UNORM;
        uint32_t     MipLevels   = 1;

        TextureUsage Usage       = TextureUsage::Sampled;
        TextureType  Type        = TextureType::Texture2D;
    };

    class RHITexture
    {
    public:
        virtual ~RHITexture() = default;

		virtual uint32_t GetWidth()     const = 0;
		virtual uint32_t GetHeight()    const = 0;
		virtual Format   GetFormat()    const = 0;
        virtual uint32_t GetMipLevels() const = 0;

        virtual void SetData(const void* data, uint32_t size) = 0;
        
    };

}