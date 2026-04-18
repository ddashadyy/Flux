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

    struct MeshData
    {
        std::vector<Vertex> Vertices;
        std::vector<uint32_t> Indices;
		IndexType Type = IndexType::Uint32;
	};

    class Mesh
    {
    public:
        Mesh(RHIDevice& device, const MeshData& meshData);

		void Draw(RHICommandList& commandList) const;

		uint32_t GetIndexCount() const { return m_IndexCount; }

    private:
		Scope<RHIBuffer> m_VertexBuffer;
		Scope<RHIBuffer> m_IndexBuffer;

        uint32_t m_IndexCount = 0;
		IndexType m_IndexType = IndexType::Uint32;
    };

}