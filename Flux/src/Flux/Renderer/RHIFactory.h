#pragma once


#include "RHIDevice.h"
#include "RHICommon.h"
#include "RendererBackend.h"

namespace Flux {

    class RHIFactory {
    public:
        static Scope<RHIDevice> CreateDevice(RendererBackend backend,
            void* windowHandle,
            uint32_t width,
            uint32_t height);
    private:
        static RendererBackend DetectBestBackend();
    };
}