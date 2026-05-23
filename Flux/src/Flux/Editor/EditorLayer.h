#pragma once

#include "Flux/Core/Layer.h"
#include "Flux/Core/Application.h"
#include "Flux/Events/KeyEvent.h"
#include "Flux/Events/MouseEvent.h"

#include "Flux/Scene/Scene.h"
#include "Flux/Scene/Entity.h"
#include "Flux/Scene/ScenePanel.h"
#include "Flux/Scene/SceneRenderer.h"
#include "Flux/Renderer/EditorCamera.h"
#include "Flux/Editor/ContentBrowser.h"

#include <imgui.h>
#include <ImGuizmo.h>

namespace Flux {

    class EditorLayer final : public Layer
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
        void LoadDefaultScene();
        void LoadModel(const std::string& path);

        void DrawDockspace();
        void DrawStatsWindow();
        void DrawViewport();
        void DrawMenuBar();

    private:
        Ref<Scene>           m_Scene;
        ScenePanel           m_ScenePanel;
        ContentBrowser       m_ContentBrowser;
        Scope<SceneRenderer> m_SceneRenderer;
        EditorCamera         m_Camera;

        glm::vec2 m_ViewportSize = { 1280.0f, 720.0f };
        glm::vec2 m_PendingViewportSize = { 0.0f, 0.0f };
        bool      m_ViewportFocused = false;
        bool      m_ViewportNeedsResize = false;

        ImGuizmo::OPERATION m_GizmoOperation = ImGuizmo::TRANSLATE;
        float               m_LastFrameTime = 0.0f;

        std::string m_CurrentScenePath;

        struct PendingSceneLoad 
        {
            bool        Active = false;
            std::string Path;
            bool        IsNew = false; // true = New Scene, false = Open
        };
        PendingSceneLoad m_PendingSceneLoad;
    };

} // namespace Flux