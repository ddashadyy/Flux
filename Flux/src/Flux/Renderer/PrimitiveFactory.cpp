#include "flpch.h"
#include "PrimitiveFactory.h"

#include "Flux/Core/Application.h"

namespace Flux {

    Ref<Texture>      PrimitiveFactory::s_DefaultWhite = nullptr;
    Ref<Texture>      PrimitiveFactory::s_DefaultNormal = nullptr;
    Scope<RHISampler> PrimitiveFactory::s_Sampler = nullptr;

    // -------------------------------------------------------------------------
    // Внутренний хелпер: загрузка 1x1 текстуры через staging
    // -------------------------------------------------------------------------
    static Ref<Texture> CreateSinglePixelTexture(RHIDevice& device, uint8_t r, uint8_t g, uint8_t b, uint8_t a, const char* debugName)
    {
        TextureSpec spec{};
        spec.Width = 1;
        spec.Height = 1;
        spec.Depth = 1;
        spec.MipLevels = 1;
        spec.GenerateMipmaps = false;
        spec.ImageFormat = Format::R8G8B8A8_UNORM;
        spec.Usage = TextureUsage::Sampled | TextureUsage::TransferDst;
        spec.Type = TextureType::Texture2D;
        spec.DebugName = debugName;

        auto tex = device.CreateTexture(spec);
        uint8_t pixel[4] = { r, g, b, a };
        tex->SetData(pixel, sizeof(pixel));
        return tex;
    }

    Ref<Texture> PrimitiveFactory::GetDefaultWhite(RHIDevice& device)
    {
        if (!s_DefaultWhite)
            s_DefaultWhite = CreateSinglePixelTexture(device, 255, 255, 255, 255, "Primitive_White");
        return s_DefaultWhite;
    }

    Ref<Texture> PrimitiveFactory::GetDefaultNormal(RHIDevice& device)
    {
        if (!s_DefaultNormal)
            s_DefaultNormal = CreateSinglePixelTexture(device, 128, 128, 255, 255, "Primitive_Normal");
        return s_DefaultNormal;
    }

    void PrimitiveFactory::Shutdown()
    {
        s_DefaultWhite.reset();
        s_DefaultNormal.reset();
        s_Sampler.reset();
    }

    // -------------------------------------------------------------------------
    // BuildModel
    // -------------------------------------------------------------------------
    Ref<Model> PrimitiveFactory::BuildModel(RHIDevice& device,
        RHIDescriptorSetLayout* textureLayout,
        std::vector<Vertex> vertices,
        std::vector<uint32_t> indices)
    {
        auto model = CreateRef<Model>();

        // Vertex buffer
        {
            uint64_t size = sizeof(Vertex) * vertices.size();

            BufferSpec st{};
            st.Size = size; st.Usage = BufferUsage::Staging;
            st.CpuVisible = true; st.DebugName = "Prim_VB_Stage";
            auto staging = device.CreateBuffer(st);
            staging->SetData(vertices.data(), size);

            BufferSpec vb{};
            vb.Size = size; vb.Usage = BufferUsage::Vertex;
            vb.CpuVisible = false; vb.DebugName = "Prim_VB";
            model->VertexBuffer = device.CreateBuffer(vb);
            device.CopyBuffer(staging.get(), model->VertexBuffer.get(), size);
        }

        // Index buffer + SubMesh
        {
            uint64_t size = sizeof(uint32_t) * indices.size();

            BufferSpec st{};
            st.Size = size; st.Usage = BufferUsage::Staging;
            st.CpuVisible = true; st.DebugName = "Prim_IB_Stage";
            auto staging = device.CreateBuffer(st);
            staging->SetData(indices.data(), size);

            SubMesh sub{};
            sub.IndexCount = (uint32_t)indices.size();
            sub.Type = IndexType::Uint32;

            BufferSpec ib{};
            ib.Size = size; ib.Usage = BufferUsage::Index;
            ib.CpuVisible = false; ib.DebugName = "Prim_IB";
            sub.IndexBuffer = device.CreateBuffer(ib);
            device.CopyBuffer(staging.get(), sub.IndexBuffer.get(), size);

            sub.Mat.Albedo = GetDefaultWhite(device);
            sub.Mat.Normal = GetDefaultNormal(device);
            sub.Mat.RoughnessMetallic = GetDefaultWhite(device);
            sub.Mat.Color = { 1.0f, 1.0f, 1.0f, 1.0f };
            sub.Mat.RoughnessOverride = 0.8f;
            sub.Mat.MetallicOverride = 0.0f;

            if (textureLayout)
            {
                if (!s_Sampler)
                {
                    SamplerSpec sp{};
                    sp.MagFilter = FilterMode::Linear;
                    sp.MinFilter = FilterMode::Linear;
                    sp.AddressU = AddressMode::Repeat;
                    sp.AddressV = AddressMode::Repeat;
                    sp.DebugName = "Prim_Sampler";
                    s_Sampler = device.CreateSampler(sp);
                }

                sub.Mat.DescriptorSet = device.CreateDescriptorSet(textureLayout);
                sub.Mat.DescriptorSet->BindTexture(0, sub.Mat.Albedo.get(), s_Sampler.get());
                sub.Mat.DescriptorSet->BindTexture(1, sub.Mat.Normal.get(), s_Sampler.get());
                sub.Mat.DescriptorSet->BindTexture(2, sub.Mat.RoughnessMetallic.get(), s_Sampler.get());
                sub.Mat.DescriptorSet->Update();
            }

            model->Meshes.push_back(std::move(sub));
        }

        for (const auto& v : vertices)
        {
            model->Bounds.Min = glm::min(model->Bounds.Min, v.Position);
            model->Bounds.Max = glm::max(model->Bounds.Max, v.Position);
        }

        return model;
    }

