#include "flpch.h"
#include "RenderGraph.h"

namespace Flux {

    // =========================================================================
    //  RGExecuteContext
    // =========================================================================

    RHITexture* RGExecuteContext::GetTexture(RGTextureHandle handle) const
    {
        return Graph->GetPhysicalTexture(handle);
    }

    // =========================================================================
    //  RenderGraph
    // =========================================================================

    void RenderGraph::Reset()
    {
        m_Passes.clear();
        m_Textures.clear();
        m_OutputHandle = {};
        m_Compiled = false;
    }

    RGTextureHandle RenderGraph::CreateTexture(const RGTextureDesc& desc)
    {
        RGPhysicalTexture phys{};
        phys.Desc = desc;
        phys.External = false;

        m_Textures.push_back(std::move(phys));
        return { (uint32_t)m_Textures.size() - 1 };
    }

    RGTextureHandle RenderGraph::ImportTexture(const std::string& name, RHITexture* texture)
    {
        FL_CORE_ASSERT(texture, "ImportTexture: null pointer");

        RGPhysicalTexture phys{};
        phys.Desc.Name = name;
        phys.External = true;
        phys.ExternalPtr = texture;

        m_Textures.push_back(std::move(phys));
        return { (uint32_t)m_Textures.size() - 1 };
    }

    RGPassBuilder RenderGraph::AddPass(const std::string& name)
    {
        RGPass pass{};
        pass.Name = name;
        m_Passes.push_back(std::move(pass));
        return RGPassBuilder(m_Passes.back());
    }

    void RenderGraph::SetOutput(RGTextureHandle handle)
    {
        m_OutputHandle = handle;
    }

    RHITexture* RenderGraph::GetPhysicalTexture(RGTextureHandle handle) const
    {
        FL_CORE_ASSERT(handle.IsValid() && handle.Index < m_Textures.size(),
            "RenderGraph: invalid texture handle");
        return m_Textures[handle.Index].Get();
    }

    // =========================================================================
    //  Compile
    // =========================================================================

    void RenderGraph::Compile(RHIDevice& device)
    {
        for (auto& phys : m_Textures)
        {
            if (phys.External) continue;

            TextureSpec spec{};
            spec.Width = phys.Desc.Width;
            spec.Height = phys.Desc.Height;
            spec.MipLevels = phys.Desc.MipLevels;
            spec.ImageFormat = phys.Desc.ImageFormat;
            spec.Samples = phys.Desc.Samples;
            spec.DebugName = phys.Desc.Name.c_str();

            // Usage → TextureUsage
            // TODO: сделать None
            TextureUsage usage = TextureUsage::Sampled;

            if (HasFlag(phys.Desc.Usage, RGTextureUsage::ColorAttachment))
                usage = usage | TextureUsage::RenderTarget;
            if (HasFlag(phys.Desc.Usage, RGTextureUsage::DepthAttachment))
                usage = usage | TextureUsage::DepthStencil;
            if (HasFlag(phys.Desc.Usage, RGTextureUsage::Sampled))
                usage = usage | TextureUsage::Sampled;
            if (HasFlag(phys.Desc.Usage, RGTextureUsage::ResolveTarget))
                usage = usage | TextureUsage::RenderTarget | TextureUsage::Sampled;

            spec.Usage = usage;
            phys.Texture = device.CreateTexture(spec);
        }

        for (size_t i = 0; i < m_Passes.size(); ++i)
        {
            BuildRenderPass(device, m_Passes[i]);
            BuildFramebuffer(device, m_Passes[i]);
        }

        m_Compiled = true;
    }

