#pragma once



namespace Flux
{
	class RHISemaphore
	{
	public:
		virtual ~RHISemaphore() = default;

		template<typename T>
		T GetHandle() const { return reinterpret_cast<T>(GetHandleImpl()); }

		virtual void Signal() = 0;
		virtual void Wait()   = 0;

	protected:
		virtual void* GetHandleImpl() const = 0;
	};
}