#pragma once

#include "Flux/Renderer/RHIDevice.h"
#include "Flux/Renderer/RHICommandList.h"
#include "Flux/Renderer/RHITexture.h"
#include "Flux/Renderer/RHIPipeline.h"
#include "Flux/Renderer/RHIShader.h"
#include "Flux/Renderer/Renderer.h"
#include "Flux/Renderer/RenderGraph.h"
#include "Flux/Scene/Scene.h"

#include <imgui.h>

namespace Flux {

    class SceneRenderer
    {
    public:
        void Init(RHIDevice& device);
        void OnResize(uint32_t width, uint32_t height);

        void Render(RHICommandList* cmdList, Ref<Scene> scene, const PerspectiveCamera& camera);

        ImTextureID             GetViewportTextureID()          const { return m_ViewportTextureID; }
        RHIDescriptorSetLayout* GetTextureDescriptorSetLayout() const { return m_Renderer->GetTextureDescriptorSetLayout(); }

    private:
        void BuildGraph();
        void BuildPipeline();
        void RegisterImGuiTexture();
        void ExtractLightsFromScene(Ref<Scene> scene);

    private:
        RHIDevice* m_Device = nullptr;
        glm::vec2  m_ViewportSize = { 1280.0f, 720.0f };

        Scope<Renderer>    m_Renderer;
        Scope<RHISampler>  m_DefaultSampler;
        Scope<RHIShader>   m_VertShader;
        Scope<RHIShader>   m_FragShader;
        Scope<RHIPipeline> m_Pipeline;

        RenderGraph m_RenderGraph;

        RGTextureHandle m_ColorMsaaHandle;
        RGTextureHandle m_DepthMsaaHandle;
        RGTextureHandle m_ColorResolveHandle;

        Ref<Scene>             m_CurrentScene;
        const PerspectiveCamera* m_CurrentCamera = nullptr;

        ImTextureID m_ViewportTextureID = 0ull;

        static constexpr SampleCount MSAA_SAMPLES = SampleCount::x8;
    };

} // namespace Flux