#pragma once

#include "Flux/Core.h"
#include "Flux/Renderer/RendererAPI.h"


namespace Flux {

	class VulkanRendererAPI final : public RendererAPI
	{
	public:
		VulkanRendererAPI() = default;
		~VulkanRendererAPI() = default;

		FL_NON_COPYABLE(VulkanRendererAPI);

		void SetViewport(float width, float height, float x = 0.0f, float y = 0.0f, float minDepth = 0.0f, float maxDepth = 1.0f) override;

		void BeginRenderPass(const glm::vec4& color) override;
		void EndRenderPass() override;

		void DrawIndexed(uint32_t indexCount = 0) override;
	};
}