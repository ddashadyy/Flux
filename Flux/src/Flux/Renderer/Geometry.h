#pragma once


#include <glm/glm.hpp>

#include "RHIBuffer.h"
#include "RHIDevice.h"
#include "RHIShader.h"
#include "RHITexture.h"

namespace Flux {

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
        IndexType        Type = IndexType::Uint32;
        Material         Mat;

        void Draw(RHICommandList& cmdList) const
        {
            cmdList.BindIndexBuffer(IndexBuffer.get(), Type);
            cmdList.DrawIndexed(IndexCount);
        }
    };

    struct Model
    {
        Scope<RHIBuffer>     VertexBuffer;
        std::vector<SubMesh> Meshes;
    };

}