#pragma once


#include "RHICommon.h"
#include <string>

namespace Flux {

    class RHIShader
    {
    public:
        virtual ~RHIShader() = default;

        template<typename T>
        T GetHandle() const { return reinterpret_cast<T>(GetHandleImpl()); }

        virtual ShaderStage        GetStage()      const = 0;
        virtual const std::string& GetEntryPoint() const = 0;

    protected:
        virtual void* GetHandleImpl() const = 0;

    };

}