#pragma once

namespace Flux {

    // GPU-GPU синхронизация (queue submit wait/signal).
    // Signal и Wait — это параметры Submit, не методы семафора.
    // CPU не должен трогать семафор напрямую.
    class RHISemaphore
    {
    public:
        virtual ~RHISemaphore() = default;

        virtual void* GetHandle() const = 0;
    };

} // namespace Flux
