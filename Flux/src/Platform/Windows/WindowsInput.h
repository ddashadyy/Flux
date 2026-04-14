#pragma once


#include "Flux/Core/Input.h"


namespace Flux {

	class WindowsInput final : public Input
	{
	protected: 
		bool IsKeyPressedImpl(int keycode) override;

		std::pair<float, float> GetMousePositionImpl() override;
		bool IsMouseButtonPressedImpl(int button) override;
		float GetMouseXImpl() override;
		float GetMouseYImpl() override;
	};

}