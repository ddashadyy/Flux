#pragma once

#include "Flux/Renderer/Geometry.h"
#include "Flux/Renderer/RHIDevice.h"

namespace Flux {

    class PrimitiveFactory
    {
    public:
        static Ref<Model> CreatePlane(RHIDevice& device,
            RHIDescriptorSetLayout* textureLayout,
            float size = 10.0f,
            uint32_t subdivisions = 1);

        static Ref<Model> CreateCube(RHIDevice& device,
            RHIDescriptorSetLayout* textureLayout);

        static void Shutdown();

    private:
        static Ref<Model> BuildModel(RHIDevice& device,
            RHIDescriptorSetLayout* textureLayout,
            std::vector<Vertex> vertices,
            std::vector<uint32_t> indices);

        static Ref<Texture> GetDefaultWhite(RHIDevice& device);
        static Ref<Texture> GetDefaultNormal(RHIDevice& device);

        static Ref<Texture> s_DefaultWhite;
        static Ref<Texture> s_DefaultNormal;

        static Scope<RHISampler> s_Sampler;
    };

} // namespace Flux