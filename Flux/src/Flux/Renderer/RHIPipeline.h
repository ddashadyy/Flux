#pragma once

#include "Flux/Core/Base.h"

#include "RHIShader.h"
#include "RHICommon.h"
#include "RHIDescriptorSet.h"
#include "RHIRenderPass.h"
#include "BufferLayout.h"

namespace Flux {

    // -------------------------------------------------------------------------
    // Primitive topology
    // -------------------------------------------------------------------------

    enum class PrimitiveTopology : uint8_t
    {
        TriangleList  = 0,
        TriangleStrip = 1,
        LineList      = 2,
        LineStrip     = 3,
        PointList     = 4,
    };

    // -------------------------------------------------------------------------
    // Pipeline type
    // -------------------------------------------------------------------------

    enum class PipelineType : uint8_t
    {
        Graphics = 0,
        Compute  = 1,
    };

    // -------------------------------------------------------------------------
    // Rasterizer state
    // -------------------------------------------------------------------------

    struct RasterizerState
    {
        CullMode  Cull       = CullMode::Back;
        FillMode  Fill       = FillMode::Solid;
        FrontFace Front      = FrontFace::CounterClockwise;

        bool      DepthClamp = false;       // нужно для shadow map (не обрезать тени за far plane)
        float     DepthBiasConstant = 0.0f; // polygon offset — тоже для shadow map
        float     DepthBiasSlope    = 0.0f;
    };

    // -------------------------------------------------------------------------
    // Blend state (расширен)
    // -------------------------------------------------------------------------

    struct RenderTargetBlendState
    {
        bool        Enable          = false;

        BlendFactor SrcColorFactor  = BlendFactor::SrcAlpha;
        BlendFactor DstColorFactor  = BlendFactor::OneMinusSrcAlpha;
        BlendOp     ColorOp         = BlendOp::Add;

        BlendFactor SrcAlphaFactor  = BlendFactor::One;
        BlendFactor DstAlphaFactor  = BlendFactor::Zero;
        BlendOp     AlphaOp         = BlendOp::Add;
    };

    // Предустановки для удобства
    namespace BlendPreset {
        inline RenderTargetBlendState Opaque()      { return {}; }
        inline RenderTargetBlendState AlphaBlend()  { RenderTargetBlendState s; s.Enable = true; return s; }
        inline RenderTargetBlendState Additive()    
        {
            RenderTargetBlendState s;
            s.Enable         = true;
            s.SrcColorFactor = BlendFactor::One;
            s.DstColorFactor = BlendFactor::One;
            s.SrcAlphaFactor = BlendFactor::One;
            s.DstAlphaFactor = BlendFactor::One;
            return s;
        }
    }

    // -------------------------------------------------------------------------
    // Depth / stencil state 
    // -------------------------------------------------------------------------

    struct DepthStencilState
    {
        bool      DepthTest      = true;
        bool      DepthWrite     = true;
        CompareOp DepthCompare   = CompareOp::Less;

        // Stencil — пока базово
        bool      StencilTest      = false;
        uint8_t   StencilReadMask  = 0xFF;
        uint8_t   StencilWriteMask = 0xFF;
    };

    // -------------------------------------------------------------------------
    // Push constants layout
    // -------------------------------------------------------------------------

    struct PushConstantRange
    {
        ShaderStage Stage  = ShaderStage::Vertex | ShaderStage::Fragment;
        uint32_t    Offset = 0;
        uint32_t    Size   = 0;
    };

    // -------------------------------------------------------------------------
    // Pipeline descriptor
    // -------------------------------------------------------------------------

    struct PipelineDesc
    {
        // Shaders
        const RHIShader* VertexShader   = nullptr;
        const RHIShader* FragmentShader = nullptr;
        const RHIShader* ComputeShader  = nullptr; // для PipelineType::Compute

        const RHIRenderPass* RenderPass = nullptr;  // nullptr для compute

        // Vertex input
        BufferLayout VertexLayout;

        // States
        RasterizerState          Rasterizer;
        RenderTargetBlendState   Blend;             // один на все RT (расширить до массива если нужно)
        DepthStencilState        DepthStencil;
        PrimitiveTopology        Topology = PrimitiveTopology::TriangleList;
        SampleCount              Samples  = SampleCount::x1;

        // Layout
        PushConstantRange        PushConstants;
        std::vector<const RHIDescriptorSetLayout*> DescriptorSetLayouts;

        PipelineType Type      = PipelineType::Graphics;
        bool         DepthOnly = false; // для shadow pass — отключает color write
        
        const char*  DebugName = nullptr;
    };

    // -------------------------------------------------------------------------
    // Pipeline
    // -------------------------------------------------------------------------

    class RHIPipeline
    {
    public:
        virtual ~RHIPipeline() = default;

        virtual void* GetHandle() const = 0;
        virtual void* GetLayout() const = 0;

        virtual PipelineType GetType()  const = 0;
        virtual bool         IsValid()  const = 0;

        virtual const PipelineDesc& GetDesc() const = 0;  // вместо GetLayoutDesc — отдаём весь desc
    };

} // namespace Flux
