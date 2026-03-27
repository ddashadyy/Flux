#pragma once

#include "Flux/Core.h"
#include <glm/glm.hpp>

namespace Flux {

    class RendererAPI
    {
    public:
        enum class API : uint8_t
        {
            None = 0,
            Vulkan = 1
        };

    public:
        virtual ~RendererAPI() = default;

        virtual void SetViewport(float width, float height, float x = 0.0f, float y = 0.0f, float minDepth = 0.0f, float maxDepth = 1.0f) = 0;

        virtual void BeginRenderPass(const glm::vec4& color) = 0;
        virtual void EndRenderPass() = 0;

		virtual void DrawIndexed(uint32_t indexCount = 0) = 0;

        inline static API GetAPI() { return s_API; }

        static const char* GetAPIName()
        {
            switch (s_API)
            {
            case API::Vulkan: return "Vulkan";
            case API::None:   return "None";
            }
            return "Unknown";
        }

		static Scope<RendererAPI> Create();

    private:
        static API s_API;
    };

}
