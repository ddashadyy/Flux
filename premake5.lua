workspace "Flux" 
	architecture "x64"
	startproject "Sandbox"

	configurations 
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

LibDir = {}
LibDir["Vulkan"] = "D:/VulkanSDK/1.4.335.0/Lib"

IncludeDir = {}
IncludeDir["spdlog"]  = "Flux/vendor/spdlog/include"
IncludeDir["GLFW"] = "Flux/vendor/GLFW/include"
IncludeDir["Vulkan"] = "D:/VulkanSDK/1.4.335.0/Include"
IncludeDir["ImGui"] = "Flux/vendor/imgui"
IncludeDir["glm"] = "Flux/vendor/glm"
IncludeDir["vma"] = "Flux/vendor/vma/include"
IncludeDir["stb"] = "Flux/vendor/stb"
IncludeDir["tinyobjloader"] = "Flux/vendor/tinyobjloader"
IncludeDir["tinygltf"] = "Flux/vendor/tinygltf/include"
IncludeDir["json"] = "Flux/vendor/json/include"
IncludeDir["ImGuiFileDialog"] = "Flux/vendor/imguifiledialog/include"
IncludeDir["ImGuizmo"] = "Flux/vendor/imguizmo/include"


include "Flux/vendor/GLFW"
include "Flux/vendor/imgui"
include "Flux/vendor/imguifiledialog"


project "Flux"
	location "Flux"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "flpch.h"
	pchsource "Flux/src/flpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl",
		"%{prj.name}/vendor/stb/**.h",
		"%{prj.name}/vendor/tinyobjloader/**.h",
		"%{prj.name}/vendor/tinygltf/**.h",
		"%{prj.name}/vendor/imguizmo/**.h",
		"%{prj.name}/vendor/imguizmo/**.cpp",
		"%{prj.name}/vendor/json/include/**.h"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Vulkan}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.vma}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.tinyobjloader}",
		"%{IncludeDir.tinygltf}",
		"%{IncludeDir.json}",
		"%{IncludeDir.ImGuiFileDialog}",
		"%{IncludeDir.ImGuizmo}"
	}

	links
	{
		"GLFW",
		"ImGui",
		"%{LibDir.Vulkan}/vulkan-1.lib",
		"ImGuiFileDialog"
	}

	filter "files:Flux/vendor/ImGuizmo/**.cpp"
    	enablepch "Off"
	filter {}

	filter "system:windows"
		systemversion "latest"
		buildoptions { "/utf-8" }  

		defines 
		{
			"FL_PLATFORM_WINDOWS",
		}

	filter "configurations:Debug"
		defines { "FL_DEBUG", "FL_ENABLE_ASSERTS" }
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines { "FL_RELEASE" }
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines { "FL_DIST" }
		runtime "Release"
		optimize "on"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	
	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Flux/src",
		"Flux/vendor",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Vulkan}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.vma}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.tinyobjloader}",
		"%{IncludeDir.tinygltf}",
		"%{IncludeDir.json}",
		"%{IncludeDir.ImGuiFileDialog}",
		"%{IncludeDir.ImGuizmo}"
	}

	links 
	{
		"Flux"
	}

	filter "system:windows"
		systemversion "latest"
		buildoptions { "/utf-8" }  
		defines { "FL_PLATFORM_WINDOWS" }

	filter "configurations:Debug"
		defines { "FL_DEBUG" }
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines { "FL_RELEASE" }
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines { "FL_DIST" }
		runtime "Release"
		optimize "on"
