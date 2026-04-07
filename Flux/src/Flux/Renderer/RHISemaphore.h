#pragma once



namespace Flux
{
	class RHISemaphore
	{
	public:
		virtual ~RHISemaphore() = default;

		virtual void Signal() = 0;
		virtual void Wait()   = 0;
	};
}