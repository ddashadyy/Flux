#include <Flux.h>
#include "imgui/imgui.h"
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_DISABLE_FAST_FLOAT
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

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

class ExampleLayer : public Flux::Layer {
public:
    ExampleLayer() : Layer("Example"), m_Camera(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f)
    {
        auto* device = &Flux::Application::Get().GetDevice();
        auto* swapchain = device->GetSwapchain();
        auto& window = Flux::Application::Get().GetWindow();

        std::vector<Flux::Vertex> vertices = {
            // Front
            {{ -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }},
            {{  0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f }},
            {{  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }},
            {{ -0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f }},
            // Back
            {{  0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f,-1.0f }, { 0.0f, 0.0f }},
            {{ -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f,-1.0f }, { 1.0f, 0.0f }},
            {{ -0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f,-1.0f }, { 1.0f, 1.0f }},
            {{  0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f,-1.0f }, { 0.0f, 1.0f }},
            // Left
            {{ -0.5f, -0.5f, -0.5f }, {-1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }},
            {{ -0.5f, -0.5f,  0.5f }, {-1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f }},
            {{ -0.5f,  0.5f,  0.5f }, {-1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }},
            {{ -0.5f,  0.5f, -0.5f }, {-1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f }},
            // Right
            {{  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }},
            {{  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f }},
            {{  0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }},
            {{  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f }},
            // Top
            {{ -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f }},
            {{  0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f }},
            {{  0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f }},
            {{ -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f }},
            // Bottom
            {{ -0.5f, -0.5f, -0.5f }, { 0.0f,-1.0f, 0.0f }, { 0.0f, 0.0f }},
            {{  0.5f, -0.5f, -0.5f }, { 0.0f,-1.0f, 0.0f }, { 1.0f, 0.0f }},
            {{  0.5f, -0.5f,  0.5f }, { 0.0f,-1.0f, 0.0f }, { 1.0f, 1.0f }},
            {{ -0.5f, -0.5f,  0.5f }, { 0.0f,-1.0f, 0.0f }, { 0.0f, 1.0f }},
        };

        std::vector<uint32_t> indices = {
             0,  1,  2,  2,  3,  0,
             4,  5,  6,  6,  7,  4,
             8,  9, 10, 10, 11,  8,
            12, 13, 14, 14, 15, 12,
            16, 17, 18, 18, 19, 16,
            20, 21, 22, 22, 23, 20,
        };

        CreateTexture();

        // --- Buffers ---
        Flux::BufferSpec vertexSpec{};
        vertexSpec.Size = sizeof(Flux::Vertex) * vertices.size();
        vertexSpec.Usage = Flux::BufferUsage::Vertex;
        vertexSpec.CpuVisible = true;
        m_VertexBuffer = device->CreateBuffer(vertexSpec);
        m_VertexBuffer->SetData(vertices.data(), vertexSpec.Size);

        Flux::BufferSpec indexSpec{};
        indexSpec.Size = sizeof(uint32_t) * indices.size();
        indexSpec.Usage = Flux::BufferUsage::Index;
        indexSpec.CpuVisible = true;
        m_IndexBuffer = device->CreateBuffer(indexSpec);
        m_IndexBuffer->SetData(indices.data(), indexSpec.Size);

        Flux::BufferSpec uniformSpec{};
        uniformSpec.Size = sizeof(UBO);
        uniformSpec.Usage = Flux::BufferUsage::Uniform;
        uniformSpec.CpuVisible = true;
        m_UniformBuffer = device->CreateBuffer(uniformSpec);

        // --- Shaders ---
        auto vertSpirv = ReadSPV("C:\\dev\\Flux\\SandBox\\assets\\shaders\\shader.vert.spv");
        auto fragSpirv = ReadSPV("C:\\dev\\Flux\\SandBox\\assets\\shaders\\shader.frag.spv");
        m_VertShader = device->CreateShader(Flux::ShaderStage::Vertex, vertSpirv);
        m_FragShader = device->CreateShader(Flux::ShaderStage::Fragment, fragSpirv);

        // --- Descriptor set ---
        Flux::DescriptorSetLayoutDesc layoutDesc{};
        layoutDesc.Bindings.emplace_back(0, Flux::DescriptorType::UniformBuffer, Flux::ShaderStage::Vertex, 1);
        layoutDesc.Bindings.emplace_back(1, Flux::DescriptorType::CombinedImageSampler, Flux::ShaderStage::Fragment, 1);
        m_DescriptorSetLayout = device->CreateDescriptorSetLayout(layoutDesc);

        m_DescriptorSet = device->CreateDescriptorSet(m_DescriptorSetLayout.get());
        m_DescriptorSet->BindBuffer(0, m_UniformBuffer.get());
        m_DescriptorSet->BindTexture(1, m_Texture.get());
        m_DescriptorSet->Update();

        // --- Depth texture ---
        Flux::TextureSpec depthSpec{};
        depthSpec.Width = window.GetWidth();
        depthSpec.Height = window.GetHeight();
        depthSpec.MipLevels = 1;
        depthSpec.ImageFormat = Flux::Format::D32_SFLOAT;
        depthSpec.Usage = Flux::TextureUsage::DepthStencil;
        m_DepthTexture = device->CreateTexture(depthSpec);

        // --- Scene render pass (color + depth) ---
        Flux::RenderPassDesc rpDesc{};
        rpDesc.ColorFormats = { swapchain->GetFormat() };
        rpDesc.HasDepth = true;
        rpDesc.DepthFormat = Flux::Format::D32_SFLOAT;
        rpDesc.ColorLoadOp = Flux::AttachmentLoadOp::Clear;
        rpDesc.ColorStoreOp = Flux::AttachmentStoreOp::Store;
        rpDesc.DepthLoadOp = Flux::AttachmentLoadOp::Clear;
        rpDesc.DepthStoreOp = Flux::AttachmentStoreOp::DontCare;
        rpDesc.ColorInitialLayout = Flux::ImageLayout::Undefined;
        rpDesc.ColorFinalLayout = Flux::ImageLayout::ColorAttachment; 
        m_SceneRenderPass = device->CreateRenderPass(rpDesc);

        // --- Framebuffer per swapchain image ---
        uint32_t imageCount = swapchain->GetImageCount();
        m_SceneFramebuffers.resize(imageCount);
        for (uint32_t i = 0; i < imageCount; i++)
        {
            Flux::FramebufferSpec fbSpec{};
            fbSpec.RenderPass = m_SceneRenderPass.get();
            fbSpec.ColorTargets = { swapchain->GetColorTarget(i) };
            fbSpec.DepthTarget = m_DepthTexture.get();
            fbSpec.Width = window.GetWidth();
            fbSpec.Height = window.GetHeight();
            m_SceneFramebuffers[i] = device->CreateFramebuffer(fbSpec);
        }

        // --- Pipeline ---
        Flux::BufferLayout vertexLayout = {
            { Flux::ShaderDataType::Float3 },
            { Flux::ShaderDataType::Float3 },
            { Flux::ShaderDataType::Float2 },
        };

        Flux::PipelineLayoutDesc pushConstantDesc;
        pushConstantDesc.Stage = Flux::ShaderStage::Vertex | Flux::ShaderStage::Fragment;
        pushConstantDesc.Offset = 0;
        pushConstantDesc.Size = sizeof(PushConstantData);

        Flux::PipelineDesc pipelineDesc{};
        pipelineDesc.VertexShader = m_VertShader.get();
        pipelineDesc.FragmentShader = m_FragShader.get();
        pipelineDesc.VertexLayout = vertexLayout;
        pipelineDesc.pipelineLayoutDesc = pushConstantDesc;
        pipelineDesc.DescriptorSetLayout = m_DescriptorSetLayout.get();
        pipelineDesc.RenderPass = m_SceneRenderPass.get();
        pipelineDesc.Topology = Flux::PrimitiveTopology::TriangleList;
        pipelineDesc.DepthStencil.DepthTest = true;
        pipelineDesc.DepthStencil.DepthWrite = true;

        m_Pipeline = device->CreatePipeline(pipelineDesc);

        // --- Camera ---
        m_Camera.SetPosition(glm::vec3(0.0f, 0.0f, 3.0f));
        m_Camera.SetRotation(glm::vec3(0.0f, -90.0f, 0.0f));

        SetCursorMode(true);

    }

