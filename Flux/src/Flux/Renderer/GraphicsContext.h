#pragma once

#include "Flux/Core.h"

namespace Flux {

	class GraphicsContext
	{
	public:
		virtual ~GraphicsContext() = default;

		virtual void Init() = 0;
		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual bool IsFrameStarted() const = 0;

		static Scope<GraphicsContext> Create(void* window);
	};

}