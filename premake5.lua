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
    -- TODO
group ""

group "Core"
	include "VulkanTutorial"
group ""