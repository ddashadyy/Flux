#pragma once

namespace Flux {

    class RendererAPI
    {
    public:
        enum class API : uint8_t
        {
            None = 0,
            Vulkan = 1
        };

        static API GetAPI() { return s_API; }

        static const char* GetAPIName()
        {
            switch (s_API)
            {
            case API::Vulkan: return "Vulkan";
            case API::None:   return "None";
            }
            return "Unknown";
        }

    private:
        static API s_API;
    };

}
