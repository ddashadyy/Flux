#pragma once

#include <glm/glm.hpp>

#include "Flux/Renderer/RHIDevice.h"
#include "Flux/Renderer/RHIBuffer.h"
#include "Flux/Renderer/RHICommandList.h"
#include "Flux/Renderer/RHIDescriptorSet.h"
#include "Flux/Renderer/RHIPipeline.h"
#include "Flux/Scene/Entity.h"

namespace Flux {

    struct DirectionalLight
    {
		glm::vec4 Direction = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f); // w не используется
        glm::vec4 Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);      // w = intensity
    };

    struct GlobalUBO
    {
        glm::mat4        View;
        glm::mat4        Projection;
        glm::vec4        CameraPos;
        DirectionalLight Light;
    };

    struct PushConstantData
    {
        glm::mat4 Model;
        glm::vec4 Color;
        float     RoughnessOverride;
        float     MetallicOverride;
        float     _pad0;
        float     _pad1;
    };

    class PerspectiveCamera;

    class Renderer
    {
    public:
        explicit Renderer(RHIDevice& device);

        void BeginScene(RHICommandList& cmd,
            RHIPipeline& pipeline,
            const PerspectiveCamera& camera,
            const DirectionalLight& light = {});

        void Submit(const Entity& entity);
        void EndScene();

        RHIDescriptorSetLayout* GetGlobalDescriptorSetLayout()  const { return m_GlobalDescriptorSetLayout.get(); }
        RHIDescriptorSetLayout* GetTextureDescriptorSetLayout() const { return m_TextureDescriptorSetLayout.get(); }

    private:
        RHIDevice& m_Device;

        RHICommandList* m_CommandList = nullptr;
        RHIPipeline*    m_Pipeline    = nullptr;

        Scope<RHIBuffer>              m_GlobalUBO;
        Scope<RHIDescriptorSetLayout> m_GlobalDescriptorSetLayout;
        Scope<RHIDescriptorSet>       m_GlobalDescriptorSet;
        Scope<RHIDescriptorSetLayout> m_TextureDescriptorSetLayout;
    };

}