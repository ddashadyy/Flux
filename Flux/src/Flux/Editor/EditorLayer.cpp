#include "flpch.h"
#include "EditorLayer.h"

#include "Flux/Events/MouseEvent.h"
#include "Flux/Events/KeyEvent.h"
#include "Flux/Core/Input.h"
#include "Flux/Core/KeyCodes.h"

#include <imgui.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>

namespace Flux {

    static std::vector<uint32_t> LoadSPIRV(const std::string& path)
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        FL_CORE_ASSERT(file.is_open(), "Failed to open SPIRV: {0}", path);
        size_t size = file.tellg();
        std::vector<uint32_t> buffer(size / sizeof(uint32_t));
        file.seekg(0);
        file.read(reinterpret_cast<char*>(buffer.data()), size);
        return buffer;
    }

    EditorLayer::EditorLayer() : Layer("EditorLayer") {}

    // =========================================================================
    //  Lifecycle
    // =========================================================================

    void EditorLayer::OnAttach()
    {
        auto& app = Application::Get();
        auto& device = app.GetDevice();
        auto& window = app.GetWindow();

        // Камера
        float aspect = (float)window.GetWidth() / (float)window.GetHeight();
        m_Camera = PerspectiveCamera(45.0f, aspect, 0.1f, 1000.0f);
        m_Camera.SetPosition({ 0.0f, 1.0f, 5.0f });

        // Directional light по умолчанию
        m_Light.Direction = glm::vec4(0.3f, -1.0f, -0.5f, 0.0f);
        m_Light.Color = glm::vec4(1.0f, 1.0f, 1.0f, 3.0f); // w = intensity

        // Рендерер
        m_Renderer = CreateScope<Renderer>(device);

        // Дефолтный самплер (linear, clamp) — для будущих ImGui texture views
        SamplerSpec samplerSpec{};
        samplerSpec.MagFilter = FilterMode::Linear;
        samplerSpec.MinFilter = FilterMode::Linear;
        samplerSpec.AddressU = AddressMode::ClampToEdge;
        samplerSpec.AddressV = AddressMode::ClampToEdge;
        samplerSpec.DebugName = "DefaultSampler";
        m_DefaultSampler = device.CreateSampler(samplerSpec);

        // Сцена
        m_Scene = CreateRef<Scene>();
        m_ScenePanel.SetScene(m_Scene);

        CreateDepthAndMsaa(window.GetWidth(), window.GetHeight());
        CreateRenderPass();
        CreateFramebuffers(window.GetWidth(), window.GetHeight());
        CreatePipeline();
        LoadScene();

        SetCursorMode(true);
    }

    void EditorLayer::OnDetach()
    {
        Application::Get().GetDevice().WaitIdle();

        m_Framebuffers.clear();
        m_Pipeline.reset();
        m_RenderPass.reset();
        m_DepthTexture.reset();
        m_MsaaColorTexture.reset();
        m_DefaultSampler.reset();
        m_Renderer.reset();
        m_Scene.reset();
    }

    // =========================================================================
    //  OnUpdate
    // =========================================================================

    void EditorLayer::OnUpdate(RHICommandList* cmdList)
    {
        auto& app = Application::Get();
        auto& window = app.GetWindow();
        auto& device = app.GetDevice();
        auto* swapchain = device.GetSwapchain();

        if (swapchain->NeedsResize())
        {
            device.WaitIdle();
            swapchain->Resize(window.GetWidth(), window.GetHeight());
            RecreateSwapchainResources(window.GetWidth(), window.GetHeight());
            return;
        }

        float currentTime = (float)glfwGetTime();
        float dt = currentTime - m_LastFrameTime;
        m_LastFrameTime = currentTime;

        ProcessKeyboard(dt);

        // Обновляем aspect ratio
        float aspect = (float)window.GetWidth() / (float)window.GetHeight();
        m_Camera.SetAspectRatio(aspect);

        uint32_t imageIndex = swapchain->GetCurrentImageIndex();

        // BeginRenderPass теперь принимает clearDepth и clearStencil явно
        cmdList->BeginRenderPass(
            m_RenderPass.get(),
            m_Framebuffers[imageIndex].get(),
            { 0.05f, 0.05f, 0.05f, 1.0f }, // clearColor
            1.0f,                            // clearDepth
            0                                // clearStencil
        );

        cmdList->SetViewport(0.0f, 0.0f, (float)window.GetWidth(), (float)window.GetHeight());
        cmdList->SetScissor(0, 0, window.GetWidth(), window.GetHeight());

        m_Renderer->BeginScene(*cmdList, *m_Pipeline, m_Camera, m_Light);

        for (auto& entity : m_Scene->GetEntities())
            m_Renderer->Submit(entity);

        m_Renderer->EndScene();

        cmdList->EndRenderPass();
    }

    // =========================================================================
    //  ImGui
    // =========================================================================

    void EditorLayer::OnImGuiRender()
    {
        DrawDockspace();
        DrawStatsWindow();
        m_ScenePanel.OnImGuiRender();
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

        ImGui::Text("FPS: %.1f (%.2f ms)",
            ImGui::GetIO().Framerate,
            1000.0f / ImGui::GetIO().Framerate);

        ImGui::Text("Camera: (%.2f, %.2f, %.2f)",
            m_Camera.GetPosition().x,
            m_Camera.GetPosition().y,
            m_Camera.GetPosition().z);

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
            stats.AllocationCount,
            stats.AllocationBytes / (1024.f * 1024.f));

        ImGui::End();
    }

    // =========================================================================
    //  Events
    // =========================================================================

    void EditorLayer::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<MouseMovedEvent>(FL_BIND_EVENT_FN(EditorLayer::OnMouseMoved));
        dispatcher.Dispatch<MouseScrolledEvent>(FL_BIND_EVENT_FN(EditorLayer::OnMouseScrolled));
        dispatcher.Dispatch<KeyPressedEvent>(FL_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
    }

    void EditorLayer::OnResize(uint32_t width, uint32_t height)
    {
        FL_CORE_WARN("OnResize: {}x{}", width, height);
        Application::Get().GetDevice().WaitIdle();
        RecreateSwapchainResources(width, height);
    }

    // =========================================================================
    //  Resource creation
    // =========================================================================

    void EditorLayer::RecreateSwapchainResources(uint32_t width, uint32_t height)
    {
        // Сначала освобождаем зависимые от swapchain ресурсы
        m_Framebuffers.clear();
        m_DepthTexture.reset();
        m_MsaaColorTexture.reset();

        // Пересоздаём
        CreateDepthAndMsaa(width, height);
        CreateFramebuffers(width, height);

        // Aspect камеры
        m_Camera.SetAspectRatio((float)width / (float)height);
    }

    void EditorLayer::CreateDepthAndMsaa(uint32_t width, uint32_t height)
    {
        auto& device = Application::Get().GetDevice();
        auto* swapchain = device.GetSwapchain();
        const bool msaa = (MSAA_SAMPLES != SampleCount::x1);

        // Depth
        TextureSpec depthSpec{};
        depthSpec.Width = width;
        depthSpec.Height = height;
        depthSpec.MipLevels = 1;
        depthSpec.ImageFormat = Format::D32_SFLOAT;
        depthSpec.Usage = TextureUsage::DepthStencil; // битмаска — только depth attachment
        depthSpec.Samples = MSAA_SAMPLES;
        depthSpec.DebugName = "DepthBuffer";
        m_DepthTexture = device.CreateTexture(depthSpec);

        // MSAA color resolve buffer
        if (msaa)
        {
            TextureSpec msaaSpec{};
            msaaSpec.Width = width;
            msaaSpec.Height = height;
            msaaSpec.ImageFormat = swapchain->GetFormat();
            // RenderTarget — рендерим в него, потом resolve в swapchain
            // Transient убран из нового RHI — используем RenderTarget
            msaaSpec.Usage = TextureUsage::RenderTarget;
            msaaSpec.MipLevels = 1;
            msaaSpec.Samples = MSAA_SAMPLES;
            msaaSpec.DebugName = "MsaaColorBuffer";
            m_MsaaColorTexture = device.CreateTexture(msaaSpec);
        }
    }

    void EditorLayer::CreateRenderPass()
    {
        auto& device = Application::Get().GetDevice();
        auto* swapchain = device.GetSwapchain();
        const bool msaa = (MSAA_SAMPLES != SampleCount::x1);

        RenderPassDesc desc{};
        desc.ColorFormats = { swapchain->GetFormat() };
        desc.HasDepth = true;
        desc.DepthFormat = Format::D32_SFLOAT;
        desc.Samples = MSAA_SAMPLES;

        desc.ColorLoadOp = AttachmentLoadOp::Clear;
        desc.ColorStoreOp = AttachmentStoreOp::Store;
        desc.DepthLoadOp = AttachmentLoadOp::Clear;
        desc.DepthStoreOp = AttachmentStoreOp::DontCare;

        desc.ColorInitialLayout = ImageLayout::Undefined;
        desc.DepthInitialLayout = ImageLayout::Undefined;
        desc.DepthFinalLayout = ImageLayout::DepthStencilAttachment;

        if (msaa)
        {
            desc.ColorFinalLayout   = ImageLayout::ColorAttachment; // MSAA буфер остаётся в ColorAttachment
            desc.HasResolve         = true;
            desc.ResolveInitialLayout = ImageLayout::Undefined;
            desc.ResolveFinalLayout   = ImageLayout::ColorAttachment; // resolve → ColorAttachment, не Present
                                                                       // ImGui дочитает и сделает Present
        }
        else
        {
            desc.ColorFinalLayout = ImageLayout::ColorAttachment; // не Present — ImGui ещё рендерит поверх
        }

        m_RenderPass = device.CreateRenderPass(desc);
    }

    void EditorLayer::CreateFramebuffers(uint32_t width, uint32_t height)
    {
        auto& device = Application::Get().GetDevice();
        auto* swapchain = device.GetSwapchain();
        const bool msaa = (MSAA_SAMPLES != SampleCount::x1);
        uint32_t count = swapchain->GetImageCount();

        m_Framebuffers.resize(count);
        for (uint32_t i = 0; i < count; i++)
        {
            FramebufferSpec spec{};
            spec.RenderPass = m_RenderPass.get();
            spec.Width = width;
            spec.Height = height;

            if (msaa)
            {
                // MSAA: color = msaa buffer, resolve = swapchain image
                spec.ColorTargets = { m_MsaaColorTexture.get() };
                spec.DepthTarget = m_DepthTexture.get();
                spec.ResolveTarget = swapchain->GetColorTarget(i);
            }
            else
            {
                // Без MSAA: color = swapchain image напрямую
                spec.ColorTargets = { swapchain->GetColorTarget(i) };
                spec.DepthTarget = m_DepthTexture.get();
            }

            m_Framebuffers[i] = device.CreateFramebuffer(spec);
        }
    }

    void EditorLayer::CreatePipeline()
    {
        auto& device = Application::Get().GetDevice();

        auto vertSpv = LoadSPIRV("C:/dev/Flux/SandBox/assets/shaders/shader.vert.spv");
        auto fragSpv = LoadSPIRV("C:/dev/Flux/SandBox/assets/shaders/shader.frag.spv");
        m_VertShader = device.CreateShader(ShaderStage::Vertex, vertSpv);
        m_FragShader = device.CreateShader(ShaderStage::Fragment, fragSpv);

        BufferLayout vertexLayout = {
            { ShaderDataType::Float3 }, // Position
            { ShaderDataType::Float3 }, // Normal
            { ShaderDataType::Float2 }, // TexCoord
            { ShaderDataType::Float3 }, // Tangent
        };

        PipelineDesc desc{};
        desc.VertexShader = m_VertShader.get();
        desc.FragmentShader = m_FragShader.get();
        desc.VertexLayout = vertexLayout;
        desc.RenderPass = m_RenderPass.get();
        desc.Topology = PrimitiveTopology::TriangleList;
        desc.Samples = MSAA_SAMPLES;

        // PushConstantRange — новое имя, вместо PipelineLayoutDesc
        desc.PushConstants.Stage = ShaderStage::Vertex | ShaderStage::Fragment;
        desc.PushConstants.Offset = 0;
        desc.PushConstants.Size = sizeof(PushConstantData);

        // Descriptor set layouts из рендерера
        desc.DescriptorSetLayouts = {
            m_Renderer->GetGlobalDescriptorSetLayout(),
            m_Renderer->GetTextureDescriptorSetLayout()
        };

        // DepthStencil state
        desc.DepthStencil.DepthTest = true;
        desc.DepthStencil.DepthWrite = true;
        desc.DepthStencil.DepthCompare = CompareOp::Less; // явно, не хардкод в Vulkan

        // Rasterizer state — явно, чтобы было понятно
        desc.Rasterizer.Cull = CullMode::Back;
        desc.Rasterizer.Fill = FillMode::Solid;
        desc.Rasterizer.Front = FrontFace::CounterClockwise;

        // Blend — непрозрачная геометрия
        desc.Blend = BlendPreset::Opaque();

        desc.DebugName = "ScenePipeline";

        m_Pipeline = device.CreatePipeline(desc);
    }

    void EditorLayer::LoadScene()
    {
        auto& assetManager = Application::Get().GetAssetManager();
        auto* texLayout = m_Renderer->GetTextureDescriptorSetLayout();

        auto batmobile = assetManager.LoadModel(
            "C:\\dev\\Flux\\SandBox\\assets\\models\\batmobile\\arkham_knight_batmobile_advanced_rig.obj",
            texLayout
        );
        auto& batmobileEntity = m_Scene->AddEntity(batmobile, "Batmobile");
        batmobileEntity.GetTransform().Position = { -6.0f, -0.5f, -4.5f };
        batmobileEntity.GetTransform().Scale = { 0.25f, 0.25f, 0.25f };

        auto street = assetManager.LoadModel(
            "C:\\dev\\Flux\\SandBox\\assets\\models\\street\\dark_street_scene.obj",
            texLayout
        );
        auto& streetEntity = m_Scene->AddEntity(street, "Street");
        (void)streetEntity;
    }

    // =========================================================================
    //  Camera controller
    // =========================================================================

    void EditorLayer::ProcessKeyboard(float dt)
    {
        if (!m_CursorLocked) return;

        float speed = m_CameraSpeed * dt;
        glm::vec3 move(0.0f);

        if (Input::IsKeyPressed(FL_KEY_W))           move.z += 1.0f;
        if (Input::IsKeyPressed(FL_KEY_S))           move.z -= 1.0f;
        if (Input::IsKeyPressed(FL_KEY_A))           move.x -= 1.0f;
        if (Input::IsKeyPressed(FL_KEY_D))           move.x += 1.0f;
        if (Input::IsKeyPressed(FL_KEY_SPACE))       move.y += 1.0f;
        if (Input::IsKeyPressed(FL_KEY_LEFT_SHIFT))  move.y -= 1.0f;

        if (glm::length(move) > 0.0f)
            move = glm::normalize(move);

        glm::vec3 pos = m_Camera.GetPosition();
        pos += m_Camera.GetForward() * move.z * speed;
        pos += m_Camera.GetRight() * move.x * speed;
        pos += glm::vec3(0, 1, 0) * move.y * speed;
        m_Camera.SetPosition(pos);
    }

    bool EditorLayer::OnMouseMoved(MouseMovedEvent& e)
    {
        if (!m_CursorLocked) return false;

        if (m_FirstMouse)
        {
            m_LastX = e.GetX(); m_LastY = e.GetY();
            m_FirstMouse = false;
            return false;
        }

        float xOff = (e.GetX() - m_LastX) * m_MouseSensitivity;
        float yOff = (m_LastY - e.GetY()) * m_MouseSensitivity;
        m_LastX = e.GetX();
        m_LastY = e.GetY();

        glm::vec3 rot = m_Camera.GetRotation();
        rot.y += xOff;
        rot.x = glm::clamp(rot.x + yOff, -89.0f, 89.0f);
        m_Camera.SetRotation(rot);
        return false;
    }

    bool EditorLayer::OnMouseScrolled(MouseScrolledEvent& e)
    {
        m_Camera.SetFOV(glm::clamp(m_Camera.GetFOV() - e.GetYOffset(), 1.0f, 90.0f));
        return false;
    }

    bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
    {
        if (e.GetKeyCode() == FL_KEY_ESCAPE)
        {
            m_CursorLocked = !m_CursorLocked;
            SetCursorMode(m_CursorLocked);
        }
        return false;
    }

    void EditorLayer::SetCursorMode(bool locked)
    {
        GLFWwindow* w = static_cast<GLFWwindow*>(
            Application::Get().GetWindow().GetNativeWindow());

        if (locked)
        {
            glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            m_FirstMouse = true;
        }
        else
        {
            glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

} // namespace Flux
