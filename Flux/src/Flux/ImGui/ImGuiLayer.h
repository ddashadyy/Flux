#pragma once

#include "Flux/Core/Layer.h"

#include "Flux/Events/ApplicationEvent.h"
#include "Flux/Events/MouseEvent.h"
#include "Flux/Events/KeyEvent.h"

#include "Flux/Renderer/RHIRenderPass.h"
#include "Flux/Renderer/RHIFramebuffer.h"

namespace Flux {

    class FLUX_API ImGuiLayer : public Layer
    {
    public:
        ImGuiLayer();
        ~ImGuiLayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnImGuiRender() override;

        void Begin();
        void End(uint32_t frameIndex = 0);

        void OnResize(uint32_t width, uint32_t height) override;

        RHIRenderPass&  GetRenderPass() const { return *m_RenderPass; }
        RHIFramebuffer& GetFramebuffer(uint32_t index) const { return *m_Framebuffers[index]; }

    private:
        void CreateRenderPass();
        void CreateFramebuffers();

    private:
        Scope<RHIRenderPass>               m_RenderPass;
        std::vector<Scope<RHIFramebuffer>> m_Framebuffers;

        float m_Time = 0.0f;
    };

}