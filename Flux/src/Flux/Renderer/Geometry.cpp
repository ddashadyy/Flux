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


	Material::Material(RHIDevice& device, const MaterialData& materialData)
	{
		m_VertexShader = device.CreateShader(ShaderStage::Vertex, materialData.VertexShaderSPIRV);
		m_FragmentShader = device.CreateShader(ShaderStage::Fragment, materialData.FragmentShaderSPIRV);

		DescriptorSetLayoutDesc layoutDesc{};
		layoutDesc.Bindings = {
			{ 0, DescriptorType::UniformBuffer },
			{ 1, DescriptorType::CombinedImageSampler },
			{ 2, DescriptorType::CombinedImageSampler },
			{ 3, DescriptorType::CombinedImageSampler },
		};

		m_DescriptorSetLayout = device.CreateDescriptorSetLayout(layoutDesc);
		m_DescriptorSet = device.CreateDescriptorSet(m_DescriptorSetLayout.get());

		BufferSpec paramsSpec{};
		paramsSpec.Size = sizeof(MaterialParams);
		paramsSpec.Usage = BufferUsage::Uniform;
		paramsSpec.CpuVisible = true;

		m_ParamsBuffer = device.CreateBuffer(paramsSpec);
		m_ParamsBuffer->SetData(&m_Params, sizeof(MaterialParams));

		m_DescriptorSet->BindBuffer(0, m_ParamsBuffer.get());
	}

	void Material::SetTexture(uint32_t binding, Ref<RHITexture> texture)
	{
		m_DescriptorSet->BindTexture(binding, texture.get());
	}

	void Material::SetParams(const MaterialParams& params)
	{
		m_Params = params;
		m_ParamsBuffer->SetData(&m_Params, sizeof(MaterialParams));
	}
}