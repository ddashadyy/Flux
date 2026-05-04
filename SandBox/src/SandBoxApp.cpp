#include <Flux.h>
#include "imgui/imgui.h"
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <fstream>

#include "Flux/Scene/Scene.h"
#include "Flux/Scene/Entity.h"

static std::vector<uint32_t> ReadSPV(const std::string& path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    FL_CORE_ASSERT(file.is_open(), "Failed to open shader file!");
    size_t size = file.tellg();
    std::vector<uint32_t> buffer(size / sizeof(uint32_t));
    file.seekg(0);
    file.read((char*)buffer.data(), size);
    return buffer;
}

const Flux::SampleCount MSAA_SAMPLES = Flux::SampleCount::x8;

//class ExampleLayer : public Flux::Layer {
//public:
//    ExampleLayer() : Layer("Example"), m_Camera(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f)
//    {
//        auto* device = &Flux::Application::Get().GetDevice();
//        auto* swapchain = device->GetSwapchain();
//        auto& window = Flux::Application::Get().GetWindow();
//        auto& assetManager = Flux::Application::Get().GetAssetManager();
//
//        // =============================================
//        //  1. Descriptor Set Layouts
//        // =============================================
//
//        // Set 0: Глобальный (камера)
//        Flux::DescriptorSetLayoutDesc globalLayoutDesc{};
//        globalLayoutDesc.Bindings.emplace_back(0, Flux::DescriptorType::UniformBuffer, Flux::ShaderStage::Vertex | Flux::ShaderStage::Fragment, 1);
//        m_GlobalSetLayout = device->CreateDescriptorSetLayout(globalLayoutDesc);
//
//        // Set 1: Текстуры (albedo, normal, roughnessMetallic)
//        Flux::DescriptorSetLayoutDesc textureLayoutDesc{};
//        textureLayoutDesc.Bindings.emplace_back(0, Flux::DescriptorType::CombinedImageSampler, Flux::ShaderStage::Fragment, 1);
//        textureLayoutDesc.Bindings.emplace_back(1, Flux::DescriptorType::CombinedImageSampler, Flux::ShaderStage::Fragment, 1);
//        textureLayoutDesc.Bindings.emplace_back(2, Flux::DescriptorType::CombinedImageSampler, Flux::ShaderStage::Fragment, 1);
//        m_TextureSetLayout = device->CreateDescriptorSetLayout(textureLayoutDesc);
//
//        // =============================================
//        //  2. Загрузка моделей и добавление в сцену
//        // =============================================
//        auto arkhamKnight = assetManager.LoadModel(
//            "C:\\dev\\Flux\\SandBox\\assets\\models\\arkham_knight\\batman_arkham_knight_the_arkham_knight.obj",
//            m_TextureSetLayout.get()
//        );
//
//        auto& arkhamKnightEntity = m_Scene.AddEntity(arkhamKnight, "ArkhamKnight");
//        arkhamKnightEntity.GetTransform().Position = { -6.0f, -0.5f, 0.0f };
//		arkhamKnightEntity.GetTransform().Rotation = { 0.0f, 180.0f, 0.0f };
//        arkhamKnightEntity.GetTransform().Scale = { 1.0f, 1.0f, 1.0f };
//
//        auto batmobile = assetManager.LoadModel(
//            "C:\\dev\\Flux\\SandBox\\assets\\models\\batmobile\\arkham_knight_batmobile_advanced_rig.obj",
//            m_TextureSetLayout.get()
//        );
//
//        auto& batmobileEntity = m_Scene.AddEntity(batmobile, "Batmobile");
//        batmobileEntity.GetTransform().Position = { -6.0f, -0.5f, -4.5f };
//        batmobileEntity.GetTransform().Scale = { 0.25f, 0.25f, 0.25f };
//
//
//        auto street = assetManager.LoadModel(
//            "C:\\dev\\Flux\\SandBox\\assets\\models\\street\\dark_street_scene.obj",
//            m_TextureSetLayout.get()
//		);
//       
//        auto& streetEntity = m_Scene.AddEntity(street, "Street");
//        
//
//        //auto encient = assetManager.LoadModel("C:\\dev\\Flux\\SandBox\\assets\\models\\ancient_corinth\\ancient_corinth.obj", m_TextureSetLayout.get());
//        //auto& encientEntity = m_Scene.AddEntity(encient, "Encient");
//
//        //auto patau = assetManager.LoadModel("C:\\dev\\Flux\\SandBox\\assets\\models\\dima\\dima.obj", m_TextureSetLayout.get());
//        //auto& patauEntity = m_Scene.AddEntity(patau, "patau");
//
//
//
//        // =============================================
//        //  3. Глобальный Uniform буфер (камера)
//        // =============================================
//        Flux::BufferSpec globalUBOSpec{};
//        globalUBOSpec.Size = sizeof(GlobalUBO);
//        globalUBOSpec.Usage = Flux::BufferUsage::Uniform;
//        globalUBOSpec.CpuVisible = true;
//        m_GlobalUBOBuffer = device->CreateBuffer(globalUBOSpec);
//
//        // =============================================
//        //  4. Глобальный Descriptor Set (Set 0)
//        // =============================================
//        m_GlobalDescriptorSet = device->CreateDescriptorSet(m_GlobalSetLayout.get());
//        m_GlobalDescriptorSet->BindBuffer(0, m_GlobalUBOBuffer.get());
//        m_GlobalDescriptorSet->Update();
//
//        // =============================================
//        //  5. Шейдеры
//        // =============================================
//        auto vertSpirv = ReadSPV("C:\\dev\\Flux\\SandBox\\assets\\shaders\\shader.vert.spv");
//        auto fragSpirv = ReadSPV("C:\\dev\\Flux\\SandBox\\assets\\shaders\\shader.frag.spv");
//        m_VertShader = device->CreateShader(Flux::ShaderStage::Vertex, vertSpirv);
//        m_FragShader = device->CreateShader(Flux::ShaderStage::Fragment, fragSpirv);
//
//        // =============================================
//        //  6. Depth texture, RenderPass, Framebuffers
//        // =============================================
//        const bool isMsaaEnabled = (MSAA_SAMPLES != Flux::SampleCount::x1);
//
//        Flux::TextureSpec depthSpec{};
//        depthSpec.Width = window.GetWidth();
//        depthSpec.Height = window.GetHeight();
//        depthSpec.MipLevels = 1;
//        depthSpec.ImageFormat = Flux::Format::D32_SFLOAT;
//        depthSpec.Usage = Flux::TextureUsage::DepthStencil;
//        depthSpec.Samples = MSAA_SAMPLES;
//        m_DepthTexture = device->CreateTexture(depthSpec);
//
//        if (isMsaaEnabled)
//        {
//            Flux::TextureSpec msaaSpec{};
//            msaaSpec.Width = window.GetWidth();
//            msaaSpec.Height = window.GetHeight();
//            msaaSpec.ImageFormat = swapchain->GetFormat();
//            msaaSpec.Usage = Flux::TextureUsage::Transient;
//            msaaSpec.MipLevels = 1;
//            msaaSpec.Samples = MSAA_SAMPLES;
//            m_MsaaColorTexture = device->CreateTexture(msaaSpec);
//        }
//
//        Flux::RenderPassDesc rpDesc{};
//        rpDesc.ColorFormats = { swapchain->GetFormat() };
//        rpDesc.HasDepth = true;
//        rpDesc.DepthFormat = Flux::Format::D32_SFLOAT;
//        rpDesc.Samples = MSAA_SAMPLES;
//        rpDesc.ColorLoadOp = Flux::AttachmentLoadOp::Clear;
//        rpDesc.ColorStoreOp = Flux::AttachmentStoreOp::Store;
//        rpDesc.DepthLoadOp = Flux::AttachmentLoadOp::Clear;
//        rpDesc.DepthStoreOp = Flux::AttachmentStoreOp::DontCare;
//
//        // КРИТИЧЕСКИ ВАЖНО: Лейауты зависят от того, куда мы рендерим!
//        if (isMsaaEnabled)
//        {
//            // При MSAA мы рендерим во временную картинку, ее начальное состояние не важно
//            rpDesc.ColorInitialLayout = Flux::ImageLayout::Undefined;
//            // В конце она должна остаться цветовым аттачментом, чтобы Вулкан мог сделать Resolve
//            rpDesc.ColorFinalLayout = Flux::ImageLayout::ColorAttachment;
//        }
//        else
//        {
//            // Без MSAA мы рендерим ПРЯМО в Swapchain. 
//            // После acquire у Swapchain лейаут PRESENT_SRC_KHR. Мы обязаны указать его!
//            rpDesc.ColorInitialLayout = Flux::ImageLayout::Present;
//            // В конце мы должны вернуть его в PRESENT, чтобы ImGui или Presentation могли его забрать
//            rpDesc.ColorFinalLayout = Flux::ImageLayout::Present;
//        }
//
//        m_SceneRenderPass = device->CreateRenderPass(rpDesc);
//
//        uint32_t imageCount = swapchain->GetImageCount();
//        m_SceneFramebuffers.resize(imageCount);
//        for (uint32_t i = 0; i < imageCount; i++)
//        {
//            Flux::FramebufferSpec fbSpec{};
//            fbSpec.RenderPass = m_SceneRenderPass.get();
//            fbSpec.ResolveTarget = nullptr; // ЯВНО сбрасываем, чтобы не было случайных 3-х аттачментов
//
//            if (isMsaaEnabled)
//            {
//                fbSpec.ColorTargets = { m_MsaaColorTexture.get() };
//                fbSpec.DepthTarget = m_DepthTexture.get();
//                fbSpec.ResolveTarget = swapchain->GetColorTarget(i);
//            }
//            else
//            {
//                fbSpec.ColorTargets = { swapchain->GetColorTarget(i) };
//                fbSpec.DepthTarget = m_DepthTexture.get();
//            }
//
//            fbSpec.Width = window.GetWidth();
//            fbSpec.Height = window.GetHeight();
//            m_SceneFramebuffers[i] = device->CreateFramebuffer(fbSpec);
//        }
//
//        // =============================================
//        //  7. Pipeline
//        // =============================================
//        Flux::BufferLayout vertexLayout = {
//            { Flux::ShaderDataType::Float3 }, // Position
//            { Flux::ShaderDataType::Float3 }, // Normal
//            { Flux::ShaderDataType::Float2 }, // TexCoord
//            { Flux::ShaderDataType::Float3 }, // Tangent 
//        };
//
//        Flux::PipelineLayoutDesc pushConstantDesc;
//        pushConstantDesc.Stage = Flux::ShaderStage::Vertex | Flux::ShaderStage::Fragment;
//        pushConstantDesc.Offset = 0;
//        pushConstantDesc.Size = sizeof(PushConstantData);
//
//        Flux::PipelineDesc pipelineDesc{};
//        pipelineDesc.VertexShader = m_VertShader.get();
//        pipelineDesc.FragmentShader = m_FragShader.get();
//        pipelineDesc.VertexLayout = vertexLayout;
//        pipelineDesc.pipelineLayoutDesc = pushConstantDesc;
//        pipelineDesc.DescriptorSetLayouts = { m_GlobalSetLayout.get(), m_TextureSetLayout.get() };
//        pipelineDesc.RenderPass = m_SceneRenderPass.get();
//        pipelineDesc.Topology = Flux::PrimitiveTopology::TriangleList;
//        pipelineDesc.Samples = MSAA_SAMPLES;
//        pipelineDesc.DepthStencil.DepthTest = true;
//        pipelineDesc.DepthStencil.DepthWrite = true;
//
//        m_Pipeline = device->CreatePipeline(pipelineDesc);
//
//        m_Camera.SetPosition(glm::vec3(-8.29f, 2.35f, -8.96f));
//        m_Camera.SetRotation(glm::vec3(-12.5f, -307.0f, 0.0f));
//        SetCursorMode(true);
//    }
//
//    void OnUpdate(Flux::RHICommandList* cmdList) override
//    {
//        auto* swapchain = Flux::Application::Get().GetDevice().GetSwapchain();
//        uint32_t imageIndex = swapchain->GetCurrentImageIndex();
//
//        float currentTime = (float)glfwGetTime();
//        float deltaTime = currentTime - m_LastFrameTime;
//        m_LastFrameTime = currentTime;
//
//        ProcessKeyboard(deltaTime);
//
//        auto& window = Flux::Application::Get().GetWindow();
//        float aspect = (float)window.GetWidth() / (float)window.GetHeight();
//
//        // Обновляем глобальный UBO
//        GlobalUBO globalUBO{};
//        globalUBO.view = m_Camera.GetViewMatrix();
//        globalUBO.projection = glm::perspective(glm::radians(m_Camera.GetFOV()), aspect, 0.1f, 100.0f);
//        globalUBO.projection[1][1] *= -1;
//        globalUBO.cameraPos = m_Camera.GetPosition();
//        globalUBO.lightCount = 4;
//        globalUBO.lights[0] = { glm::vec4(0.02f, 2.82f,  5.40f, 0.0f), glm::vec4(1.0f, 0.85f, 0.5f, 300.0f) };
//        globalUBO.lights[1] = { glm::vec4(0.11f, 2.82f,  2.13f, 0.0f), glm::vec4(1.0f, 0.85f, 0.5f, 300.0f) };
//        globalUBO.lights[2] = { glm::vec4(0.02f, 2.82f, -2.04f, 0.0f), glm::vec4(1.0f, 0.85f, 0.5f, 300.0f) };
//        globalUBO.lights[3] = { glm::vec4(-0.02f, 2.82f, -5.45f, 0.0f), glm::vec4(1.0f, 0.85f, 0.5f, 300.0f) };
//        m_GlobalUBOBuffer->SetData(&globalUBO, sizeof(GlobalUBO));
//
//        // Рендер
//        cmdList->BeginRenderPass(m_SceneRenderPass.get(), m_SceneFramebuffers[imageIndex].get(), { 0.05f, 0.05f, 0.05f, 1.0f });
//        cmdList->SetViewport(0, 0, (float)window.GetWidth(), (float)window.GetHeight());
//        cmdList->SetScissor(0, 0, window.GetWidth(), window.GetHeight());
//        cmdList->SetPipeline(m_Pipeline.get());
//        cmdList->BindDescriptorSet(0, m_GlobalDescriptorSet.get());
//
//        // Цикл по сцене
//        for (const auto& entity : m_Scene.GetEntities())
//        {
//            auto model = entity.GetModel();
//            if (!model) continue;
//
//            PushConstantData pushData{};
//            pushData.model = entity.GetTransform().GetMatrix();
//
//            for (const auto& subMesh : model->Meshes)
//            {
//                pushData.color = subMesh.Mat.Color;
//                pushData.roughnessOverride = subMesh.Mat.RoughnessOverride;
//                pushData.metallicOverride = subMesh.Mat.MetallicOverride;
//
//                if (entity.GetName() == "Street")
//                {
//                    pushData.color = { 0.0f, 0.0f, 0.0f, 1.0f };
//                }
//
//				else if (entity.GetName() == "Batmobile")
//                {
//                    pushData.color = { 0.8f, 0.8f, 0.8f, 1.0f };
//                    pushData.roughnessOverride = 0.2f;
//                    pushData.metallicOverride = 0.38f;
//                }
//
//                cmdList->PushConstants(m_Pipeline.get(), &pushData);
//                cmdList->BindDescriptorSet(1, subMesh.Mat.DescriptorSet.get());
//                subMesh.Draw(*cmdList);
//            }
//        }
//
//        cmdList->EndRenderPass();
//    }
//
//    void OnImGuiRender() override
//    {
//        ImGui::Text("Renderer API: Vulkan");
//        ImGui::Text("FPS: %.1f (%.2f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
//        ImGui::Text("Camera Position: (x = %.2f, y = %.2f, z = %.2f)",
//            m_Camera.GetPosition().x, m_Camera.GetPosition().y, m_Camera.GetPosition().z);
//        ImGui::Text("Camera Rotation: (x = %.2f, y = %.2f, z = %.2f)",
//            m_Camera.GetRotation().x, m_Camera.GetRotation().y, m_Camera.GetRotation().z);
//
//        // Сцена — список энтити
//        if (ImGui::CollapsingHeader("Scene", ImGuiTreeNodeFlags_DefaultOpen))
//        {
//            for (auto& entity : m_Scene.GetEntities())
//            {
//                ImGui::PushID(entity.GetName().c_str());
//                if (ImGui::TreeNode(entity.GetName().c_str()))
//                {
//                    auto& t = entity.GetTransform();
//                    ImGui::DragFloat3("Position", &t.Position.x, 0.1f);
//                    ImGui::DragFloat3("Rotation", &t.Rotation.x, 1.0f);
//                    ImGui::DragFloat3("Scale", &t.Scale.x, 0.01f);
//                    ImGui::TreePop();
//                }
//                ImGui::PopID();
//            }
//        }
//
//        if (ImGui::CollapsingHeader("VRAM Statistics", ImGuiTreeNodeFlags_DefaultOpen))
//        {
//            auto& device = Flux::Application::Get().GetDevice();
//            auto stats = device.GetMemoryStatistics();
//
//            ImGui::Text("=== Memory Allocations ===");
//            ImGui::Text("Allocation Count: %u", stats.AllocationCount);
//            ImGui::Text("Allocation Bytes: %.2f MB", stats.AllocationBytes / (1024.0f * 1024.0f));
//
//            ImGui::Spacing();
//            ImGui::Text("=== Usage & Budget ===");
//            float usagePercent = (stats.Usage * 100.0f) / stats.Budget;
//            ImGui::Text("Usage: %.2f MB / %.2f MB (%.1f%%)",
//                stats.Usage / (1024.0f * 1024.0f),
//                stats.Budget / (1024.0f * 1024.0f),
//                usagePercent);
//            ImGui::ProgressBar(usagePercent / 100.0f, ImVec2(200, 20),
//                std::to_string((int)usagePercent).append("%").c_str());
//
//            if (usagePercent > 90.0f)
//                ImGui::TextColored(ImVec4(1, 0, 0, 1), "WARNING: VRAM almost full!");
//            else if (usagePercent > 75.0f)
//                ImGui::TextColored(ImVec4(1, 1, 0, 1), "CAUTION: High VRAM usage");
//            else
//                ImGui::TextColored(ImVec4(0, 1, 0, 1), "OK: Normal VRAM usage");
//        }
//    }
//
//    void OnEvent(Flux::Event& event) override
//    {
//        Flux::EventDispatcher dispatcher(event);
//        dispatcher.Dispatch<Flux::MouseMovedEvent>(FL_BIND_EVENT_FN(ExampleLayer::OnMouseMoved));
//        dispatcher.Dispatch<Flux::MouseScrolledEvent>(FL_BIND_EVENT_FN(ExampleLayer::OnMouseScrolled));
//        dispatcher.Dispatch<Flux::KeyPressedEvent>(FL_BIND_EVENT_FN(ExampleLayer::OnKeyPressed));
//    }
//
//    void OnResize(uint32_t width, uint32_t height) override
//    {
//        auto* device = &Flux::Application::Get().GetDevice();
//        auto* swapchain = device->GetSwapchain();
//        const bool isMsaaEnabled = (MSAA_SAMPLES != Flux::SampleCount::x1);
//
//        m_SceneFramebuffers.clear();
//
//        // 1. Пересоздаем Depth
//        Flux::TextureSpec depthSpec{};
//        depthSpec.Width = width;
//        depthSpec.Height = height;
//        depthSpec.MipLevels = 1;
//        depthSpec.ImageFormat = Flux::Format::D32_SFLOAT;
//        depthSpec.Usage = Flux::TextureUsage::DepthStencil;
//        depthSpec.Samples = MSAA_SAMPLES;
//        m_DepthTexture = device->CreateTexture(depthSpec);
//
//        // 2. Пересоздаем MSAA Color буфер ТОЛЬКО если он нужен
//        if (isMsaaEnabled)
//        {
//            Flux::TextureSpec msaaSpec{};
//            msaaSpec.Width = width;
//            msaaSpec.Height = height;
//            msaaSpec.ImageFormat = swapchain->GetFormat();
//            msaaSpec.Usage = Flux::TextureUsage::Transient;
//            msaaSpec.MipLevels = 1;
//            msaaSpec.Samples = MSAA_SAMPLES;
//            m_MsaaColorTexture = device->CreateTexture(msaaSpec);
//        }
//
//        // 3. Заново собираем фреймбуферы
//        uint32_t imageCount = swapchain->GetImageCount();
//        m_SceneFramebuffers.reserve(imageCount);
//        for (uint32_t i = 0; i < imageCount; i++)
//        {
//            Flux::FramebufferSpec fbSpec{};
//            fbSpec.RenderPass = m_SceneRenderPass.get();
//            fbSpec.ResolveTarget = nullptr; // <--- ОБЯЗАТЕЛЬНО сбрасываем в nullptr по умолчанию
//
//            if (isMsaaEnabled)
//            {
//                fbSpec.ColorTargets = { m_MsaaColorTexture.get() };         // Рендерим в промежуточную
//                fbSpec.DepthTarget = m_DepthTexture.get();
//                fbSpec.ResolveTarget = swapchain->GetColorTarget(i);        // Резолвим в свопчейн
//            }
//            else
//            {
//                fbSpec.ColorTargets = { swapchain->GetColorTarget(i) };     // Рендерим прямо в свопчейн
//                fbSpec.DepthTarget = m_DepthTexture.get();
//                // ResolveTarget остается nullptr (не передаем)
//            }
//
//            fbSpec.Width = width;
//            fbSpec.Height = height;
//            m_SceneFramebuffers.push_back(device->CreateFramebuffer(fbSpec));
//        }
//    }
//
//private:
//    void ProcessKeyboard(float deltaTime)
//    {
//        float speed = m_CameraSpeed * deltaTime;
//        glm::vec3 movement(0.0f);
//
//        if (Flux::Input::IsKeyPressed(FL_KEY_W))          movement.z += 1.0f;
//        if (Flux::Input::IsKeyPressed(FL_KEY_S))          movement.z -= 1.0f;
//        if (Flux::Input::IsKeyPressed(FL_KEY_A))          movement.x -= 1.0f;
//        if (Flux::Input::IsKeyPressed(FL_KEY_D))          movement.x += 1.0f;
//        if (Flux::Input::IsKeyPressed(FL_KEY_SPACE))      movement.y += 1.0f;
//        if (Flux::Input::IsKeyPressed(FL_KEY_LEFT_SHIFT)) movement.y -= 1.0f;
//
//        if (glm::length(movement) > 0.0f)
//            movement = glm::normalize(movement);
//
//        glm::vec3 forward = m_Camera.GetForward();
//        glm::vec3 right = m_Camera.GetRight();
//        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
//
//        glm::vec3 pos = m_Camera.GetPosition();
//        pos += forward * movement.z * speed;
//        pos += right * movement.x * speed;
//        pos += up * movement.y * speed;
//        m_Camera.SetPosition(pos);
//    }
//
//    bool OnMouseMoved(Flux::MouseMovedEvent& e)
//    {
//        if (m_FirstMouse) {
//            m_LastX = e.GetX(); m_LastY = e.GetY();
//            m_FirstMouse = false;
//            return false;
//        }
//        float xOffset = (e.GetX() - m_LastX) * m_MouseSensitivity;
//        float yOffset = (m_LastY - e.GetY()) * m_MouseSensitivity;
//        m_LastX = e.GetX(); m_LastY = e.GetY();
//
//        glm::vec3 rotation = m_Camera.GetRotation();
//        rotation.y += xOffset;
//        rotation.x = glm::clamp(rotation.x + yOffset, -89.0f, 89.0f);
//        m_Camera.SetRotation(rotation);
//        return false;
//    }
//
//    bool OnMouseScrolled(Flux::MouseScrolledEvent& e)
//    {
//        m_Camera.SetFOV(glm::clamp(m_Camera.GetFOV() - e.GetYOffset(), 1.0f, 90.0f));
//        return false;
//    }
//
//    bool OnKeyPressed(Flux::KeyPressedEvent& e)
//    {
//        if (e.GetKeyCode() == FL_KEY_ESCAPE) 
//        {
//            static bool cursorLocked = true;
//            cursorLocked = !cursorLocked;
//            SetCursorMode(cursorLocked);
//        }
//        return false;
//    }
//
//    void SetCursorMode(bool locked)
//    {
//        GLFWwindow* w = static_cast<GLFWwindow*>(Flux::Application::Get().GetWindow().GetNativeWindow());
//        if (locked) 
//        {
//            glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
//            m_FirstMouse = true;
//        }
//        else 
//            glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
//    }
//
//private:
//    struct alignas(16) PointLight {
//        glm::vec4 Position;  // w не используется
//        glm::vec4 Color;     // w = intensity
//    };
//
//    struct alignas(16) GlobalUBO {
//        glm::mat4 view;
//        glm::mat4 projection;
//        glm::vec3 cameraPos;
//        int       lightCount;
//        PointLight lights[8];
//    };
//
//    struct PushConstantData {
//        glm::mat4 model;
//        glm::vec4 color;
//        float roughnessOverride; // -1 = не используем, 0..1 = override
//        float metallicOverride;
//        float _pad[2];
//    };
//
//    Flux::PerspectiveCamera m_Camera;
//    float m_CameraSpeed = 5.0f;
//    float m_MouseSensitivity = 0.1f;
//    float m_LastFrameTime = 0.0f;
//    float m_LastX = 0.0f, m_LastY = 0.0f;
//    bool  m_FirstMouse = true;
//
//    Flux::Scene m_Scene;
//
//    Flux::Scope<Flux::RHIBuffer>              m_GlobalUBOBuffer;
//    Flux::Scope<Flux::RHIDescriptorSetLayout> m_GlobalSetLayout;
//    Flux::Scope<Flux::RHIDescriptorSetLayout> m_TextureSetLayout;
//    Flux::Scope<Flux::RHIDescriptorSet>       m_GlobalDescriptorSet;
//
//    Flux::Scope<Flux::RHIShader>   m_VertShader;
//    Flux::Scope<Flux::RHIShader>   m_FragShader;
//    Flux::Scope<Flux::RHIPipeline> m_Pipeline;
//
//    Flux::Scope<Flux::RHITexture>                  m_MsaaColorTexture;
//    Flux::Scope<Flux::RHITexture>                  m_DepthTexture;
//    Flux::Scope<Flux::RHIRenderPass>               m_SceneRenderPass;
//    std::vector<Flux::Scope<Flux::RHIFramebuffer>> m_SceneFramebuffers;
//};

class Sandbox : public Flux::Application {
public:
    Sandbox() { PushLayer(new Flux::EditorLayer()); }
    ~Sandbox() {}
};

Flux::Application* Flux::CreateApplication()
{
    return new Sandbox();
}