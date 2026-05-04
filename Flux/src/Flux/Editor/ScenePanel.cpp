#include "flpch.h"
#include "ScenePanel.h"

#include <imgui.h>
#include <ImGuiFileDialog.h>
#include <glm/gtc/type_ptr.hpp>

namespace Flux {

    void ScenePanel::OnImGuiRender()
    {
        DrawHierarchy();
        DrawInspector();
    }

    void ScenePanel::DrawHierarchy()
    {
        ImGui::Begin("Hierarchy");

        if (!m_Scene)
        {
            ImGui::TextDisabled("No scene loaded");
            ImGui::End();
            return;
        }

        auto& entities = m_Scene->GetEntities();

        for (int i = 0; i < (int)entities.size(); i++)
        {
            const auto& entity = entities[i];

            // Fallback label если имя пустое
            std::string label = entity.GetName().empty()
                ? ("Entity " + std::to_string(i))
                : entity.GetName();

            bool selected = (m_SelectedIndex == i);
            if (ImGui::Selectable(label.c_str(), selected))
                m_SelectedIndex = i;
        }

        ImGui::End();
    }

    void ScenePanel::DrawInspector()
    {
        ImGui::Begin("Inspector");

        if (!m_Scene || m_SelectedIndex < 0 ||
            m_SelectedIndex >= (int)m_Scene->GetEntities().size())
        {
            ImGui::TextDisabled("Nothing selected");
            ImGui::End();
            return;
        }

        Entity& entity = m_Scene->GetEntities()[m_SelectedIndex];
        Transform& t = entity.GetTransform();

        // --- Имя ---
        {
            char buf[256];
            std::strncpy(buf, entity.GetName().c_str(), sizeof(buf));
            buf[sizeof(buf) - 1] = '\0';
            if (ImGui::InputText("Name", buf, sizeof(buf)))
                entity.SetName(buf);
        }

        ImGui::Separator();

        // --- Transform ---
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::DragFloat3("Position", glm::value_ptr(t.Position), 0.1f);
            ImGui::DragFloat3("Rotation", glm::value_ptr(t.Rotation), 1.0f);
            ImGui::DragFloat3("Scale", glm::value_ptr(t.Scale), 0.01f, 0.001f, 100.0f);
        }

        ImGui::End();
    }

}