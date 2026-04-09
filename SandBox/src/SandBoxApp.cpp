#include <Flux.h>
#include "imgui/imgui.h"
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <fstream>

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
    ExampleLayer() : Layer("Example")
    {
        auto* device = &Flux::Application::Get().GetDevice();


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

        // буферы
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

        // шейдеры
        FL_CORE_INFO("Buffers created");

        auto vertSpirv = ReadSPV("C:\\dev\\Flux\\SandBox\\assets\\shaders\\shader.vert.spv");
        FL_CORE_INFO("Vert SPV loaded, size: {0}", vertSpirv.size());

        auto fragSpirv = ReadSPV("C:\\dev\\Flux\\SandBox\\assets\\shaders\\shader.frag.spv");
        FL_CORE_INFO("Frag SPV loaded, size: {0}", fragSpirv.size());

        m_VertShader = device->CreateShader(Flux::ShaderStage::Vertex, vertSpirv);
        m_FragShader = device->CreateShader(Flux::ShaderStage::Fragment, fragSpirv);

        // descriptor set layout
        Flux::DescriptorSetLayoutDesc layoutDesc{};
        layoutDesc.Bindings.push_back({ 0, Flux::DescriptorType::UniformBuffer, Flux::ShaderStage::Vertex, 1 });
        m_DescriptorSetLayout = device->CreateDescriptorSetLayout(layoutDesc);

        // descriptor set
        m_DescriptorSet = device->CreateDescriptorSet(m_DescriptorSetLayout.get());
        m_DescriptorSet->BindBuffer(0, m_UniformBuffer.get());
        m_DescriptorSet->Update();

        // render pass
        Flux::RenderPassDesc rpDesc{};
        rpDesc.ColorFormats = { Flux::Format::B8G8R8A8_UNORM };
        rpDesc.HasDepth = false;
        m_RenderPass = device->CreateRenderPass(rpDesc);

        // pipeline
        Flux::BufferLayout vertexLayout = {
            { Flux::ShaderDataType::Float3 }, // Position
            { Flux::ShaderDataType::Float3 }, // Normal
            { Flux::ShaderDataType::Float2 }, // TexCoord
        };

        Flux::PipelineDesc pipelineDesc{};
        pipelineDesc.VertexShader = m_VertShader.get();
        pipelineDesc.FragmentShader = m_FragShader.get();
        pipelineDesc.VertexLayout = vertexLayout;
        pipelineDesc.DescriptorSetLayout = m_DescriptorSetLayout.get();
        pipelineDesc.RenderPass = m_RenderPass.get();
        pipelineDesc.Topology = Flux::PrimitiveTopology::TriangleList;
        pipelineDesc.DepthStencil.DepthTest = false;
        pipelineDesc.DepthStencil.DepthWrite = false;
        m_Pipeline = device->CreatePipeline(pipelineDesc);
    }

    void OnUpdate() override
    {
        auto* device = &Flux::Application::Get().GetDevice();
        auto* cmdList = device->GetCommandList();

        // обновляем UBO
        UBO ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f),
            (float)glfwGetTime(),
            glm::vec3(0.5f, 1.0f, 0.0f));
        ubo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));
        ubo.projection = glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f);
        ubo.projection[1][1] *= -1; // Vulkan Y flip

        m_UniformBuffer->SetData(&ubo, sizeof(UBO));

        // рендер
        cmdList->BeginRenderPass(m_RenderPass.get());
        cmdList->SetPipeline(m_Pipeline.get());
        cmdList->BindDescriptorSet(m_DescriptorSet.get());
        cmdList->BindVertexBuffer(m_VertexBuffer.get());
        cmdList->BindIndexBuffer(m_IndexBuffer.get());
        cmdList->DrawIndexed(36, 1);
        cmdList->EndRenderPass();
    }

    void OnImGuiRender() override
    {
        ImGui::Text("Flux Engine - Cube");
        ImGui::Text("Backend: Vulkan");
    }

private:
    struct UBO {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 projection;
    };

    Flux::Scope<Flux::RHIBuffer>              m_VertexBuffer;
    Flux::Scope<Flux::RHIBuffer>              m_IndexBuffer;
    Flux::Scope<Flux::RHIBuffer>              m_UniformBuffer;
    Flux::Scope<Flux::RHIShader>              m_VertShader;
    Flux::Scope<Flux::RHIShader>              m_FragShader;
    Flux::Scope<Flux::RHIDescriptorSetLayout> m_DescriptorSetLayout;
    Flux::Scope<Flux::RHIDescriptorSet>       m_DescriptorSet;
    Flux::Scope<Flux::RHIRenderPass>          m_RenderPass;
    Flux::Scope<Flux::RHIPipeline>            m_Pipeline;
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