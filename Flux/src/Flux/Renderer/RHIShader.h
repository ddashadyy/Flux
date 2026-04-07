#pragma once


#include <string>

namespace Flux {

    enum class ShaderStage : uint8_t
    {
        Vertex,
        Fragment,
        Compute,
    };

    class RHIShader
    {
    public:
        virtual ~RHIShader() = default;

        virtual ShaderStage        GetStage()      const = 0;
        virtual const std::string& GetEntryPoint() const = 0;

    };

}