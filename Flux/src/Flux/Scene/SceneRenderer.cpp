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

        CreateOffscreenResources(1280, 720);
        CreatePipeline();
    }

    void SceneRenderer::ExtractLightsFromScene(Ref<Scene> scene)
    {
        m_Renderer->ClearLights();

        auto& registry = scene->GetRegistry();
        auto view = registry.view<TransformComponent, LightComponent>(entt::exclude<DestroyFlag>);

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
            // else if (Directional / Spot) ...
        }
    }

    void SceneRenderer::Render(RHICommandList* cmdList, Ref<Scene> scene, const PerspectiveCamera& camera)
    {
        ExtractLightsFromScene(scene);

        cmdList->BeginRenderPass(
            m_GeometryPass.get(),
            m_GeometryFramebuffer.get(),
            { 0.05f, 0.05f, 0.05f, 1.0f },
            1.0f, 0
        );

        cmdList->SetViewport(0.0f, 0.0f, m_ViewportSize.x, m_ViewportSize.y);
        cmdList->SetScissor(0, 0, (uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

        m_Renderer->BeginScene(*cmdList, *m_Pipeline, camera, m_ViewportSize.x, m_ViewportSize.y);

        auto& registry = scene->GetRegistry();

        auto meshView = registry.view<TransformComponent, MeshComponent>(entt::exclude<DestroyFlag>);

        for (auto entity : meshView)
        {
            auto& transform = meshView.get<TransformComponent>(entity);
            auto& mesh = meshView.get<MeshComponent>(entity);

            if (mesh.Model) 
            {
                m_Renderer->Submit(mesh.Model, transform.WorldMatrix);
            }
        }

        m_Renderer->EndScene();
        cmdList->EndRenderPass();
    }

    void SceneRenderer::OnResize(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0 || (width == (uint32_t)m_ViewportSize.x && height == (uint32_t)m_ViewportSize.y))
            return;

        m_Device->WaitIdle();

        m_ViewportSize = { (float)width, (float)height };

        m_GeometryFramebuffer.reset();
        m_GeometryPass.reset();
        m_ColorMsaa.reset();
        m_DepthMsaa.reset();
        m_ColorResolve.reset();

        CreateOffscreenResources(width, height);
    }

    void SceneRenderer::CreateOffscreenResources(uint32_t width, uint32_t height)
    {
        // MSAA Color
        TextureSpec colorMsaaSpec{};
        colorMsaaSpec.Width = width;
        colorMsaaSpec.Height = height;
        colorMsaaSpec.MipLevels = 1;
        colorMsaaSpec.ImageFormat = Format::R8G8B8A8_UNORM;
        colorMsaaSpec.Usage = TextureUsage::RenderTarget;
        colorMsaaSpec.Samples = MSAA_SAMPLES;
        colorMsaaSpec.DebugName = "OffscreenColorMSAA";
        m_ColorMsaa = m_Device->CreateTexture(colorMsaaSpec);

        // MSAA Depth
        TextureSpec depthMsaaSpec{};
        depthMsaaSpec.Width = width;
        depthMsaaSpec.Height = height;
        depthMsaaSpec.MipLevels = 1;
        depthMsaaSpec.ImageFormat = Format::D32_SFLOAT;
        depthMsaaSpec.Usage = TextureUsage::DepthStencil;
        depthMsaaSpec.Samples = MSAA_SAMPLES;
        depthMsaaSpec.DebugName = "OffscreenDepthMSAA";
        m_DepthMsaa = m_Device->CreateTexture(depthMsaaSpec);

        // Resolve Color (x1)
        TextureSpec resolveSpec{};
        resolveSpec.Width = width;
        resolveSpec.Height = height;
        resolveSpec.MipLevels = 1;
        resolveSpec.ImageFormat = Format::R8G8B8A8_UNORM;
        resolveSpec.Usage = TextureUsage::RenderTarget | TextureUsage::Sampled;
        resolveSpec.Samples = SampleCount::x1;
        resolveSpec.DebugName = "OffscreenColorResolve";
        m_ColorResolve = m_Device->CreateTexture(resolveSpec);

        // Render Pass (Resolve)
        RenderPassDesc desc{};
        desc.ColorFormats = { Format::R8G8B8A8_UNORM };
        desc.HasDepth = true;
        desc.DepthFormat = Format::D32_SFLOAT;
        desc.Samples = MSAA_SAMPLES;

        desc.ColorLoadOp = AttachmentLoadOp::Clear;
        desc.ColorStoreOp = AttachmentStoreOp::DontCare;
        desc.DepthLoadOp = AttachmentLoadOp::Clear;
        desc.DepthStoreOp = AttachmentStoreOp::DontCare;

        desc.HasResolve = true;
        desc.ColorInitialLayout = ImageLayout::Undefined;
        desc.ColorFinalLayout = ImageLayout::ColorAttachment;

        desc.ResolveInitialLayout = ImageLayout::Undefined;
        desc.ResolveFinalLayout = ImageLayout::ShaderReadOnly;

        desc.DepthInitialLayout = ImageLayout::Undefined;
        desc.DepthFinalLayout = ImageLayout::DepthStencilAttachment;

        m_GeometryPass = m_Device->CreateRenderPass(desc);

        FramebufferSpec fbSpec{};
        fbSpec.RenderPass = m_GeometryPass.get();
        fbSpec.Width = width;
        fbSpec.Height = height;
        fbSpec.ColorTargets = { m_ColorMsaa.get() };
        fbSpec.DepthTarget = m_DepthMsaa.get();
        fbSpec.ResolveTarget = m_ColorResolve.get();
        m_GeometryFramebuffer = m_Device->CreateFramebuffer(fbSpec);

        auto vkSampler = static_cast<VkSampler>(m_DefaultSampler->GetHandle());
        auto vkImageView = static_cast<VkImageView>(m_ColorResolve->GetNativeImageView());
        m_ViewportTextureID = (ImTextureID)ImGui_ImplVulkan_AddTexture(
            vkSampler,
            vkImageView,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );
    }

    void SceneRenderer::CreatePipeline()
    {
        auto vertSpv = LoadSPIRV("C:/dev/Flux/SandBox/assets/shaders/shader.vert.spv");
        auto fragSpv = LoadSPIRV("C:/dev/Flux/SandBox/assets/shaders/shader.frag.spv");
        m_VertShader = m_Device->CreateShader(ShaderStage::Vertex, vertSpv);
        m_FragShader = m_Device->CreateShader(ShaderStage::Fragment, fragSpv);

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
        desc.RenderPass = m_GeometryPass.get();
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
}