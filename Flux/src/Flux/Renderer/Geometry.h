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


    struct MaterialParams
    {
        glm::vec4 BaseColor = { 1.0f, 1.0f, 1.0f, 1.0f }; // rgb = color, a = opacity
        glm::vec4 Properties = { 0.5f, 0.0f, 0.0f, 0.0f }; // r = roughness, g = metallic
    };

    struct MaterialData
    {
        std::vector<uint32_t> VertexShaderSPIRV;
        std::vector<uint32_t> FragmentShaderSPIRV;
    };

    class Material
    {
    public:
        Material(RHIDevice& device, const MaterialData& materialData);

        void SetTexture(uint32_t binding, Ref<RHITexture> texture);
        void SetParams(const MaterialParams& params);

        RHIShader& GetVertexShader()   const { return *m_VertexShader; }
        RHIShader& GetFragmentShader() const { return *m_FragmentShader; }
        RHIDescriptorSet& GetDescriptorSet()  const { return *m_DescriptorSet; }

    private:
        Ref<RHIShader>                m_VertexShader;
        Ref<RHIShader>                m_FragmentShader;
        Scope<RHIDescriptorSetLayout> m_DescriptorSetLayout;
        Scope<RHIDescriptorSet>       m_DescriptorSet;
        Scope<RHIBuffer>              m_ParamsBuffer;
        MaterialParams                m_Params;
    };

}