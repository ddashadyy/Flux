#pragma once


#include <glm/glm.hpp>

#include "Flux/Core.h"

namespace Flux {

	struct UniformBufferObject
	{
		glm::mat4 Model;
		glm::mat4 View;
		glm::mat4 Projection;
	};

	class UniformBuffer
	{
	public:
		virtual ~UniformBuffer() = default;
		virtual void SetData(const UniformBufferObject& ubo) = 0;

		static Ref<UniformBuffer> Create(uint32_t size);
	};

}