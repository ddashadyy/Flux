#pragma once


#include "RHICommon.h"
#include <string>

namespace Flux {

    class RHIShader
    {
    public:
        virtual ~RHIShader() = default;

        virtual ShaderStage        GetStage()      const = 0;
        virtual const std::string& GetEntryPoint() const = 0;

    };

}