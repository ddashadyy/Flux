#include "flpch.h"
#include "VulkanRendererAPI.h"
#include "VulkanContext.h"


namespace Flux {

	void VulkanRendererAPI::SetViewport(float width, float height, float x, float y, float minDepth, float maxDepth)
	{
		VkCommandBuffer currentCommandBuffer = VulkanContext::Get().GetCurrentCommandBuffer();

		VkViewport viewport{};
		viewport.x = x;
		viewport.y = y;
		viewport.width = width;
		viewport.height = height;
		viewport.minDepth = minDepth;
		viewport.maxDepth = maxDepth;
		vkCmdSetViewport(currentCommandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset.x = static_cast<int32_t>(x);
		scissor.offset.y = static_cast<int32_t>(y);
		scissor.extent.width = static_cast<uint32_t>(width);
		scissor.extent.height = static_cast<uint32_t>(height);
		vkCmdSetScissor(currentCommandBuffer, 0, 1, &scissor);
	}

	void VulkanRendererAPI::BeginRenderPass(const glm::vec4& color)
	{
		auto& vulkanContext = VulkanContext::Get();

		VkClearValue clearColor = { color.r, color.g, color.b, color.a };
		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = vulkanContext.GetRenderPass();
		renderPassBeginInfo.framebuffer = vulkanContext.GetCurrentFramebuffer();
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.renderArea.extent = vulkanContext.GetVulkanSwapchain().GetExtent();
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(vulkanContext.GetCurrentCommandBuffer(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VulkanRendererAPI::EndRenderPass()
	{
		vkCmdEndRenderPass(VulkanContext::Get().GetCurrentCommandBuffer());
	}

	void VulkanRendererAPI::DrawIndexed(uint32_t indexCount)
	{
		if (indexCount == 0)
		{
			FL_CORE_WARN("DrawIndexed called with indexCount = 0");
			return;
		}

		auto& context = VulkanContext::Get();
		VkCommandBuffer cmd = context.GetCurrentCommandBuffer();

		vkCmdDrawIndexed(cmd, indexCount, 1, 0, 0, 0);
	}
}