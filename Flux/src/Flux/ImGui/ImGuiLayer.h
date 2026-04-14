#pragma once

#include "Flux/Core/Layer.h"

#include "Flux/Events/ApplicationEvent.h"
#include "Flux/Events/MouseEvent.h"
#include "Flux/Events/KeyEvent.h"

namespace Flux {

	class FLUX_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		void Begin();
		void End(uint32_t frameIndex = 0);

	private:
		float m_Time = 0.0f;
	};

}