    void OnUpdate(Flux::RHICommandList* cmdList) override
    {

        auto* swapchain = Flux::Application::Get().GetDevice().GetSwapchain();
        uint32_t imageIndex = swapchain->GetCurrentImageIndex();

        float currentTime = (float)glfwGetTime();
        float deltaTime = currentTime - m_LastFrameTime;
        m_LastFrameTime = currentTime;

        ProcessKeyboard(deltaTime);

        auto& window = Flux::Application::Get().GetWindow();
        float aspect = (float)window.GetWidth() / (float)window.GetHeight();

        UBO globalUBO{};
        globalUBO.view = m_Camera.GetViewMatrix();
        globalUBO.projection = glm::perspective(glm::radians(m_Camera.GetFOV()), aspect, 0.1f, 100.0f);
        globalUBO.projection[1][1] *= -1;

        m_UniformBuffer->SetData(&globalUBO, sizeof(UBO));

        cmdList->BeginRenderPass(m_SceneRenderPass.get(), m_SceneFramebuffers[imageIndex].get());
        cmdList->SetViewport(0, 0, (float)window.GetWidth(), (float)window.GetHeight());
        cmdList->SetScissor(0, 0, window.GetWidth(), window.GetHeight());
        cmdList->SetPipeline(m_Pipeline.get());
        cmdList->BindDescriptorSet(m_DescriptorSet.get());
        cmdList->BindVertexBuffer(m_VertexBuffer.get());
        cmdList->BindIndexBuffer(m_IndexBuffer.get());

        const int cubeCount = 10;

        for (int i = 0; i < cubeCount; ++i) {
            PushConstantData pushData{};

            float diagonalOffset = static_cast<float>(i) * 1.5f;
            glm::vec3 position(
                diagonalOffset - (cubeCount * 1.5f / 2.0f), 
                sin(currentTime + i) * 0.5f,
                -5.0f - diagonalOffset * 0.5f
            );

            pushData.model = glm::translate(glm::mat4(1.0f), position);
            float rotationAngle = currentTime * 0.8f + static_cast<float>(i) * 0.5f;
            pushData.model = glm::rotate(pushData.model, rotationAngle, glm::vec3(0.5f, 1.0f, 0.3f));

            float t = (cubeCount > 1) ? static_cast<float>(i) / static_cast<float>(cubeCount - 1) : 0.0f;
            pushData.color = glm::vec4(0.5f + t * 0.5f, 0.3f + t * 0.7f, 1.0f - t * 0.5f, 1.0f);
            pushData.timeOffset = currentTime + static_cast<float>(i);

            cmdList->PushConstants(m_Pipeline.get(), &pushData);
            cmdList->DrawIndexed(36, 1);
        }

        cmdList->EndRenderPass();        
    }