    // -------------------------------------------------------------------------
    // Plane
    // -------------------------------------------------------------------------
    Ref<Model> PrimitiveFactory::CreatePlane(RHIDevice& device,
        RHIDescriptorSetLayout* textureLayout,
        float size,
        uint32_t subdivisions)
    {
        std::vector<Vertex>   vertices;
        std::vector<uint32_t> indices;

        uint32_t gridSize = subdivisions + 1;
        float    step = size / (float)subdivisions;
        float    half = size * 0.5f;
        float    uvStep = 1.0f / (float)subdivisions;

        for (uint32_t z = 0; z < gridSize; ++z)
        {
            for (uint32_t x = 0; x < gridSize; ++x)
            {
                Vertex v{};
                v.Position = glm::vec3(-half + x * step, 0.0f, -half + z * step);
                v.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
                v.TexCoord = glm::vec2(x * uvStep, z * uvStep);
                v.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);
                vertices.push_back(v);
            }
        }

        for (uint32_t z = 0; z < subdivisions; ++z)
        {
            for (uint32_t x = 0; x < subdivisions; ++x)
            {
                uint32_t tl = z * gridSize + x;
                uint32_t tr = tl + 1;
                uint32_t bl = tl + gridSize;
                uint32_t br = bl + 1;

                indices.push_back(tl); indices.push_back(bl); indices.push_back(tr);
                indices.push_back(tr); indices.push_back(bl); indices.push_back(br);
            }
        }

        return BuildModel(device, textureLayout, std::move(vertices), std::move(indices));
    }

    // -------------------------------------------------------------------------
    // Cube — lambda вместо FaceData чтобы MSVC не путался с initializer list
    // -------------------------------------------------------------------------
    Ref<Model> PrimitiveFactory::CreateCube(RHIDevice& device,
        RHIDescriptorSetLayout* textureLayout)
    {
        std::vector<Vertex>   vertices;
        std::vector<uint32_t> indices;
        vertices.reserve(24);
        indices.reserve(36);

        auto addFace = [&](
            glm::vec3 n, glm::vec3 t,
            glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3,
            glm::vec2 uv0, glm::vec2 uv1, glm::vec2 uv2, glm::vec2 uv3) {
                uint32_t base = (uint32_t)vertices.size();
                auto addV = [&](glm::vec3 p, glm::vec2 uv) {
                    Vertex vert{};
                    vert.Position = p; vert.Normal = n;
                    vert.TexCoord = uv; vert.Tangent = t;
                    vertices.push_back(vert);
                    };
                addV(p0, uv0); addV(p1, uv1); addV(p2, uv2); addV(p3, uv3);
                // clockwise
                indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
                indices.push_back(base + 0); indices.push_back(base + 3); indices.push_back(base + 2);
            };

        // +Y top
        addFace({ 0,1,0 }, { 1,0,0 },
            { -0.5f, 0.5f,-0.5f }, { 0.5f, 0.5f,-0.5f }, { 0.5f, 0.5f, 0.5f }, { -0.5f, 0.5f, 0.5f },
            { 0,0 }, { 1,0 }, { 1,1 }, { 0,1 });
        // -Y bottom
        addFace({ 0,-1,0 }, { 1,0,0 },
            { -0.5f,-0.5f, 0.5f }, { 0.5f,-0.5f, 0.5f }, { 0.5f,-0.5f,-0.5f }, { -0.5f,-0.5f,-0.5f },
            { 0,0 }, { 1,0 }, { 1,1 }, { 0,1 });
        // +X right
        addFace({ 1,0,0 }, { 0,0,-1 },
            { 0.5f,-0.5f, 0.5f }, { 0.5f, 0.5f, 0.5f }, { 0.5f, 0.5f,-0.5f }, { 0.5f,-0.5f,-0.5f },
            { 0,0 }, { 0,1 }, { 1,1 }, { 1,0 });
        // -X left
        addFace({ -1,0,0 }, { 0,0,1 },
            { -0.5f,-0.5f,-0.5f }, { -0.5f, 0.5f,-0.5f }, { -0.5f, 0.5f, 0.5f }, { -0.5f,-0.5f, 0.5f },
            { 0,0 }, { 0,1 }, { 1,1 }, { 1,0 });
        // +Z front
        addFace({ 0,0,1 }, { 1,0,0 },
            { -0.5f,-0.5f, 0.5f }, { -0.5f, 0.5f, 0.5f }, { 0.5f, 0.5f, 0.5f }, { 0.5f,-0.5f, 0.5f },
            { 0,0 }, { 0,1 }, { 1,1 }, { 1,0 });
        // -Z back
        addFace({ 0,0,-1 }, { -1,0,0 },
            { 0.5f,-0.5f,-0.5f }, { 0.5f, 0.5f,-0.5f }, { -0.5f, 0.5f,-0.5f }, { -0.5f,-0.5f,-0.5f },
            { 0,0 }, { 0,1 }, { 1,1 }, { 1,0 });

        return BuildModel(device, textureLayout, std::move(vertices), std::move(indices));
    }

} // namespace Flux