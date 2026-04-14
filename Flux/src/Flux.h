#pragma once


// For use Flux applications

#include "Flux/Core/Application.h"
#include "Flux/Core/Layer.h"
#include "Flux/Core/Log.h"

#include "Flux/Core/Input.h"
#include "Flux/Core/KeyCodes.h"
#include "Flux/Core/MouseButtonCodes.h"

#include "Flux/ImGui/ImGuiLayer.h"

// ---Camera-------------
#include "Flux/Renderer/PerspectiveCamera.h"
// ----------------------

// ---Input-------------
#include "Platform/Windows/WindowsInput.h"

// ---Geometry-------------
#include "Flux/Renderer/Geometry.h"
// ------------------------

// ---Renderer-------------
#include "Flux/Renderer/PerspectiveCamera.h"

#include "Flux/Renderer/RHIBuffer.h"
#include "Flux/Renderer/RHIPipeline.h"
#include "Flux/Renderer/RHIShader.h"
#include "Flux/Renderer/RHITexture.h"
#include "Flux/Renderer/RHIFence.h"
#include "Flux/Renderer/RHISemaphore.h"

#include "Flux/Renderer/RendererBackend.h"



// ------------------------

// ---Entry Point-----------
#include "Flux/Core/EntryPoint.h"

// -------------------------