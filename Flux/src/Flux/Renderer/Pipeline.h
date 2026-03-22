#pragma once


#include "Flux/Core.h"

#include "Shader.h"
#include "UniformBuffer.h"

namespace Flux {

	class Pipeline
	{
	public:
		virtual ~Pipeline() = default;

		virtual void Bind() const = 0;
		virtual void SetUniformBuffer(const Ref<UniformBuffer>& uniformBuffer) = 0;

		static Ref<Pipeline> Create(const Ref<Shader>& shader);
	};

}