    void RenderGraph::BuildRenderPass(RHIDevice& device, RGPass& pass)
    {
        RenderPassDesc desc{};

        // Color attachments
        for (auto handle : pass.ColorWrites)
        {
            auto& phys = m_Textures[handle.Index];
            desc.ColorFormats.push_back(phys.Desc.ImageFormat);
            desc.Samples = phys.Desc.Samples;
        }

        // Depth
        if (pass.HasDepth())
        {
            auto& depthPhys = m_Textures[pass.DepthWrite.Index];
            desc.HasDepth = true;
            desc.DepthFormat = depthPhys.Desc.ImageFormat;
        }

        // Resolve
        if (pass.HasResolve())
            desc.HasResolve = true;

        // Load / Store ops
        desc.ColorLoadOp = AttachmentLoadOp::Clear;
        desc.ColorStoreOp = pass.HasResolve()
            ? AttachmentStoreOp::DontCare
            : AttachmentStoreOp::Store;

        desc.DepthLoadOp = AttachmentLoadOp::Clear;
        desc.DepthStoreOp = AttachmentStoreOp::DontCare;

        // Layouts — color
        desc.ColorInitialLayout = ImageLayout::Undefined;
        desc.ColorFinalLayout = pass.HasResolve()
            ? ImageLayout::ColorAttachment
            : ImageLayout::ShaderReadOnly;

        // Layouts — resolve
        if (pass.HasResolve())
        {
            desc.ResolveInitialLayout = ImageLayout::Undefined;
            desc.ResolveFinalLayout = ImageLayout::ShaderReadOnly;
        }

        // Layouts — depth
        desc.DepthInitialLayout = ImageLayout::Undefined;
        desc.DepthFinalLayout = ImageLayout::DepthStencilAttachment;

        pass.CompiledRenderPass = device.CreateRenderPass(desc);
    }

    void RenderGraph::BuildFramebuffer(RHIDevice& device, RGPass& pass)
    {
        FL_CORE_ASSERT(!pass.ColorWrites.empty() || pass.HasDepth(),
            "RGPass '{}' has no attachments", pass.Name);

        uint32_t width = 0;
        uint32_t height = 0;

        if (!pass.ColorWrites.empty())
        {
            auto& phys = m_Textures[pass.ColorWrites[0].Index];
            width = phys.Desc.Width;
            height = phys.Desc.Height;
        }
        else if (pass.HasDepth())
        {
            auto& phys = m_Textures[pass.DepthWrite.Index];
            width = phys.Desc.Width;
            height = phys.Desc.Height;
        }

        FramebufferSpec fbSpec{};
        fbSpec.RenderPass = pass.CompiledRenderPass.get();
        fbSpec.Width = width;
        fbSpec.Height = height;

        for (auto handle : pass.ColorWrites)
            fbSpec.ColorTargets.push_back(m_Textures[handle.Index].Get());

        if (pass.HasDepth())
            fbSpec.DepthTarget = m_Textures[pass.DepthWrite.Index].Get();

        if (pass.HasResolve())
            fbSpec.ResolveTarget = m_Textures[pass.Resolve.Dst.Index].Get();

        pass.CompiledFramebuffer = device.CreateFramebuffer(fbSpec);
    }

    // =========================================================================
    //  Execute
    // =========================================================================

    void RenderGraph::Execute(RHICommandList* cmdList)
    {
        FL_CORE_ASSERT(m_Compiled, "RenderGraph::Execute called before Compile()");

        for (auto& pass : m_Passes)
        {
            if (!pass.ExecuteFn) continue;

            RGExecuteContext ctx{};
            ctx.CmdList = cmdList;
            ctx.RenderPass = pass.CompiledRenderPass.get();
            ctx.Framebuffer = pass.CompiledFramebuffer.get();
            ctx.Graph = this;

            pass.ExecuteFn(ctx);
        }
    }

    RHIRenderPass* RenderGraph::GetPassRenderPass(const std::string& passName) const
    {
        for (auto& pass : m_Passes)
            if (pass.Name == passName)
                return pass.CompiledRenderPass.get();
        return nullptr;
    }

    // =========================================================================
    //  Resize — пересоздаёт transient ресурсы под новый размер
    // =========================================================================

    void RenderGraph::Resize(RHIDevice& device, uint32_t width, uint32_t height)
    {
        for (auto& phys : m_Textures)
        {
            if (phys.External) continue;
            phys.Desc.Width = width;
            phys.Desc.Height = height;
            phys.Texture.reset();
        }

        for (auto& pass : m_Passes)
        {
            pass.CompiledRenderPass.reset();
            pass.CompiledFramebuffer.reset();
        }

        m_Compiled = false;
        Compile(device);
    }

} // namespace Flux