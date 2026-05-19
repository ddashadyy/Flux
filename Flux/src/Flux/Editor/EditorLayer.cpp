#include "flpch.h"
#include "EditorLayer.h"

#include "Flux/Events/MouseEvent.h"
#include "Flux/Events/KeyEvent.h"
#include "Flux/Core/Input.h"
#include "Flux/Core/KeyCodes.h"
#include "Flux/ImGui/ImGuiLayer.h"

#include "Flux/Renderer/PrimitiveFactory.h"
#include "Flux/Renderer/RayCaster.h"

#include "Flux/Scene/Entity.h"
#include "Flux/Scene/Components.h"

#include <imgui.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include <filesystem>

namespace Flux {

    EditorLayer::EditorLayer() : Layer("EditorLayer") {}

    // =========================================================================
    //  Lifecycle
    // =========================================================================

    void EditorLayer::OnAttach()
    {
        auto& app = Application::Get();
        auto& window = app.GetWindow();

        float aspect = (float)window.GetWidth() / (float)window.GetHeight();

        m_Camera = EditorCamera(45.0f, aspect, 0.1f, 1000.0f);
        m_Camera.GetCamera().SetPosition({ -8.29f, 2.35f, -8.96f });
        m_Camera.GetCamera().SetRotation({ -12.5f, -307.0f, 0.0f });
        m_Camera.SetKeyCallback([this](int key) {
            if (m_Camera.IsCursorLocked()) return;
            if (key == FL_KEY_T) m_GizmoOperation = ImGuizmo::TRANSLATE;
            if (key == FL_KEY_R) m_GizmoOperation = ImGuizmo::ROTATE;
            if (key == FL_KEY_G) m_GizmoOperation = ImGuizmo::SCALE;
            });

        m_SceneRenderer = CreateScope<SceneRenderer>();
        m_SceneRenderer->Init(app.GetDevice());

        m_Scene = CreateRef<Scene>();
        m_ScenePanel.SetScene(m_Scene);

        m_ContentBrowser.SetImportCallback([this](const std::string& path) {
            if (!path.empty())
                LoadModel(path);
            });
        m_ContentBrowser.SetRootPath(std::filesystem::current_path() / "assets");

        LoadDefaultScene();
    }

    void EditorLayer::OnDetach()
    {
        Application::Get().GetDevice().WaitIdle();
        PrimitiveFactory::Shutdown();

        m_SceneRenderer.reset();
        m_Scene.reset();
    }

    void EditorLayer::LoadModel(const std::string& path)
    {
        auto& assetManager = Application::Get().GetAssetManager();
        auto* texLayout = m_SceneRenderer->GetTextureDescriptorSetLayout();

        auto model = assetManager.LoadModel(path, texLayout);
        if (!model) return;

        const std::string name = std::filesystem::path(path).stem().string();
        Entity entity = m_Scene->CreateEntity(name);
        entity.AddComponent<MeshComponent>(model);
    }

    // =========================================================================
    //  OnUpdate
    // =========================================================================

    void EditorLayer::OnUpdate(RHICommandList* cmdList, uint32_t imageIndex)
    {
        if (m_ViewportNeedsResize)
        {
            m_ViewportNeedsResize = false;
            m_ViewportSize = m_PendingViewportSize;

            if (m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f)
            {
                m_SceneRenderer->OnResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
                m_Camera.GetCamera().SetAspectRatio(m_ViewportSize.x / m_ViewportSize.y);
            }
        }

        float currentTime = (float)glfwGetTime();
        float dt = currentTime - m_LastFrameTime;
        m_LastFrameTime = currentTime;

        m_Camera.OnUpdate(dt);
        m_Scene->OnUpdate(dt);
        m_SceneRenderer->Render(cmdList, m_Scene, m_Camera.GetCamera());

        if (!m_Scene->GetRegistry().view<DestroyFlag>().empty())
        {
            Application::Get().GetDevice().WaitIdle();
            m_Scene->ProcessDeletions();
        }
    }

    // =========================================================================
    //  ImGui
    // =========================================================================

