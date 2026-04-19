#pragma once


#include "RHICommon.h"
#include <vector>


namespace Flux {

    enum class ImageLayout : uint8_t
    {
        Undefined              = 0,
        ColorAttachment        = 1,
        DepthStencilAttachment = 2,
        ShaderReadOnly         = 3,
        Present                = 4,
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
        Format              DepthFormat = Format::D32_SFLOAT;

        bool                HasDepth = false;
        bool                DepthReadOnly = false; // для shadow map

        SampleCount         Samples = SampleCount::x1;

        AttachmentLoadOp  ColorLoadOp = AttachmentLoadOp::Clear;
        AttachmentStoreOp ColorStoreOp = AttachmentStoreOp::Store;
        AttachmentLoadOp  DepthLoadOp = AttachmentLoadOp::Clear;
        AttachmentStoreOp DepthStoreOp = AttachmentStoreOp::DontCare;

        ImageLayout ColorInitialLayout = ImageLayout::Undefined; 
        ImageLayout ColorFinalLayout = ImageLayout::Present;

    };

    class RHIRenderPass 
    {
    public:
        virtual ~RHIRenderPass() = default;

        template<typename T>
        T GetHandle() const { return reinterpret_cast<T>(GetHandleImpl()); }

        virtual uint32_t GetColorAttachmentCount() const = 0;
        virtual bool     HasDepthAttachment()      const = 0;

        virtual const RenderPassDesc& GetDesc() const = 0;

    protected:
        virtual void* GetHandleImpl() const = 0;
    };
}