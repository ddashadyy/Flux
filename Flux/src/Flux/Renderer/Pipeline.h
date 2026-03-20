#pragma once


#include "Flux/Core.h"
#include "Shader.h"

namespace Flux {

	class Pipeline
	{
	public:
		virtual ~Pipeline() = default;
		virtual void Bind() const = 0;

		static Ref<Pipeline> Create(const Ref<Shader>& shader);
	};

}