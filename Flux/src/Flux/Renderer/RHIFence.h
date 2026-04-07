#pragma once


#include <cstdint>

namespace Flux {

    // Барьер
    class RHIFence 
    {
    public:
        virtual ~RHIFence() = default;

        virtual void Wait(uint64_t timeout = UINT64_MAX) = 0;
        virtual void Reset() = 0;
        virtual bool IsSignaled() const = 0;
    };
}