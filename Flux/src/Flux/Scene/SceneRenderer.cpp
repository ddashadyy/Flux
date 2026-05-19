#include "flpch.h"
#include "SceneRenderer.h"

#include "Components.h"

#include <backends/imgui_impl_vulkan.h>

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

    // =========================================================================
    //  Init
    // =========================================================================

    void SceneRenderer::Init(RHIDevice& device)
    {
        m_Device = &device;
        m_Renderer = CreateScope<Renderer>(device);

        SamplerSpec samplerSpec{};
        samplerSpec.MagFilter = FilterMode::Linear;
        samplerSpec.MinFilter = FilterMode::Linear;
        samplerSpec.AddressU = AddressMode::ClampToEdge;
        samplerSpec.AddressV = AddressMode::ClampToEdge;
        m_DefaultSampler = m_Device->CreateSampler(samplerSpec);

        auto vertSpv = LoadSPIRV("C:/dev/Flux/SandBox/assets/shaders/shader.vert.spv");
        auto fragSpv = LoadSPIRV("C:/dev/Flux/SandBox/assets/shaders/shader.frag.spv");
        m_VertShader = m_Device->CreateShader(ShaderStage::Vertex, vertSpv);
        m_FragShader = m_Device->CreateShader(ShaderStage::Fragment, fragSpv);

        BuildGraph();
        BuildPipeline();
    }

    // =========================================================================
    //  BuildGraph
    // =========================================================================

    void SceneRenderer::BuildGraph()
    {
        m_RenderGraph.Reset();

        uint32_t w = (uint32_t)m_ViewportSize.x;
        uint32_t h = (uint32_t)m_ViewportSize.y;

        m_ColorMsaaHandle = m_RenderGraph.CreateTexture({
            .Name = "SceneColorMSAA",
            .ImageFormat = Format::R8G8B8A8_UNORM,
            .Width = w, .Height = h,
            .MipLevels = 1,
            .Samples = MSAA_SAMPLES,
            .Usage = RGTextureUsage::ColorAttachment,
            });

        m_DepthMsaaHandle = m_RenderGraph.CreateTexture({
            .Name = "SceneDepthMSAA",
            .ImageFormat = Format::D32_SFLOAT,
            .Width = w, .Height = h,
            .MipLevels = 1,
            .Samples = MSAA_SAMPLES,
            .Usage = RGTextureUsage::DepthAttachment,
            });

        m_ColorResolveHandle = m_RenderGraph.CreateTexture({
            .Name = "SceneColorResolve",
            .ImageFormat = Format::R8G8B8A8_UNORM,
            .Width = w, .Height = h,
            .MipLevels = 1,
            .Samples = SampleCount::x1,
            .Usage = RGTextureUsage::ColorAttachment
                         | RGTextureUsage::Sampled
                         | RGTextureUsage::ResolveTarget,
            });

        m_RenderGraph.AddPass("GeometryPass")
            .Write(m_ColorMsaaHandle)
            .WriteDepth(m_DepthMsaaHandle)
            .Resolve(m_ColorMsaaHandle, m_ColorResolveHandle)
            .Execute([this](RGExecuteContext& ctx) {
            ctx.CmdList->BeginRenderPass(
                ctx.RenderPass,
                ctx.Framebuffer,
                { 0.05f, 0.05f, 0.05f, 1.0f },
                1.0f, 0
            );

            ctx.CmdList->SetViewport(
                0.0f, 0.0f,
                m_ViewportSize.x, m_ViewportSize.y
            );
            ctx.CmdList->SetScissor(
                0, 0,
                (uint32_t)m_ViewportSize.x,
                (uint32_t)m_ViewportSize.y
            );

            m_Renderer->BeginScene(
                *ctx.CmdList,
                *m_Pipeline,
                *m_CurrentCamera,
                m_ViewportSize.x, m_ViewportSize.y
            );

            auto& registry = m_CurrentScene->GetRegistry();
            auto meshView = registry.view<TransformComponent, MeshComponent>(
                entt::exclude<DestroyFlag>);

            for (auto entity : meshView)
            {
                auto& transform = meshView.get<TransformComponent>(entity);
                auto& mesh = meshView.get<MeshComponent>(entity);
                if (mesh.Model)
                    m_Renderer->Submit(mesh.Model, transform.WorldMatrix);
            }

            m_Renderer->EndScene();
            ctx.CmdList->EndRenderPass();
                });

        m_RenderGraph.SetOutput(m_ColorResolveHandle);
        m_RenderGraph.Compile(*m_Device);

        RegisterImGuiTexture();
    }

    // =========================================================================
    //  BuildPipeline
    // =========================================================================

    void SceneRenderer::BuildPipeline()
    {
        RHIRenderPass* geometryRenderPass = m_RenderGraph.GetPassRenderPass("GeometryPass");
        FL_CORE_ASSERT(geometryRenderPass, "GeometryPass RenderPass not found");

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
        desc.RenderPass = geometryRenderPass;
        desc.Topology = PrimitiveTopology::TriangleList;
        desc.Samples = MSAA_SAMPLES;

        desc.PushConstants.Stage = ShaderStage::Vertex | ShaderStage::Fragment;
        desc.PushConstants.Offset = 0;
        desc.PushConstants.Size = sizeof(PushConstantData);

        desc.DescriptorSetLayouts = {
            m_Renderer->GetGlobalDescriptorSetLayout(),
            m_Renderer->GetTextureDescriptorSetLayout()
        };

        desc.DepthStencil.DepthTest = true;
        desc.DepthStencil.DepthWrite = true;
        desc.DepthStencil.DepthCompare = CompareOp::Less;

        desc.Rasterizer.Cull = CullMode::None;
        desc.Rasterizer.Fill = FillMode::Solid;
        desc.Rasterizer.Front = FrontFace::Clockwise;

        desc.Blend = BlendPreset::Opaque();
        desc.DebugName = "ScenePipeline";

        m_Pipeline = m_Device->CreatePipeline(desc);
    }

    // =========================================================================
    //  RegisterImGuiTexture
    // =========================================================================

    void SceneRenderer::RegisterImGuiTexture()
    {
        RHITexture* resolveTexture = m_RenderGraph.GetPhysicalTexture(m_ColorResolveHandle);
        FL_CORE_ASSERT(resolveTexture, "SceneRenderer: resolve texture is null");

        auto vkSampler = static_cast<VkSampler>(m_DefaultSampler->GetHandle());
        auto vkImageView = static_cast<VkImageView>(resolveTexture->GetNativeImageView());

        m_ViewportTextureID = (ImTextureID)ImGui_ImplVulkan_AddTexture(
            vkSampler,
            vkImageView,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );
    }

    // =========================================================================
    //  Render
    // =========================================================================

    void SceneRenderer::Render(RHICommandList* cmdList, Ref<Scene> scene, const PerspectiveCamera& camera)
    {
        m_CurrentScene = scene;
        m_CurrentCamera = &camera;

        ExtractLightsFromScene(scene);
        m_RenderGraph.Execute(cmdList);

        m_CurrentScene = nullptr;
        m_CurrentCamera = nullptr;
    }

    // =========================================================================
    //  OnResize
    // =========================================================================

    void SceneRenderer::OnResize(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0) return;
        if (width == (uint32_t)m_ViewportSize.x && height == (uint32_t)m_ViewportSize.y) return;

        m_Device->WaitIdle();
        m_ViewportSize = { (float)width, (float)height };

        m_RenderGraph.Resize(*m_Device, width, height);

        RegisterImGuiTexture();
    }

    // =========================================================================
    //  ExtractLightsFromScene
    // =========================================================================

    void SceneRenderer::ExtractLightsFromScene(Ref<Scene> scene)
    {
        m_Renderer->ClearLights();

        auto& registry = scene->GetRegistry();
        auto view = registry.view<TransformComponent, LightComponent>(
            entt::exclude<DestroyFlag>);

        for (auto entity : view)
        {
            auto& transform = view.get<TransformComponent>(entity);
            auto& light = view.get<LightComponent>(entity);

            if (light.Type == LightComponent::LightType::Point)
            {
                PointLight pl;
                pl.Position = glm::vec4(transform.Translation, 1.0f);
                pl.Color = glm::vec4(light.Color, light.Intensity);
                m_Renderer->AddPointLight(pl);
            }
        }
    }

} // namespace Flux