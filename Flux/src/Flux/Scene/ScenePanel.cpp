#include "flpch.h"
#include "ScenePanel.h"

#include "Flux/Scene/Components.h"

#include <imgui.h>
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

        if (!m_Scene) {
            ImGui::TextDisabled("No scene loaded");
            ImGui::End();
            return;
        }

        ImGui::Separator();

        auto& registry = m_Scene->GetRegistry();
        auto view = registry.view<TagComponent, TransformComponent>();

        for (auto entityHandle : view)
        {
            Entity entity(entityHandle, m_Scene.get());

            if (entity.HasComponent<RelationshipComponent>() &&
                entity.GetComponent<RelationshipComponent>().ParentID != UUID(nullptr)) continue;

            DrawEntityNode(entity);
        }

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
            m_SelectedEntity = Entity();

        ImGui::End();
    }

    void ScenePanel::DrawInspector()
    {
        ImGui::Begin("Inspector");

        if (!m_Scene || !m_SelectedEntity)
        {
            ImGui::TextDisabled("Nothing selected");
            ImGui::End();
            return;
        }

        Entity& entity = m_SelectedEntity;

        // --- Имя ---
        if (entity.HasComponent<TagComponent>())
        {
            auto& tag = entity.GetComponent<TagComponent>();
            char buf[256];
            std::strncpy(buf, tag.Tag.c_str(), sizeof(buf));
            buf[sizeof(buf) - 1] = '\0';
            if (ImGui::InputText("##Name", buf, sizeof(buf)))
                tag.Tag = buf;
        }

        ImGui::Separator();

        // --- Transform ---
        if (entity.HasComponent<TransformComponent>())
        {
            auto& t = entity.GetComponent<TransformComponent>();
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::DragFloat3("Position", glm::value_ptr(t.Translation), 0.05f);

                glm::vec3 rotation = t.GetEulerAngles();
                if (ImGui::DragFloat3("Rotation", glm::value_ptr(rotation), 0.5f))
                    t.SetEulerAngles(rotation);

                ImGui::DragFloat3("Scale", glm::value_ptr(t.Scale), 0.005f, 0.001f, 100.0f);
            }
        }

        // --- Material ---
        if (entity.HasComponent<MeshComponent>())
        {
            auto model = entity.GetComponent<MeshComponent>().Model;
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
        }

        // --- Light ---
        if (entity.HasComponent<LightComponent>())
        {
            auto& light = entity.GetComponent<LightComponent>();

            if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::SameLine(ImGui::GetWindowWidth() - 30.0f);
                if (ImGui::SmallButton("x##RemoveLight"))
                {
                    entity.RemoveComponent<LightComponent>();
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

        ImGui::Separator();

        if (ImGui::Button("+ Add Component", ImVec2(-1, 0)))
            ImGui::OpenPopup("AddComponentPopup");

        if (ImGui::BeginPopup("AddComponentPopup"))
        {
            if (!entity.HasComponent<LightComponent>())
            {
                if (ImGui::MenuItem("Point Light"))
                    entity.AddComponent<LightComponent>();
            }
            ImGui::EndPopup();
        }

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

    void ScenePanel::DrawEntityNode(Entity entity)
    {
        auto& tag = entity.GetComponent<TagComponent>().Tag;
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

        if (entity.HasComponent<RelationshipComponent>()) {
            if (entity.GetComponent<RelationshipComponent>().Children.empty())
                flags |= ImGuiTreeNodeFlags_Leaf;
        }
        else {
            flags |= ImGuiTreeNodeFlags_Leaf;
        }

        bool selected = (m_SelectedEntity == entity);
        if (selected) flags |= ImGuiTreeNodeFlags_Selected;

        const char* displayName = tag.empty() ? "Unnamed Entity" : tag.c_str();
        const std::string nodeLabel = std::string(displayName) + "##" + std::to_string((uint32_t)entity);

        bool opened = ImGui::TreeNodeEx(nodeLabel.c_str(), flags);

        if (ImGui::IsItemClicked())
            m_SelectedEntity = entity;

        // --- Drag & Drop ---
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            ImGui::SetDragDropPayload("ENTITY_NODE", &entity, sizeof(Entity));
            ImGui::Text("%s", displayName);
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_NODE"))
            {
                Entity draggedEntity = *(const Entity*)payload->Data;
                m_Scene->SetParent(draggedEntity, entity);
            }
            ImGui::EndDragDropTarget();
        }

        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Duplicate")) DuplicateSelectedEntity();
            if (ImGui::MenuItem("Delete"))    DeleteSelectedEntity();

            if (entity.HasComponent<RelationshipComponent>() &&
                entity.GetComponent<RelationshipComponent>().ParentID != UUID(nullptr))
            {
                if (ImGui::MenuItem("Unparent"))
                {
                    auto& rel = entity.GetComponent<RelationshipComponent>();
                    Entity parent = m_Scene->GetEntityByUUID(rel.ParentID);
                    if (parent)
                    {
                        auto& parentRel = parent.GetComponent<RelationshipComponent>();
                        UUID childID = entity.GetComponent<IDComponent>().ID;
                        auto it = std::remove(parentRel.Children.begin(), parentRel.Children.end(), childID);
                        if (it != parentRel.Children.end())
                            parentRel.Children.erase(it, parentRel.Children.end());
                    }
                    rel.ParentID = UUID(nullptr);
                }
            }
            ImGui::EndPopup();
        }

        if (opened)
        {
            if (entity.HasComponent<RelationshipComponent>())
            {
                auto& rel = entity.GetComponent<RelationshipComponent>();
                for (UUID childID : rel.Children)
                {
                    Entity child = m_Scene->GetEntityByUUID(childID);
                    if (child)
                        DrawEntityNode(child);
                }
            }
            ImGui::TreePop();
        }
    }

    void ScenePanel::ProcessHotkeys()
    {
        if (!m_Scene || !m_SelectedEntity) return;

        if (ImGui::IsKeyPressed(ImGuiKey_Delete))
            DeleteSelectedEntity();

        if (ImGui::IsKeyPressed(ImGuiKey_D) && ImGui::GetIO().KeyCtrl)
            DuplicateSelectedEntity();
    }

    void ScenePanel::DeleteSelectedEntity()
    {
        if (!m_Scene || !m_SelectedEntity) return;

        m_Scene->DestroyEntity(m_SelectedEntity);
        m_SelectedEntity = Entity();
    }

    void ScenePanel::DuplicateSelectedEntity()
    {
        if (!m_Scene || !m_SelectedEntity) return;

        Entity newEntity = m_Scene->CreateEntity(
            m_SelectedEntity.GetComponent<TagComponent>().Tag + " Copy");

        if (m_SelectedEntity.HasComponent<TransformComponent>())
            newEntity.GetComponent<TransformComponent>() =
            m_SelectedEntity.GetComponent<TransformComponent>();

        if (m_SelectedEntity.HasComponent<MeshComponent>())
            newEntity.AddComponent<MeshComponent>(m_SelectedEntity.GetComponent<MeshComponent>());

        if (m_SelectedEntity.HasComponent<LightComponent>())
            newEntity.AddComponent<LightComponent>(m_SelectedEntity.GetComponent<LightComponent>());

        if (m_SelectedEntity.HasComponent<RelationshipComponent>())
        {
            auto& srcRel = m_SelectedEntity.GetComponent<RelationshipComponent>();
            if (srcRel.ParentID != UUID(nullptr))
            {
                Entity parent = m_Scene->GetEntityByUUID(srcRel.ParentID);
                if (parent)
                    m_Scene->SetParent(newEntity, parent);
            }
        }

        newEntity.GetComponent<TransformComponent>().Translation.x += 1.0f;
        m_SelectedEntity = newEntity;
    }

} // namespace Flux