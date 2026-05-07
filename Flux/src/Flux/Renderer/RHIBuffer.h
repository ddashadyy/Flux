#pragma once

#include "Flux/Core/Base.h"
#include "RHICommon.h" 

namespace Flux {

    enum class BufferUsage : uint8_t
    {
        Vertex   = BIT(0),
        Index    = BIT(1),
        Uniform  = BIT(2),
        Storage  = BIT(3),
        Staging  = BIT(4),
        Indirect = BIT(5), // для indirect draw
    };
    FL_ENABLE_BITWISE_OPERATORS(BufferUsage);

    struct BufferSpec
    {
        uint64_t    Size       = 0;
        BufferUsage Usage      = BufferUsage::Vertex;
        bool        CpuVisible = false;
        const char* DebugName  = nullptr; 
    };


    class RHIBuffer
    {
    public:
        virtual ~RHIBuffer() = default;

        virtual void* GetHandle() const = 0;

        virtual void* Map()   = 0;
        virtual void  Unmap() = 0;

        virtual const BufferSpec& GetSpec()  const = 0; 
        uint64_t    GetSize()  const { return GetSpec().Size; }
        BufferUsage GetUsage() const { return GetSpec().Usage; }

        void SetData(const void* data, uint64_t size, uint64_t offset = 0)
        {
            uint8_t* ptr = static_cast<uint8_t*>(Map());
            memcpy(ptr + offset, data, size);
            Unmap();
        }
    };

} // namespace Flux
