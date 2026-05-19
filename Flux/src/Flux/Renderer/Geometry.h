#pragma once


#include <glm/glm.hpp>

#include "RHIBuffer.h"
#include "RHIDevice.h"
#include "RHIShader.h"
#include "RHITexture.h"

#include <filesystem>

namespace Flux {

    struct AABB
    {
        glm::vec3 Min{ FLT_MAX,  FLT_MAX,  FLT_MAX };
        glm::vec3 Max{ -FLT_MAX, -FLT_MAX, -FLT_MAX };

        glm::vec3 Center()  const { return (Min + Max) * 0.5f; }
        glm::vec3 Extents() const { return (Max - Min) * 0.5f; }
        bool      IsValid() const { return Min.x <= Max.x; }
    };

    struct Vertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoord;
        glm::vec3 Tangent;   

        bool operator == (const Vertex& other) const
        {
            return Position == other.Position &&
                Normal == other.Normal &&
                TexCoord == other.TexCoord &&
                Tangent == other.Tangent;  
        }
    };

    using Texture = RHITexture;

    struct Material
    {
        Ref<Texture> Albedo;
        Ref<Texture> Normal;
        Ref<Texture> RoughnessMetallic;

        glm::vec4 Color = glm::vec4(1.0f);
        // -1 == брать из текстуры
        float     RoughnessOverride = -1.0f;
        float     MetallicOverride = -1.0f;

        Scope<RHIDescriptorSet> DescriptorSet;
    };

    struct SubMesh
    {
        Scope<RHIBuffer> IndexBuffer;
        uint32_t         IndexCount = 0;
        uint32_t         BaseVertex = 0; 
        IndexType        Type = IndexType::Uint32;
        Material         Mat;

        void Draw(RHICommandList& cmdList) const
        {
            cmdList.BindIndexBuffer(IndexBuffer.get(), Type);
            cmdList.DrawIndexed(IndexCount, 1, 0, BaseVertex, 0);
        }
    };

    struct Model
    {
        std::filesystem::path Path;
        Scope<RHIBuffer>     VertexBuffer;
        std::vector<SubMesh> Meshes;
        AABB                 Bounds;

        const std::filesystem::path& GetPath() const { return Path; }
    };

}