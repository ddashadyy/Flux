#pragma once


#include "Flux/Core.h"

namespace Flux {

	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const {}
		virtual void Unbind() const {}

		virtual const std::string& GetName() const = 0;

		static Ref<Shader> Create(const std::string& filePath);
	};
}