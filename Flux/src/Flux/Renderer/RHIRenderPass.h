#pragma once


#include <vector>

namespace Flux {

    enum class Format : uint8_t 
    { 
        R8B8G8A8_UNORM      = 0,
        B8G8R8A8_UNORM      = 1,
        D32_SFLOAT          = 2,
        R32G32_SFLOAT       = 2,
        R32G32B32_SFLOAT    = 2,
        R32G32B32A32_SFLOAT = 2,
    };

    enum class AttachmentLoadOp : uint8_t
    { 
        Load     = 0, 
        Clear    = 1, 
        DontCare = 2
    };

    enum class AttachmentStoreOp : uint8_t
    { 
        Store    = 0, 
        DontCare = 1
    };

    struct RenderPassDesc
    {
        std::vector<Format> ColorFormats = {};
        Format DepthFormat               = Format::R8B8G8A8_UNORM;
        AttachmentLoadOp LoadOp          = AttachmentLoadOp::Load;
        AttachmentStoreOp StoreOp        = AttachmentStoreOp::Store;
        bool HasDepth                    = false;
    };

    class RHIRenderPass 
    {
    public:
        virtual ~RHIRenderPass() = default;

        virtual uint32_t GetColorAttachmentCount() const = 0;
        virtual bool     HasDepthAttachment()      const = 0;
    };
}