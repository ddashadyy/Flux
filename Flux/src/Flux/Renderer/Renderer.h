#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "RHIDevice.h"
#include "RHIBuffer.h"
#include "RHICommandList.h"
#include "RHIDescriptorSet.h"
#include "RHIPipeline.h"

#include "Geometry.h"


namespace Flux {

    struct PointLight
    {
        glm::vec4 Position = glm::vec4(0.0f);  // w не используется
        glm::vec4 Color    = glm::vec4(1.0f);  // w = intensity
    };

    
    struct GlobalUBO
    {
        glm::mat4  View;
        glm::mat4  Projection;
        glm::vec3  CameraPos;
        int        LightCount = 0;
        PointLight Lights[8];
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

        void BeginScene(RHICommandList& cmd, RHIPipeline& pipeline,
            const PerspectiveCamera& camera, float viewportWidth, float viewportHeight);

        void AddPointLight(const PointLight& light);
        void ClearLights();

        void Submit(Ref<Model> model, const glm::mat4& transform);
        void SubmitSkinned(Ref<Model> model, const glm::mat4& transform,
            RHIBuffer* skinningBuffer,
            RHIDescriptorSet* skinningDescriptorSet,
            RHIPipeline* skinnedPipeline);

        
        void EndScene();

        RHIDescriptorSetLayout* GetGlobalDescriptorSetLayout()   const { return m_GlobalDescriptorSetLayout.get(); }
        RHIDescriptorSetLayout* GetTextureDescriptorSetLayout()  const { return m_TextureDescriptorSetLayout.get(); }
        RHIDescriptorSetLayout* GetSkinningDescriptorSetLayout() const { return m_SkinningDescriptorSetLayout.get(); }

    private:
        RHIDevice& m_Device;

        RHICommandList* m_CommandList = nullptr;
        RHIPipeline*    m_Pipeline    = nullptr;

        Scope<RHIBuffer>              m_GlobalUBO;
        Scope<RHIDescriptorSetLayout> m_GlobalDescriptorSetLayout;
        Scope<RHIDescriptorSet>       m_GlobalDescriptorSet;
        Scope<RHIDescriptorSetLayout> m_TextureDescriptorSetLayout;

        Scope<RHIDescriptorSetLayout> m_SkinningDescriptorSetLayout;

        std::vector<PointLight> m_PointLights;
    };

}