#pragma once

namespace Flux {

    // GPU-GPU синхронизация (queue submit wait/signal).
    // Signal и Wait — это параметры Submit, не методы семафора.
    // CPU не должен трогать семафор напрямую.
    class RHISemaphore
    {
    public:
        virtual ~RHISemaphore() = default;

        template<typename T>
        T GetHandle() const { return reinterpret_cast<T>(GetHandleImpl()); }

    protected:
        virtual void* GetHandleImpl() const = 0;
    };

} // namespace Flux