    void EditorLayer::OnImGuiRender()
    {
        ImGuizmo::BeginFrame();

        DrawDockspace();
        DrawViewport();
        DrawStatsWindow();
        m_ScenePanel.OnImGuiRender();
        m_ContentBrowser.OnImGuiRender();
    }

    void EditorLayer::OnEvent(Event& event)
    {
        m_Camera.OnEvent(event);
    }

    void EditorLayer::OnResize(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0) return;
        Application::Get().GetImGuiLayer()->CreateFramebuffers(width, height);
        m_Camera.GetCamera().SetAspectRatio((float)width / (float)height);
    }

    void EditorLayer::DrawDockspace()
    {
        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus |
            ImGuiWindowFlags_NoBackground;

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("##Dockspace", nullptr, flags);
        ImGui::PopStyleVar(3);
        ImGui::DockSpace(ImGui::GetID("MainDockspace"));
        ImGui::End();
    }

    void EditorLayer::DrawStatsWindow()
    {
        ImGui::Begin("Stats");

        ImGui::Text("FPS: %.1f (%.2f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
        ImGui::Text("Camera: (%.2f, %.2f, %.2f)",
            m_Camera.GetCamera().GetPosition().x,
            m_Camera.GetCamera().GetPosition().y,
            m_Camera.GetCamera().GetPosition().z);

        auto stats = Application::Get().GetDevice().GetMemoryStatistics();
        if (stats.Budget > 0)
        {
            float usageMb = stats.Usage / (1024.f * 1024.f);
            float budgetMb = stats.Budget / (1024.f * 1024.f);
            float usagePct = (float)stats.Usage / (float)stats.Budget;

            ImGui::Text("VRAM: %.1f / %.1f MB (%.0f%%)", usageMb, budgetMb, usagePct * 100.0f);
            ImGui::ProgressBar(usagePct, ImVec2(-1, 0));
        }

        ImGui::Text("Allocations: %u (%.1f MB)",
            stats.AllocationCount, stats.AllocationBytes / (1024.f * 1024.f));

        ImGui::End();
    }

    void EditorLayer::DrawViewport()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Viewport");

        m_ViewportFocused = ImGui::IsWindowFocused();
        ImVec2 size = ImGui::GetContentRegionAvail();

        uint32_t w = (uint32_t)size.x;
        uint32_t h = (uint32_t)size.y;

        if (w > 0 && h > 0 && (w != (uint32_t)m_ViewportSize.x || h != (uint32_t)m_ViewportSize.y))
        {
            m_PendingViewportSize = { (float)w, (float)h };
            m_ViewportNeedsResize = true;
        }

        ImGui::Image(m_SceneRenderer->GetViewportTextureID(), size);

        // --- Drag & Drop из Content Browser ---
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
            {
                std::string path = (const char*)payload->Data;
                LoadModel(path);
            }
            ImGui::EndDragDropTarget();
        }

        // --- Ray picking ---
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)
            && ImGui::IsWindowHovered()
            && !m_Camera.IsCursorLocked()
            && !ImGuizmo::IsOver())
        {
            ImVec2 viewportPos = ImGui::GetWindowPos();
            ImVec2 contentMin = ImGui::GetCursorStartPos();
            ImVec2 mousePos = ImGui::GetMousePos();

            float mouseX = mousePos.x - (viewportPos.x + contentMin.x);
            float mouseY = mousePos.y - (viewportPos.y + contentMin.y);

            float ndcX = (mouseX / m_ViewportSize.x) * 2.0f - 1.0f;
            float ndcY = 1.0f - (mouseY / m_ViewportSize.y) * 2.0f;

            Entity pickedEntity = RayCaster::PickEntity(ndcX, ndcY, m_Camera.GetCamera(), *m_Scene);
            m_ScenePanel.SetSelectedEntity(pickedEntity);
        }

        // --- ImGuizmo ---
        Entity selectedEntity = m_ScenePanel.GetSelectedEntity();
        if (selectedEntity && !m_Camera.IsCursorLocked())
        {
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();

            ImVec2 vpPos = ImGui::GetWindowPos();
            ImGuizmo::SetRect(vpPos.x, vpPos.y, m_ViewportSize.x, m_ViewportSize.y);

            const glm::mat4& view = m_Camera.GetCamera().GetViewMatrix();
            const glm::mat4& proj = m_Camera.GetCamera().GetProjectionMatrix();

            auto& t = selectedEntity.GetComponent<TransformComponent>();
            glm::mat4 modelMat = t.WorldMatrix;

            ImGuizmo::Manipulate(
                glm::value_ptr(view),
                glm::value_ptr(proj),
                m_GizmoOperation,
                ImGuizmo::LOCAL,
                glm::value_ptr(modelMat)
            );

            if (ImGuizmo::IsUsing())
            {
                glm::mat4 localMat = modelMat;
                if (selectedEntity.HasComponent<RelationshipComponent>())
                {
                    auto& rel = selectedEntity.GetComponent<RelationshipComponent>();
                    if (rel.ParentID != UUID(nullptr))
                    {
                        Entity parent = m_Scene->GetEntityByUUID(rel.ParentID);
                        if (parent)
                            localMat = glm::inverse(
                                parent.GetComponent<TransformComponent>().WorldMatrix) * modelMat;
                    }
                }

                t.Translation = glm::vec3(localMat[3]);

                glm::vec3 scale;
                scale.x = glm::length(glm::vec3(localMat[0]));
                scale.y = glm::length(glm::vec3(localMat[1]));
                scale.z = glm::length(glm::vec3(localMat[2]));
                t.Scale = scale;

                glm::mat4 rotMat = localMat;
                rotMat[0] /= scale.x;
                rotMat[1] /= scale.y;
                rotMat[2] /= scale.z;
                t.Rotation = glm::quat_cast(rotMat);

                if (selectedEntity.HasComponent<RelationshipComponent>() &&
                    selectedEntity.GetComponent<RelationshipComponent>().ParentID != UUID(nullptr))
                {
                    Entity parent = m_Scene->GetEntityByUUID(
                        selectedEntity.GetComponent<RelationshipComponent>().ParentID);
                    t.WorldMatrix = parent
                        ? parent.GetComponent<TransformComponent>().WorldMatrix * t.GetLocalMatrix()
                        : t.GetLocalMatrix();
                }
                else
                    t.WorldMatrix = t.GetLocalMatrix();
            }
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

    // =========================================================================
    //  Сцена
    // =========================================================================

    void EditorLayer::RecreateSwapchainResources(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0) return;
        Application::Get().GetImGuiLayer()->CreateFramebuffers(width, height);
    }

    void EditorLayer::LoadDefaultScene()
    {
        auto* texLayout = m_SceneRenderer->GetTextureDescriptorSetLayout();
        auto& device = Application::Get().GetDevice();

        auto floor = PrimitiveFactory::CreatePlane(device, texLayout, 20.0f, 4);
        Entity floorEntity = m_Scene->CreateEntity("Floor");
        floorEntity.AddComponent<MeshComponent>(floor);
        floorEntity.GetComponent<TransformComponent>().Translation = { 0.0f, -0.5f, 0.0f };

        auto cube = PrimitiveFactory::CreateCube(device, texLayout);
        Entity cubeEntity = m_Scene->CreateEntity("Cube");
        cubeEntity.AddComponent<MeshComponent>(cube);
        cubeEntity.GetComponent<TransformComponent>().Translation = { 0.0f, 0.0f, 0.0f };

        auto makeLight = [&](const char* name, glm::vec3 pos) {
            Entity e = m_Scene->CreateEntity(name);
            e.GetComponent<TransformComponent>().Translation = pos;
            e.AddComponent<LightComponent>();
            e.GetComponent<LightComponent>().Color = { 1.0f, 0.85f, 0.5f };
            e.GetComponent<LightComponent>().Intensity = 300.0f;
            };

        makeLight("Point Light 1", { 0.02f, 2.82f,  5.40f });
        makeLight("Point Light 2", { 0.11f, 2.82f,  2.13f });
        makeLight("Point Light 3", { 0.02f, 2.82f, -2.04f });
        makeLight("Point Light 4", { -0.02f, 2.82f, -5.45f });
    }

} // namespace Flux