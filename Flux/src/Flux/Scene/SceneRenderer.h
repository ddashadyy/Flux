#pragma once

#include "Flux/Renderer/RHIDevice.h"
#include "Flux/Renderer/RHICommandList.h"
#include "Flux/Renderer/RHIFramebuffer.h"
#include "Flux/Renderer/RHIRenderPass.h"
#include "Flux/Renderer/RHITexture.h"
#include "Flux/Renderer/RHIPipeline.h"
#include "Flux/Renderer/Renderer.h"
#include "Flux/Renderer/EditorCamera.h"
#include "Flux/Scene/Scene.h"

#include <imgui.h>

namespace Flux {

    class SceneRenderer
    {
    public:
        void Init(RHIDevice& device);
        void OnResize(uint32_t width, uint32_t height);

        void Render(RHICommandList* cmdList, Ref<Scene> scene, const PerspectiveCamera& camera);

        ImTextureID GetViewportTextureID() const { return m_ViewportTextureID; }

        RHIDescriptorSetLayout* GetTextureDescriptorSetLayout() const { return m_Renderer->GetTextureDescriptorSetLayout(); }
        void AddPointLight(const PointLight& light) { m_Renderer->AddPointLight(light); }

    private:
        void CreateOffscreenResources(uint32_t width, uint32_t height);
        void CreatePipeline();
        void ExtractLightsFromScene(Ref<Scene> scene); 

    private:
        RHIDevice* m_Device = nullptr;
        glm::vec2  m_ViewportSize = { 1280.0f, 720.0f };

        Scope<Renderer>       m_Renderer;

        Scope<RHITexture>     m_ColorMsaa;
        Scope<RHITexture>     m_DepthMsaa;
        Scope<RHITexture>     m_ColorResolve;

        Scope<RHIRenderPass>  m_GeometryPass;
        Scope<RHIFramebuffer> m_GeometryFramebuffer;
        Scope<RHISampler>     m_DefaultSampler;

        Scope<RHIShader> m_VertShader;
        Scope<RHIShader> m_FragShader;

        Scope<RHIPipeline>    m_Pipeline;


        ImTextureID m_ViewportTextureID = 0;

        static constexpr SampleCount MSAA_SAMPLES = SampleCount::x8;

        // TODO: Пост-процессинг (Bloom, Tone Mapping)
    };
}