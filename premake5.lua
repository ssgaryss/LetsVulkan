include "Dependencies.lua"

workspace "LetsVulkan"
	architecture "x64"
	startproject "VulkanTutorial"
	
	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
    include "VulkanTutorial/vendor/glfw"
group ""

group "Core"
	include "VulkanTutorial"
group ""