    void OnImGuiRender() override
    {
        ImGui::Text("Renderer API: Vulkan");

        ImGui::Text("Camera Position: (x = %.2f, y = %.2f, z = %.2f)",
            m_Camera.GetPosition().x,
            m_Camera.GetPosition().y,
            m_Camera.GetPosition().z);
        ImGui::Text("Camera Rotation: (x = %.2f, y = %.2f, z = %.2f)",
            m_Camera.GetRotation().x,
            m_Camera.GetRotation().y,
            m_Camera.GetRotation().z);

        if (ImGui::CollapsingHeader("VRAM Statistics", ImGuiTreeNodeFlags_DefaultOpen))
        {
            auto& device = Flux::Application::Get().GetDevice();
            auto stats = device.GetMemoryStatistics();

            ImGui::Text("=== Memory Allocations ===");
            ImGui::Text("Allocation Count: %u", stats.AllocationCount);
            ImGui::Text("Allocation Bytes: %.2f MB (%.2f GB)",
                stats.AllocationBytes / (1024.0f * 1024.0f),
                stats.AllocationBytes / (1024.0f * 1024.0f * 1024.0f));

            ImGui::Spacing();
            ImGui::Text("=== Memory Blocks ===");
            ImGui::Text("Block Count: %u", stats.BlockCount);
            ImGui::Text("Block Bytes: %.2f MB (%.2f GB)",
                stats.BlockBytes / (1024.0f * 1024.0f),
                stats.BlockBytes / (1024.0f * 1024.0f * 1024.0f));

            ImGui::Spacing();
            ImGui::Text("=== Usage & Budget ===");
            ImGui::Text("Usage: %.2f MB / %.2f MB (%.1f%%)",
                stats.Usage / (1024.0f * 1024.0f),
                stats.Budget / (1024.0f * 1024.0f),
                (stats.Usage * 100.0f) / stats.Budget);

            float usagePercent = (stats.Usage * 100.0f) / stats.Budget;
            ImGui::ProgressBar(usagePercent / 100.0f, ImVec2(200, 20),
                std::to_string((int)usagePercent).append("%").c_str());

            if (usagePercent > 90.0f)
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "WARNING: VRAM almost full!");
            else if (usagePercent > 75.0f)
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "CAUTION: High VRAM usage");
            else
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "OK: Normal VRAM usage");
        }
    }

    void OnEvent(Flux::Event& event) override
    {
        Flux::EventDispatcher dispatcher(event);
        dispatcher.Dispatch<Flux::MouseMovedEvent>(FL_BIND_EVENT_FN(ExampleLayer::OnMouseMoved));
        dispatcher.Dispatch<Flux::MouseScrolledEvent>(FL_BIND_EVENT_FN(ExampleLayer::OnMouseScrolled));
        dispatcher.Dispatch<Flux::KeyPressedEvent>(FL_BIND_EVENT_FN(ExampleLayer::OnKeyPressed));
    }

    void OnResize(uint32_t width, uint32_t height) override
    {
        auto* device = &Flux::Application::Get().GetDevice();
        auto* swapchain = device->GetSwapchain();

        Flux::TextureSpec depthSpec{};
        depthSpec.Width = width;
        depthSpec.Height = height;
        depthSpec.MipLevels = 1;
        depthSpec.ImageFormat = Flux::Format::D32_SFLOAT;
        depthSpec.Usage = Flux::TextureUsage::DepthStencil;
        m_DepthTexture = device->CreateTexture(depthSpec);

        uint32_t imageCount = swapchain->GetImageCount();
        m_SceneFramebuffers.resize(imageCount);
        for (uint32_t i = 0; i < imageCount; i++)
        {
            Flux::FramebufferSpec fbSpec{};
            fbSpec.RenderPass = m_SceneRenderPass.get();
            fbSpec.ColorTargets = { swapchain->GetColorTarget(i) };
            fbSpec.DepthTarget = m_DepthTexture.get();
            fbSpec.Width = width;
            fbSpec.Height = height;
            m_SceneFramebuffers[i] = device->CreateFramebuffer(fbSpec);
        }
    }

