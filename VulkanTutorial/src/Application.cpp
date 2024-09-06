#include "Application.h"
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <exception>
#include <iostream>
#include <format>
#include <vector>

namespace VulkanTutorial {

	void Application::run()
	{
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

	void Application::initWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		m_Window = glfwCreateWindow(m_Width, m_Height, "VulkanTutorial", nullptr, nullptr);
	}

	void Application::initVulkan()
	{
		showExtentionInformation();
		createInstance();
	}
	
	void Application::mainLoop()
	{
		while (!glfwWindowShouldClose(m_Window)) {
			glfwPollEvents();
		}
	}
	
	void Application::cleanup()
	{
		vkDestroyInstance(m_Instance, nullptr);
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	void Application::showExtentionInformation()
	{
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		std::cout << extensionCount << " extensions supported\n";

		// 分配 VkExtensionProperties 数组来存储扩展信息
		std::vector<VkExtensionProperties> extensions(extensionCount);
		// 第二次调用：获取扩展详细信息
		vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions.data());

		// 输出所有扩展名称
		std::cout << "Available Vulkan instance extensions:\n";
		for (uint32_t i = 0; i < extensionCount; i++) {
			std::cout << std::format("\t{} (version {})\n", extensions[i].extensionName, extensions[i].specVersion);
		}
	}

	void Application::createInstance()
	{
		VkApplicationInfo AppInfo{};
		AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		AppInfo.pApplicationName = "VulkanTutorial";
		AppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		AppInfo.pEngineName = "No Engine";
		AppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		AppInfo.apiVersion = VK_API_VERSION_1_3;

		VkInstanceCreateInfo InstanceInfo{};
		InstanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		InstanceInfo.pApplicationInfo = &AppInfo;

		uint32_t GLFWExtentionCount = 0;
		const char** GLFWExtentions;
		GLFWExtentions = glfwGetRequiredInstanceExtensions(&GLFWExtentionCount);

		InstanceInfo.enabledExtensionCount = GLFWExtentionCount;
		InstanceInfo.ppEnabledExtensionNames = GLFWExtentions;

		InstanceInfo.enabledLayerCount = 0;

		VkResult Result = vkCreateInstance(&InstanceInfo, nullptr, &m_Instance);

		if (Result != VK_SUCCESS)
			throw std::runtime_error("Fail to create vulkan instance !");
	}

}