#include "flpch.h"
#include "Geometry.h"


namespace Flux {

	Mesh::Mesh(RHIDevice& device, const MeshData& meshData)
	{
		BufferSpec vertexBufferSpec{};
		vertexBufferSpec.Size       = sizeof(Vertex) * meshData.Vertices.size();
		vertexBufferSpec.Usage      = BufferUsage::Vertex;
		vertexBufferSpec.CpuVisible = true;

		m_VertexBuffer = device.CreateBuffer(vertexBufferSpec);
		m_VertexBuffer->SetData(meshData.Vertices.data(), vertexBufferSpec.Size);

		BufferSpec indexBufferSpec{};
		indexBufferSpec.Size       = sizeof(uint32_t) * meshData.Indices.size();
		indexBufferSpec.Usage      = BufferUsage::Index;
		indexBufferSpec.CpuVisible = true;

		m_IndexBuffer = device.CreateBuffer(indexBufferSpec);
		m_IndexBuffer->SetData(meshData.Indices.data(), indexBufferSpec.Size);

		m_IndexCount = static_cast<uint32_t>(meshData.Indices.size());
		m_IndexType  = meshData.Type;
	}

	void Mesh::Draw(RHICommandList& commandList) const
	{
		commandList.BindVertexBuffer(m_VertexBuffer.get());
		commandList.BindIndexBuffer(m_IndexBuffer.get(), m_IndexType);
		commandList.DrawIndexed(m_IndexCount);
	}

}