#include "flpch.h"
#include "ScenePanel.h"

#include <imgui.h>
#include <ImGuiFileDialog.h>
#include <glm/gtc/type_ptr.hpp>

namespace Flux {

    void ScenePanel::OnImGuiRender()
    {
        ProcessHotkeys(); 
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

        if (ImGui::Button("+ Import Model"))
        {
            IGFD::FileDialogConfig cfg;
            cfg.path = ".";
            ImGuiFileDialog::Instance()->OpenDialog(
                "ImportModelDlg", "Choose Model", ".obj,.gltf,.glb", cfg);
        }

        if (ImGuiFileDialog::Instance()->Display("ImportModelDlg",
            ImGuiWindowFlags_NoCollapse, ImVec2(600, 400)))
        {
            if (ImGuiFileDialog::Instance()->IsOk() && m_ImportCallback)
                m_ImportCallback(ImGuiFileDialog::Instance()->GetFilePathName());
            ImGuiFileDialog::Instance()->Close();
        }

        ImGui::Separator();

        auto& entities = m_Scene->GetEntities();
        for (int i = 0; i < (int)entities.size(); i++)
        {
            const auto& entity = entities[i];
            std::string label = entity.GetName().empty()
                ? ("Entity " + std::to_string(i))
                : entity.GetName();

            bool selected = (m_SelectedIndex == i);
            if (ImGui::Selectable(label.c_str(), selected))
                m_SelectedIndex = (m_SelectedIndex == i) ? -1 : i;

            if (ImGui::BeginPopupContextItem(label.c_str()))
            {
                if (ImGui::MenuItem("Duplicate")) DuplicateSelectedEntity();
                if (ImGui::MenuItem("Delete"))    DeleteSelectedEntity();
                ImGui::EndPopup();
            }
        }

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
            m_SelectedIndex = -1;

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
            if (ImGui::InputText("##Name", buf, sizeof(buf)))
                entity.SetName(buf);
        }

        ImGui::Separator();

        // --- Transform ---
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::DragFloat3("Position", glm::value_ptr(t.Position), 0.05f);
            ImGui::DragFloat3("Rotation", glm::value_ptr(t.Rotation), 0.5f);
            ImGui::DragFloat3("Scale", glm::value_ptr(t.Scale), 0.005f, 0.001f, 100.0f);
        }

        // --- Material ---
        auto model = entity.GetModel();
        if (model)
        {
            if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
            {
                Material& first = model->Meshes[0].Mat;

                if (ImGui::ColorEdit4("Color##Material", glm::value_ptr(first.Color)))
                    for (auto& mesh : model->Meshes) mesh.Mat.Color = first.Color;

                bool useRoughness = (first.RoughnessOverride >= 0.0f);
                if (ImGui::Checkbox("Override Roughness", &useRoughness))
                    for (auto& mesh : model->Meshes)
                        mesh.Mat.RoughnessOverride = useRoughness ? 0.5f : -1.0f;
                if (useRoughness)
                {
                    ImGui::SameLine();
                    if (ImGui::SliderFloat("##Roughness", &first.RoughnessOverride, 0.0f, 1.0f))
                        for (auto& mesh : model->Meshes)
                            mesh.Mat.RoughnessOverride = first.RoughnessOverride;
                }

                bool useMetallic = (first.MetallicOverride >= 0.0f);
                if (ImGui::Checkbox("Override Metallic", &useMetallic))
                    for (auto& mesh : model->Meshes)
                        mesh.Mat.MetallicOverride = useMetallic ? 0.0f : -1.0f;
                if (useMetallic)
                {
                    ImGui::SameLine();
                    if (ImGui::SliderFloat("##Metallic", &first.MetallicOverride, 0.0f, 1.0f))
                        for (auto& mesh : model->Meshes)
                            mesh.Mat.MetallicOverride = first.MetallicOverride;
                }
            }
        }

        // ==========================================
        // --- Компоненты ---
        // ==========================================
        if (entity.HasLight())
        {
            auto& light = entity.GetLight();

            if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::SameLine(ImGui::GetWindowWidth() - 30.0f);
                if (ImGui::SmallButton("x##RemoveLight"))
                {
                    entity.RemoveLight();
                }
                else
                {
                    const char* lightTypes[] = { "Point", "Directional", "Spot" };
                    int currentType = (int)light.Type;
                    if (ImGui::Combo("Type", &currentType, lightTypes, 3))
                        light.Type = (LightComponent::LightType)currentType;

                    ImGui::ColorEdit3("Color##Light", glm::value_ptr(light.Color));
                    ImGui::DragFloat("Intensity", &light.Intensity, 0.1f, 0.0f, 1000.0f);

                    if (light.Type != LightComponent::LightType::Directional)
                        ImGui::DragFloat("Range", &light.Range, 0.5f, 0.1f, 100.0f);
                }
            }
        }

        // ==========================================
        // --- Добавление компонентов ---
        // ==========================================
        ImGui::Separator();

        if (ImGui::Button("+ Add Component", ImVec2(-1, 0)))
            ImGui::OpenPopup("AddComponentPopup");

        if (ImGui::BeginPopup("AddComponentPopup"))
        {
            if (!entity.HasLight())
            {
                if (ImGui::MenuItem("Point Light"))
                {
                    entity.AddLight();
                }
            }

            ImGui::EndPopup();
        }

        // ==========================================
        // --- Экшены сущности ---
        // ==========================================
        ImGui::Separator();

        if (ImGui::Button("Duplicate", ImVec2(-1, 0)))
            DuplicateSelectedEntity();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.15f, 0.15f, 1.0f));

        if (ImGui::Button("Delete Entity", ImVec2(-1, 0)))
            DeleteSelectedEntity();

        ImGui::PopStyleColor(3);

        ImGui::End();
    }

    void ScenePanel::ProcessHotkeys()
    {
        if (!m_Scene || m_SelectedIndex < 0) return;

        if (ImGui::IsKeyPressed(ImGuiKey_Delete))
            DeleteSelectedEntity();

        if (ImGui::IsKeyPressed(ImGuiKey_D) && ImGui::GetIO().KeyCtrl)
            DuplicateSelectedEntity();
    }

    void ScenePanel::DeleteSelectedEntity()
    {
        if (!m_Scene || m_SelectedIndex < 0) return;

        m_Scene->RemoveEntity(m_SelectedIndex);
        m_SelectedIndex = -1; 
    }

    void ScenePanel::DuplicateSelectedEntity()
    {
        if (!m_Scene || m_SelectedIndex < 0) return;

        m_Scene->DuplicateEntity(m_SelectedIndex);
        m_SelectedIndex = (int)m_Scene->GetEntities().size() - 1;
    }

} // namespace Flux