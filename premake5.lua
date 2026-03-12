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

IncludeDir = {}
IncludeDir["GLFW"] = "Flux/vendor/GLFW/include"
IncludeDir["Vulkan"] = "D:/VulkanSDK/1.4.335.0/Include"
IncludeDir["ImGui"] = "Flux/vendor/imgui"
IncludeDir["glm"] = "Flux/vendor/glm"

include "Flux/vendor/GLFW"
include "Flux/vendor/imgui"

-- startproject "Sandbox"

project "Flux"
	location "Flux"
	kind "SharedLib"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "flpch.h"
	pchsource "Flux/src/flpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Vulkan}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}"
	}

	links
	{
		"GLFW",
		"ImGui"
	}

	filter "system:windows"
		cppdialect "C++20"
		staticruntime "On"
		systemversion "latest"
		buildoptions { "/utf-8" }  

		defines 
		{
			"FL_PLATFORM_WINDOWS",
			"FL_BUILD_DLL"
		}

		postbuildcommands 
		{
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox")
		}

	filter "configurations:Debug"
		defines { "FL_DEBUG" }
		buildoptions "/MDd"
		symbols "On"

	filter "configurations:Release"
		defines { "FL_RELEASE" }
		buildoptions "/MD"
		optimize "On"

	filter "configurations:Dist"
		defines { "FL_DIST" }
		buildoptions "/MD"
		optimize "On"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"

	language "C++"

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
		"Flux/vendor/spdlog/include",
		"%{IncludeDir.glm}"
	}

	links 
	{
		"Flux"
	}

	filter "system:windows"
		cppdialect "C++20"
		staticruntime "On"
		systemversion "latest"
		buildoptions { "/utf-8" }  

		defines 
		{
			"FL_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines { "FL_DEBUG" }
		buildoptions "/MDd"
		symbols "On"

	filter "configurations:Release"
		defines { "FL_RELEASE" }
		buildoptions "/MD"
		optimize "On"

	filter "configurations:Dist"
		defines { "FL_DIST" }
		buildoptions "/MD"
		optimize "On"
