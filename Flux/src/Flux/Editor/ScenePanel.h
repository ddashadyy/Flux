#pragma once

#include "Flux/Scene/Scene.h"

namespace Flux {

    class ScenePanel
    {
    public:
        ScenePanel() = default;
        explicit ScenePanel(Ref<Scene> scene) : m_Scene(std::move(scene)) {}

        void SetScene(Ref<Scene> scene) { m_Scene = std::move(scene); m_SelectedIndex = -1; }

        void OnImGuiRender();

    private:
        void DrawHierarchy();
        void DrawInspector();

    private:
        Ref<Scene> m_Scene;
        int        m_SelectedIndex = -1;
    };

}