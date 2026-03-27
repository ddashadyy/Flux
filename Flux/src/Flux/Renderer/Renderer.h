#pragma once


#include "Flux/Core.h"
#include "Buffer.h"
#include "Pipeline.h"
#include "RendererAPI.h"

namespace Flux {

	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(/* camera */);
		static void EndScene();

		static void BeginRenderPass(const glm::vec4& color);
		static void EndRenderPass();

		static void SetViewport(float width, float height, float x = 0.0f, float y = 0.0f, float minDepth = 0.0f, float maxDepth = 1.0f);

		static void Submit(
			const Ref<Pipeline>& pipeline,
			const Ref<VertexBuffer>& vertexBuffer,
			const Ref<IndexBuffer>& indexBuffer
		);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	};

}