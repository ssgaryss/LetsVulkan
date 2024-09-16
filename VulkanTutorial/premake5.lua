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

    filter "system:windows"
        systemversion "latest"
        defines "VK_USE_PLATFORM_WIN32_KHR"

    filter "configurations:Debug"
        defines "VK_TUTORIAL_DEBUG"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines "VK_TUTORIAL_RELEASE"
        runtime "Release"
        optimize "On"

    filter "configurations:Dist"
        defines "VK_TUTORIAL_DIST"
        runtime "Release"
        optimize "On"