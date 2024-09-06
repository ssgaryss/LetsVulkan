project "VulkanTutorial"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "On"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs
    {
        "%{IncludeDir.glfw}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.VulkanSDK}"
    }

    links
    {
        "glfw",
        "%{Library.Vulkan}"
    }

    defines
    {
        "GLFW_INCLUDE_VULKAN",
        "GLM_FORCE_RADIANS",
        "GLM_FORCE_DEPTH_ZERO_TO_ONE"
    }

    filter "configurations:Debug"
        defines "PIKA_DEBUG"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines "PIKA_RELEASE"
        runtime "Release"
        optimize "On"

    filter "configurations:Dist"
        defines "PIKA_DIST"
        runtime "Release"
        optimize "On"