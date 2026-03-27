#pragma once


#include "Flux/Core.h"
#include "RendererAPI.h"



// Raw render commands
namespace Flux {

	class RenderCommand
	{
	public:
		inline static void SetViewport(float width, float height, float x = 0.0f, float y = 0.0f, float minDepth = 0.0f, float maxDepth = 1.0f)
		{
			s_RendererAPI->SetViewport(width, height, x, y, minDepth, maxDepth);
		}

		inline static void BeginRenderPass(const glm::vec4& color)
		{
			s_RendererAPI->BeginRenderPass(color);
		}

		inline static void EndRenderPass()
		{
			s_RendererAPI->EndRenderPass();
		}

		inline static void DrawIndexed(uint32_t indexCount)
		{
			s_RendererAPI->DrawIndexed(indexCount);
		}

	private:
		static Scope<RendererAPI> s_RendererAPI;

	};


}
