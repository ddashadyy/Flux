#pragma once

#include "Flux/Scene/Scene.h"
#include "Flux/Scene/Entity.h"

namespace Flux {

    class ScenePanel
    {
    public:
        ScenePanel() = default;
        explicit ScenePanel(Ref<Scene> scene) : m_Scene(std::move(scene)) {}

        void SetScene(Ref<Scene> scene) { m_Scene = std::move(scene); m_SelectedEntity = Entity(); }

        void SetSelectedEntity(Entity entity) { m_SelectedEntity = entity; }
        Entity GetSelectedEntity() const { return m_SelectedEntity; }

        void OnImGuiRender();

    private:
        void DrawHierarchy();
        void DrawInspector();
        void DrawEntityNode(Entity entity);

        void ProcessHotkeys();
        void DeleteSelectedEntity();
        void DuplicateSelectedEntity();

    private:
        Ref<Scene> m_Scene;
        Entity     m_SelectedEntity;
    };

} // namespace Flux