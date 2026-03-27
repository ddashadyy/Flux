#include "flpch.h"
#include "Renderer.h"
#include "RenderCommand.h"

namespace Flux {
	void Renderer::BeginScene()
	{

	}

	void Renderer::EndScene()
	{

	}

	void Renderer::BeginRenderPass(const glm::vec4& color)
	{
		RenderCommand::BeginRenderPass(color);
	}

	void Renderer::EndRenderPass()
	{
		RenderCommand::EndRenderPass();
	}

	void Renderer::SetViewport(float width, float height, float x, float y, float minDepth, float maxDepth)
	{
		RenderCommand::SetViewport(width, height, x, y, minDepth, maxDepth);
	}

	void Renderer::Submit(const Ref<Pipeline>& pipeline, const Ref<VertexBuffer>& vertexBuffer, const Ref<IndexBuffer>& indexBuffer)
	{
		pipeline->Bind();
		vertexBuffer->Bind();
		indexBuffer->Bind();
		RenderCommand::DrawIndexed(indexBuffer->GetCount());
	}
}