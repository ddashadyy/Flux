#include "flpch.h"
#include "Renderer.h"

#include "Flux/Renderer/Geometry.h"
#include "Flux/Renderer/Camera.h"
#include "Flux/Core/Application.h"
#include "Flux/Renderer/PerspectiveCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Flux {

    Renderer::Renderer(RHIDevice& device)
        : m_Device(device)
    {
        // Set 0: GlobalUBO (камера + свет)
        DescriptorSetLayoutDesc globalLayoutDesc{};
        globalLayoutDesc.Bindings.emplace_back(
            0,
            DescriptorType::UniformBuffer,
            ShaderStage::Vertex | ShaderStage::Fragment,
            1
        );
        m_GlobalDescriptorSetLayout = m_Device.CreateDescriptorSetLayout(globalLayoutDesc);

        // Set 1: текстуры (albedo, normal, roughnessMetallic)
        DescriptorSetLayoutDesc textureLayoutDesc{};
        textureLayoutDesc.Bindings.emplace_back(0, DescriptorType::CombinedImageSampler, ShaderStage::Fragment, 1);
        textureLayoutDesc.Bindings.emplace_back(1, DescriptorType::CombinedImageSampler, ShaderStage::Fragment, 1);
        textureLayoutDesc.Bindings.emplace_back(2, DescriptorType::CombinedImageSampler, ShaderStage::Fragment, 1);
        m_TextureDescriptorSetLayout = m_Device.CreateDescriptorSetLayout(textureLayoutDesc);

        // GlobalUBO буфер — CpuVisible, обновляется каждый кадр
        BufferSpec uboSpec{};
        uboSpec.Size = sizeof(GlobalUBO);
        uboSpec.Usage = BufferUsage::Uniform;
        uboSpec.CpuVisible = true;
        m_GlobalUBO = m_Device.CreateBuffer(uboSpec);

        // Descriptor set для GlobalUBO
        m_GlobalDescriptorSet = m_Device.CreateDescriptorSet(m_GlobalDescriptorSetLayout.get());
        m_GlobalDescriptorSet->BindBuffer(0, m_GlobalUBO.get());
        m_GlobalDescriptorSet->Update();
    }

    void Renderer::BeginScene(RHICommandList& cmd,
        RHIPipeline& pipeline,
        const PerspectiveCamera& camera,
        const DirectionalLight& light)
    {
        m_CommandList = &cmd;
        m_Pipeline = &pipeline;

        auto& window = Application::Get().GetWindow();
        float aspect = (float)window.GetWidth() / (float)window.GetHeight();

        GlobalUBO ubo{};
        ubo.View = camera.GetViewMatrix();
        ubo.Projection = glm::perspective(glm::radians(camera.GetFOV()), aspect, 0.1f, 1000.0f);
        ubo.Projection[1][1] *= -1; // Vulkan Y flip
        ubo.CameraPos = glm::vec4(camera.GetPosition(), 1.0f);
        ubo.Light = light;

        m_GlobalUBO->SetData(&ubo, sizeof(GlobalUBO));

		m_CommandList->SetPipeline(m_Pipeline);
        m_CommandList->BindDescriptorSet(0, m_GlobalDescriptorSet.get(), m_Pipeline);
    }

    void Renderer::Submit(const Entity& entity)
    {
        auto model = entity.GetModel();
        if (!model) return;

        m_CommandList->BindVertexBuffer(model->VertexBuffer.get()); 

        for (const auto& subMesh : model->Meshes)
        {
            m_CommandList->BindDescriptorSet(1, subMesh.Mat.DescriptorSet.get(), m_Pipeline);

            PushConstantData push{};
            push.Model = entity.GetTransform().GetMatrix();
            push.Color = subMesh.Mat.Color;
            push.RoughnessOverride = subMesh.Mat.RoughnessOverride;
            push.MetallicOverride = subMesh.Mat.MetallicOverride;

            m_CommandList->PushConstants(
                m_Pipeline,
                ShaderStage::Vertex | ShaderStage::Fragment,
                0,
                sizeof(PushConstantData),
                &push
            );

            subMesh.Draw(*m_CommandList);
        }
    }

    void Renderer::EndScene()
    {
        m_CommandList = nullptr;
        m_Pipeline = nullptr;
    }

}