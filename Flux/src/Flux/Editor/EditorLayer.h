#pragma once

#include "Flux/Core/Layer.h"
#include "Flux/Core/Application.h"
#include "Flux/Events/KeyEvent.h"
#include "Flux/Events/MouseEvent.h"

#include "Flux/Scene/Scene.h"
#include "Flux/Scene/Entity.h"
#include "Flux/Editor/ScenePanel.h"

#include "Flux/Renderer/Renderer.h"
#include "Flux/Renderer/RHICommandList.h"
#include "Flux/Renderer/RHIPipeline.h"
#include "Flux/Renderer/RHIRenderPass.h"
#include "Flux/Renderer/RHIFramebuffer.h"
#include "Flux/Renderer/RHIShader.h"
#include "Flux/Renderer/RHITexture.h"
#include "Flux/Renderer/PerspectiveCamera.h"

namespace Flux {

    class EditorLayer : public Layer
    {
    public:
        EditorLayer();
        ~EditorLayer() = default;

        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate(RHICommandList* cmdList, uint32_t imageIndex) override;
        void OnImGuiRender() override;
        void OnEvent(Event& event) override;
        void OnResize(uint32_t width, uint32_t height) override;

    private:
        void RecreateSwapchainResources(uint32_t width, uint32_t height);
        void CreateDepthAndMsaa(uint32_t width, uint32_t height);
        void CreateRenderPass();
        void CreateFramebuffers(uint32_t width, uint32_t height);
        void CreatePipeline();
        void LoadScene();

        void ProcessKeyboard(float dt);
        bool OnMouseMoved(MouseMovedEvent& e);
        bool OnMouseScrolled(MouseScrolledEvent& e);
        bool OnKeyPressed(KeyPressedEvent& e);
        void SetCursorMode(bool locked);

        void DrawDockspace();
        void DrawStatsWindow();
        void DrawViewport();

    private:
        static constexpr SampleCount MSAA_SAMPLES = SampleCount::x8;

        // Сцена
        Ref<Scene>  m_Scene;
        ScenePanel  m_ScenePanel;

        // Рендерер
        Scope<Renderer> m_Renderer;

        // Камера и свет
        PerspectiveCamera m_Camera;

        // Shaders & pipeline
        Scope<RHIShader>   m_VertShader;
        Scope<RHIShader>   m_FragShader;
        Scope<RHIPipeline> m_Pipeline;

        // Render pass + attachments
        Scope<RHITexture>                  m_DepthTexture;
        Scope<RHITexture>                  m_MsaaColorTexture;
        Scope<RHIRenderPass>               m_RenderPass;
        std::vector<Scope<RHIFramebuffer>> m_Framebuffers;

        // Sampler для ImGui texture preview (если понадобится)
        Scope<RHISampler> m_DefaultSampler;

        // Camera controller state
        float m_CameraSpeed = 5.0f;
        float m_MouseSensitivity = 0.1f;
        float m_LastFrameTime = 0.0f;
        float m_LastX = 0.0f, m_LastY = 0.0f;
        bool  m_FirstMouse = true;
        bool  m_CursorLocked = true;
    };

} // namespace Flux
