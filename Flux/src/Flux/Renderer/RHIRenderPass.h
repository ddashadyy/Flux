#pragma once

#include "RHICommon.h"

#include <vector>

namespace Flux {

    enum class ImageLayout : uint8_t
    {
        Undefined              = 0,
        ColorAttachment        = 1,
        DepthStencilAttachment = 2,
        DepthStencilReadOnly   = 3, // для shadow map sampling
        ShaderReadOnly         = 4,
        TransferSrc            = 5,
        TransferDst            = 6,
        Present                = 7,
    };

    enum class AttachmentLoadOp : uint8_t
    {
        Load     = 0,
        Clear    = 1,
        DontCare = 2,
    };

    enum class AttachmentStoreOp : uint8_t
    {
        Store    = 0,
        DontCare = 1,
    };

    struct RenderPassDesc
    {
        std::vector<Format> ColorFormats = {};

        // Depth
        Format            DepthFormat    = Format::D32_SFLOAT;
        bool              HasDepth       = false;
        bool              DepthReadOnly  = false; // shadow map — читаем, не пишем

        SampleCount       Samples        = SampleCount::x1;

        // Color attachment ops
        AttachmentLoadOp  ColorLoadOp    = AttachmentLoadOp::Clear;
        AttachmentStoreOp ColorStoreOp   = AttachmentStoreOp::Store;

        // Depth attachment ops
        AttachmentLoadOp  DepthLoadOp    = AttachmentLoadOp::Clear;
        AttachmentStoreOp DepthStoreOp   = AttachmentStoreOp::DontCare;

        // Layouts
        ImageLayout ColorInitialLayout   = ImageLayout::Undefined;
        ImageLayout ColorFinalLayout     = ImageLayout::Present;  // для swapchain рендера
        ImageLayout DepthInitialLayout   = ImageLayout::Undefined;
        ImageLayout DepthFinalLayout     = ImageLayout::DepthStencilAttachment;

        // MSAA resolve — нужен если Samples > x1
        bool        HasResolve           = false;
        ImageLayout ResolveInitialLayout = ImageLayout::Undefined;
        ImageLayout ResolveFinalLayout   = ImageLayout::Present;
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

} // namespace Flux
