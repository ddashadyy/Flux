#pragma once

#include "RHICommon.h"

#include <string>
#include <vector>

namespace Flux {

    struct ShaderSpec
    {
        ShaderStage           Stage      = ShaderStage::Vertex;
        std::vector<uint32_t> Spirv      = {};
        std::string           EntryPoint = "main";
        const char*           DebugName  = nullptr;
    };

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

} // namespace Flux
