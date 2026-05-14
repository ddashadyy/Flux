#pragma once

#include "Flux/Scene/Scene.h"
#include <functional>

namespace Flux {

    class ScenePanel
    {
    public:
        using ImportModelCallback = std::function<void(const std::string& path)>;

        ScenePanel() = default;
        explicit ScenePanel(Ref<Scene> scene) : m_Scene(std::move(scene)) {}

        void SetScene(Ref<Scene> scene) { m_Scene = std::move(scene); m_SelectedIndex = -1; }
        void SetImportModelCallback(ImportModelCallback cb) { m_ImportCallback = std::move(cb); }
        void SetSelectedIndex(int index) { m_SelectedIndex = index; }

        int GetSelectedIndex() const { return m_SelectedIndex; }

        void OnImGuiRender();

    private:
        void DrawHierarchy();
        void DrawInspector();

        void ProcessHotkeys();       
        void DeleteSelectedEntity(); 
        void DuplicateSelectedEntity(); 

    private:
        Ref<Scene>            m_Scene;
        int                   m_SelectedIndex = -1;
        ImportModelCallback   m_ImportCallback;
    };
}