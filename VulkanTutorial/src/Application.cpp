#include "Application.h"
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <exception>
#include <iostream>
#include <format>
#include <unordered_map>

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
		createInstance();
		setupDebugMessenger();
		pickPhysicalDevice();
		//createSurface();
		//createLogicalDevice();
		//createSwapChain();
		//createImageViews();
		//createRenderPass();
		//createGraphicsPipeline();
		//createFramebuffers();
		//createCommandPool();
		//createCommandBuffer();
		//createSyncObjects();
	}

	void Application::mainLoop()
	{
		while (!glfwWindowShouldClose(m_Window)) {
			glfwPollEvents();
			//drawFrame();
		}
		//vkDeviceWaitIdle(device);
	}

	void Application::cleanup()
	{
		//vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
		//vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
		//vkDestroyFence(device, inFlightFence, nullptr);
		//vkDestroyCommandPool
		//vkDestroyFramebuffer
		//vkDestroyPipeline
		//vkDestroyPipelineLayout
		//vkDestroyRenderPass
		//vkDestroyImageView(device, imageView, nullptr);
		//vkDestroySwapchainKHR
		//vkDestroyDevice(device, nullptr);
		//vkDestroySurfaceKHR(instance, surface, nullptr);
		if (IsEnableValidationLayer) {
			auto Func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT");
			Func(m_Instance, m_DebugMessenger, nullptr);
		}
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

	std::vector<const char*> Application::getRequiredExtentions()
	{
		uint32_t GLFWExtentionCount = 0;
		const char** GLFWExtentions;
		GLFWExtentions = glfwGetRequiredInstanceExtensions(&GLFWExtentionCount);

		std::vector<const char*> Extentions(GLFWExtentions, GLFWExtentions + GLFWExtentionCount);
		if (IsEnableValidationLayer)
			Extentions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		return Extentions;
	}

	uint32_t Application::ratePhysicalDevice(VkPhysicalDevice vPhysicalDevice)
	{
		uint32_t Score = 0;
		VkPhysicalDeviceProperties Properties;
		VkPhysicalDeviceFeatures Features;
		vkGetPhysicalDeviceProperties(vPhysicalDevice, &Properties);
		vkGetPhysicalDeviceFeatures(vPhysicalDevice, &Features);
		if (Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) // 独立显卡
			Score += 1000;
		Score += Properties.limits.maxImageDimension2D;                    // Texture最大size支持
		if (!Features.geometryShader)                                      // 是否支持Geometry Shader
			return 0;
		return Score;
	}

	void Application::createInstance()
	{
		std::cout << "Try to create Vulkan instance ..." << "\n";
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

		showExtentionInformation();
		auto Extension = getRequiredExtentions();
		InstanceInfo.enabledExtensionCount = static_cast<uint32_t>(Extension.size());
		InstanceInfo.ppEnabledExtensionNames = Extension.data();
		std::cout << "Require " << InstanceInfo.enabledExtensionCount << " extensions\n";
		for (const auto& it : Extension)
			std::cout << std::format("\t{}\n", it);

		InstanceInfo.enabledLayerCount = 0;

		VkResult Result = vkCreateInstance(&InstanceInfo, nullptr, &m_Instance);

		if (Result != VK_SUCCESS)
			throw std::runtime_error("Fail to create vulkan instance !");
		else
			std::cout << "Success to create Vulkan instance !" << "\n";
	}

	void Application::setupDebugMessenger()
	{
		std::cout << "Try to set up Vulkan debug messenger ..." << "\n";
		VkDebugUtilsMessengerCreateInfoEXT CreateInfo{};
		CreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		CreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
		CreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		CreateInfo.pfnUserCallback = [](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageTypes,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData) -> decltype(auto) {
				std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
				return VK_FALSE;
			};
		// vkCreateDebugUtilsMessengerEXT是一个扩展函数，并没有默认导入，因此需要显示手动导入
		auto Func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT");
		if (!Func || Func(m_Instance, &CreateInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
			throw std::runtime_error("Failed to set up debug messenger!");
		else
			std::cout << "Success to set up Vulkan debug messenger !" << "\n";
	}

	void Application::pickPhysicalDevice()
	{
		std::cout << "Try to pick physical device for Vulkan ..." << "\n";
		uint32_t PhysicalDeviceCount = 0;
		vkEnumeratePhysicalDevices(m_Instance, &PhysicalDeviceCount, nullptr);
		if (PhysicalDeviceCount == 0)
			throw std::runtime_error("Failed to find GPUs with Vulkan support!");
		std::vector<VkPhysicalDevice> PhysicalDeveices(PhysicalDeviceCount);
		vkEnumeratePhysicalDevices(m_Instance, &PhysicalDeviceCount, PhysicalDeveices.data());
		std::cout << "Available physical devices:\n";
		std::unordered_map<uint32_t, VkPhysicalDevice> ScoresDevices;
		uint32_t MaxScore = 0;
		for (const auto& Device : PhysicalDeveices) {
			auto Score = ratePhysicalDevice(Device);
			ScoresDevices[Score] = Device;
			MaxScore = std::max(MaxScore, Score);
			VkPhysicalDeviceProperties PhysicalDeviceProperties;
			vkGetPhysicalDeviceProperties(Device, &PhysicalDeviceProperties);
			std::cout << std::format("\t{}\n", PhysicalDeviceProperties.deviceName);
		}
		m_PhysicalDevice = ScoresDevices[MaxScore];
		VkPhysicalDeviceProperties ChosenPhysicalDeviceProperties;
		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &ChosenPhysicalDeviceProperties);
		if (m_PhysicalDevice == VK_NULL_HANDLE)
			throw std::runtime_error("Failed to find GPUs with Vulkan support!");
		std::cout << "Success to pick physical device " << ChosenPhysicalDeviceProperties.deviceName << " for Vulkan !" << "\n";
	}

}