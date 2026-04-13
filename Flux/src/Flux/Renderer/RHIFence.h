#pragma once


#include <cstdint>

namespace Flux {

    // Барьер
    class RHIFence 
    {
    public:
        virtual ~RHIFence() = default;

        template<typename T>
        T GetHandle() const { return reinterpret_cast<T>(GetHandleImpl()); }

        virtual void Wait(uint64_t timeout = UINT64_MAX) = 0;
        virtual void Reset() = 0;
        virtual bool IsSignaled() const = 0;

    protected:
        virtual void* GetHandleImpl() const = 0;
    };
}