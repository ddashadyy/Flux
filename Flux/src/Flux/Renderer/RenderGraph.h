#pragma once

#include "Flux/Core/Base.h"
#include "Flux/Renderer/RHIDevice.h"
#include "Flux/Renderer/RHITexture.h"
#include "Flux/Renderer/RHIRenderPass.h"
#include "Flux/Renderer/RHIFramebuffer.h"
#include "Flux/Renderer/RHICommandList.h"

#include <functional>
#include <string>
#include <vector>

namespace Flux {

    // =========================================================================
    //  Хендлы ресурсов
    // =========================================================================

    struct RGTextureHandle
    {
        uint32_t Index = UINT32_MAX;
        bool IsValid() const { return Index != UINT32_MAX; }
    };

    inline bool operator==(RGTextureHandle a, RGTextureHandle b) { return a.Index == b.Index; }
    inline bool operator!=(RGTextureHandle a, RGTextureHandle b) { return a.Index != b.Index; }

    // =========================================================================
    //  Описание текстуры внутри графа
    // =========================================================================

    enum class RGTextureUsage : uint32_t
    {
        None = 0,
        ColorAttachment = 1 << 0,
        DepthAttachment = 1 << 1,
        Sampled = 1 << 2,   // (post-process, shadow sampler)
        ResolveTarget = 1 << 3,   
    };

    inline RGTextureUsage operator|(RGTextureUsage a, RGTextureUsage b)
    {
        return (RGTextureUsage)((uint32_t)a | (uint32_t)b);
    }

    inline bool HasFlag(RGTextureUsage val, RGTextureUsage flag)
    {
        return ((uint32_t)val & (uint32_t)flag) != 0;
    }

    struct RGTextureDesc
    {
        std::string   Name;
        Format        ImageFormat = Format::R8G8B8A8_UNORM;
        uint32_t      Width = 1;
        uint32_t      Height = 1;
        uint32_t      MipLevels = 1;
        SampleCount   Samples = SampleCount::x1;
        RGTextureUsage Usage = RGTextureUsage::ColorAttachment;
        bool          External = false; // true = граф не создаёт, берёт снаружи
    };

    // =========================================================================
    //  Физический ресурс 
    // =========================================================================

    struct RGPhysicalTexture
    {
        Scope<RHITexture> Texture;
        RGTextureDesc     Desc;
        bool              External = false;
        RHITexture* ExternalPtr = nullptr; // если External == true

        RHITexture* Get() const
        {
            return External ? ExternalPtr : Texture.get();
        }
    };

    // =========================================================================
    //  Контекст выполнения пасса 
    // =========================================================================

    class RenderGraph;

    struct RGExecuteContext
    {
        RHICommandList* CmdList = nullptr;
        RHIRenderPass* RenderPass = nullptr;
        RHIFramebuffer* Framebuffer = nullptr;
        RenderGraph* Graph = nullptr;

        RHITexture* GetTexture(RGTextureHandle handle) const;
    };

    // =========================================================================
    //  Пасс
    // =========================================================================

    using RGExecuteCallback = std::function<void(RGExecuteContext&)>;

    struct RGResolveInfo
    {
        RGTextureHandle Src;  // MSAA source
        RGTextureHandle Dst;  // resolve target
    };

    struct RGPass
    {
        std::string Name;

        std::vector<RGTextureHandle> ColorWrites;  // color attachments
        RGTextureHandle              DepthWrite;   // depth attachment
        std::vector<RGTextureHandle> Reads;        // shader reads (для барьеров)
        RGResolveInfo                Resolve;      // MSAA resolve (опционально)

        // Параметры RenderPass
        Scope<RHIRenderPass>   CompiledRenderPass;
        Scope<RHIFramebuffer>  CompiledFramebuffer;

        // Callback с командами
        RGExecuteCallback ExecuteFn;

        bool HasResolve()   const { return Resolve.Src.IsValid() && Resolve.Dst.IsValid(); }
        bool HasDepth()     const { return DepthWrite.IsValid(); }
    };

    // =========================================================================
    //  Билдер пасса — fluent API
    // =========================================================================

    class RGPassBuilder
    {
    public:
        RGPassBuilder(RGPass& pass) : m_Pass(pass) {}

        // Пишем в color attachment
        RGPassBuilder& Write(RGTextureHandle handle)
        {
            m_Pass.ColorWrites.push_back(handle);
            return *this;
        }

        // Пишем в depth
        RGPassBuilder& WriteDepth(RGTextureHandle handle)
        {
            m_Pass.DepthWrite = handle;
            return *this;
        }

        // MSAA resolve
        RGPassBuilder& Resolve(RGTextureHandle src, RGTextureHandle dst)
        {
            m_Pass.Resolve = { src, dst };
            return *this;
        }

        // Читаем как текстуру в шейдере (post-process)
        RGPassBuilder& Read(RGTextureHandle handle)
        {
            m_Pass.Reads.push_back(handle);
            return *this;
        }

        // Колбэк с командами рендера
        void Execute(RGExecuteCallback fn)
        {
            m_Pass.ExecuteFn = std::move(fn);
        }

    private:
        RGPass& m_Pass;
    };

    // =========================================================================
    //  RenderGraph
    // =========================================================================

    class RenderGraph
    {
    public:
        RenderGraph() = default;

        void Reset();

        // --- Ресурсы ---
        RGTextureHandle CreateTexture(const RGTextureDesc& desc);
        RGTextureHandle ImportTexture(const std::string& name, RHITexture* texture);

        // --- Пассы ---
        RGPassBuilder   AddPass(const std::string& name);
        void            SetOutput(RGTextureHandle handle);
        RGTextureHandle GetOutput() const { return m_OutputHandle; }

        // --- Компиляция и выполнение ---
        void           Compile(RHIDevice& device);
        void           Execute(RHICommandList* cmdList);
        RHITexture*    GetPhysicalTexture(RGTextureHandle handle) const;
        void           Resize(RHIDevice& device, uint32_t width, uint32_t height);
        RHIRenderPass* GetPassRenderPass(const std::string& passName) const;

        bool IsCompiled() const { return m_Compiled; }

    private:
        void BuildRenderPass(RHIDevice& device, RGPass& pass);
        void BuildFramebuffer(RHIDevice& device, RGPass& pass);

        ImageLayout GetInitialLayout(RGTextureHandle handle, size_t passIndex) const;
        ImageLayout GetFinalLayout(RGTextureHandle handle, size_t passIndex) const;

    private:
        std::vector<RGPhysicalTexture> m_Textures;
        std::vector<RGPass>            m_Passes;

        RGTextureHandle m_OutputHandle;
        bool            m_Compiled = false;
    };

} // namespace Flux