#pragma once


// For use Flux applications

#include "Flux/Application.h"
#include "Flux/Layer.h"
#include "Flux/Log.h"

#include "Flux/Input.h"
#include "Flux/KeyCodes.h"
#include "Flux/MouseButtonCodes.h"

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

#include "Flux/Renderer/RendererBackend.h"

#include "Platform/Vulkan/VulkanRenderPass.h"

// ------------------------

// ---Entry Point-----------
#include "Flux/EntryPoint.h"


// -------------------------