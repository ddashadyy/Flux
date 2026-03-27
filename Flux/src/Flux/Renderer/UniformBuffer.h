#pragma once


#include <glm/glm.hpp>

#include "Flux/Core.h"

namespace Flux {

	struct UniformBufferObject
	{
		alignas(16) glm::mat4 Model;
		alignas(16) glm::mat4 View;
		alignas(16) glm::mat4 Projection;
	};

	class UniformBuffer
	{
	public:
		virtual ~UniformBuffer() = default;
		virtual void SetData(const UniformBufferObject& ubo) = 0;

		static Ref<UniformBuffer> Create(uint32_t size);
	};

}