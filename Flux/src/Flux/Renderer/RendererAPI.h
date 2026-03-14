#pragma once

namespace Flux {

    class RendererAPI
    {
    public:
        enum class API
        {
            None = 0,
            Vulkan = 1
        };

        static API GetAPI() { return s_API; }

    private:
        static API s_API;
    };

}
