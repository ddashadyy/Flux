#pragma once

#include "Base.h"
#include "Flux/Events/Event.h"

namespace Flux {

    class RHICommandList;

    class FLUX_API Layer
    {
    public:
        Layer(const std::string& name = "Layer");
        virtual ~Layer();

        virtual void OnAttach() {}
        virtual void OnDetach() {}
        virtual void OnUpdate(RHICommandList* cmdList) {}
        virtual void OnImGuiRender() {}
        virtual void OnEvent(Event& event) {}
        virtual void OnResize(uint32_t width, uint32_t height) {}

        const std::string& GetName() const { return m_DebugName; }

    protected:
        std::string m_DebugName;
    };
}