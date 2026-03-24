#pragma once


// For use Flux applications

#include "Flux/Application.h"
#include "Flux/Layer.h"
#include "Flux/Log.h"

#include "Flux/Input.h"
#include "Flux/KeyCodes.h"
#include "Flux/MouseButtonCodes.h"

#include "Flux/ImGui/ImGuiLayer.h"


// ---Geometry-------------
#include "Flux/Geometry/Vertex.h"
// ------------------------

// ---Renderer-------------
#include "Flux/Renderer/Shader.h"
#include "Flux/Renderer/Pipeline.h"
#include "Flux/Renderer/GraphicsContext.h"
#include "Flux/Renderer/RendererAPI.h"
#include "Flux/Renderer/Buffer.h"


#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanPipeline.h"
#include "Platform/Vulkan/VulkanBuffer.h"
// ------------------------

// ---Entry Point-----------
#include "Flux/EntryPoint.h"


// -------------------------