private:
    void CreateTexture()
    {
        int width, height, channels;
        stbi_uc* data = stbi_load("C:\\dev\\Flux\\SandBox\\assets\\textures\\diamond.png",
            &width, &height, &channels, 4);

        if (!data)
        {
            FL_CORE_ERROR("Failed to load texture image!");
            return;
        }

        Flux::TextureSpec spec{};
        spec.Width = width;
        spec.Height = height;
        spec.MipLevels = 1;
        spec.ImageFormat = Flux::Format::R8G8B8A8_UNORM;
        spec.Usage = Flux::TextureUsage::Sampled;

        m_Texture = Flux::Application::Get().GetDevice().CreateTexture(spec);
        m_Texture->SetData(data, width * height * 4);

        stbi_image_free(data);
    }

    void ProcessKeyboard(float deltaTime)
    {
        float speed = m_CameraSpeed * deltaTime;
        glm::vec3 movement(0.0f);

        if (Flux::Input::IsKeyPressed(GLFW_KEY_W))          movement.z += 1.0f;
        if (Flux::Input::IsKeyPressed(GLFW_KEY_S))          movement.z -= 1.0f;
        if (Flux::Input::IsKeyPressed(GLFW_KEY_A))          movement.x -= 1.0f;
        if (Flux::Input::IsKeyPressed(GLFW_KEY_D))          movement.x += 1.0f;
        if (Flux::Input::IsKeyPressed(GLFW_KEY_SPACE))      movement.y += 1.0f;
        if (Flux::Input::IsKeyPressed(GLFW_KEY_LEFT_SHIFT)) movement.y -= 1.0f;

        if (glm::length(movement) > 0.0f)
            movement = glm::normalize(movement);

        glm::mat4 view = m_Camera.GetViewMatrix();
        glm::vec3 forward = glm::normalize(glm::vec3(-view[2][0], -view[2][1], -view[2][2]));
        glm::vec3 right = glm::normalize(glm::vec3(view[0][0], view[0][1], view[0][2]));
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        glm::vec3 newPosition = m_Camera.GetPosition();
        newPosition += forward * movement.z * speed;
        newPosition += right * movement.x * speed;
        newPosition += up * movement.y * speed;

        m_Camera.SetPosition(newPosition);
    }

    bool OnMouseMoved(Flux::MouseMovedEvent& e)
    {
        if (m_FirstMouse)
        {
            m_LastX = e.GetX();
            m_LastY = e.GetY();
            m_FirstMouse = false;
            return false;
        }

        float xOffset = (e.GetX() - m_LastX) * m_MouseSensitivity;
        float yOffset = (m_LastY - e.GetY()) * m_MouseSensitivity;
        m_LastX = e.GetX();
        m_LastY = e.GetY();

        glm::vec3 rotation = m_Camera.GetRotation();
        rotation.y += xOffset;
        rotation.x += yOffset;
        rotation.x = glm::clamp(rotation.x, -89.0f, 89.0f);

        m_Camera.SetRotation(rotation);
        return false;
    }

    bool OnMouseScrolled(Flux::MouseScrolledEvent& e)
    {
        float fov = glm::clamp(m_Camera.GetFOV() - e.GetYOffset(), 1.0f, 90.0f);
        m_Camera.SetFOV(fov);
        return false;
    }

    bool OnKeyPressed(Flux::KeyPressedEvent& e)
    {
        if (e.GetKeyCode() == GLFW_KEY_ESCAPE)
        {
            static bool cursorLocked = true;
            cursorLocked = !cursorLocked;
            SetCursorMode(cursorLocked);
        }
        return false;
    }

    void SetCursorMode(bool locked)
    {
        GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(
            Flux::Application::Get().GetWindow().GetNativeWindow());

        if (locked)
        {
            glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            m_FirstMouse = true;
        }
        else
        {
            glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

private:
    struct UBO {
        //alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 projection;
    };

    struct PushConstantData {
        glm::mat4 model;
        glm::vec4 color;
        alignas(16) float timeOffset;
    };

    Flux::PerspectiveCamera m_Camera;
    float m_CameraSpeed = 5.0f;
    float m_MouseSensitivity = 0.1f;
    float m_LastFrameTime = 0.0f;
    float m_LastX = 0.0f;
    float m_LastY = 0.0f;
    bool  m_FirstMouse = true;

    // Geometry & material
    Flux::Scope<Flux::RHIBuffer>              m_VertexBuffer;
    Flux::Scope<Flux::RHIBuffer>              m_IndexBuffer;
    Flux::Scope<Flux::RHIBuffer>              m_UniformBuffer;
    Flux::Scope<Flux::RHIShader>              m_VertShader;
    Flux::Scope<Flux::RHIShader>              m_FragShader;
    Flux::Scope<Flux::RHIDescriptorSetLayout> m_DescriptorSetLayout;
    Flux::Scope<Flux::RHIDescriptorSet>       m_DescriptorSet;
    Flux::Scope<Flux::RHITexture>             m_Texture;
    Flux::Scope<Flux::RHIPipeline>            m_Pipeline;

    // Render pass & framebuffers
    Flux::Scope<Flux::RHITexture>                  m_DepthTexture;
    Flux::Scope<Flux::RHIRenderPass>               m_SceneRenderPass;
    std::vector<Flux::Scope<Flux::RHIFramebuffer>> m_SceneFramebuffers;
};

class Sandbox : public Flux::Application {
public:
    Sandbox() { PushLayer(new ExampleLayer()); }
    ~Sandbox() {}
};

Flux::Application* Flux::CreateApplication()
{
    return new Sandbox();
}