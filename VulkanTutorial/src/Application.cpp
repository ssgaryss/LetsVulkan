#include "Application.h"
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <exception>
#include <iostream>
#include <format>
#include <iomanip>
#include <numeric>
#include <algorithm>
#include <fstream>
#include <unordered_map>
#include <set>

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
		createSurface();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		//createRenderPass();
		createGraphicsPipeline();
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
		std::cout << "Try to clean up ..." << "\n";
		//vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
		//vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
		//vkDestroyFence(device, inFlightFence, nullptr);
		//vkDestroyCommandPool
		//vkDestroyFramebuffer
		vkDestroyPipeline(m_LogicalDevice, m_Pipeline, nullptr);
		//vkDestroyPipelineLayout
		//vkDestroyRenderPass
		for (const auto& View : m_SwapchainImageViews)
			vkDestroyImageView(m_LogicalDevice, View, nullptr);
		vkDestroySwapchainKHR(m_LogicalDevice, m_Swapchain, nullptr);
		vkDestroyDevice(m_LogicalDevice, nullptr);
		vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
		if (IsEnableValidationLayer) {
			auto Func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT");
			Func(m_Instance, m_DebugMessenger, nullptr);
		}
		vkDestroyInstance(m_Instance, nullptr);
		glfwDestroyWindow(m_Window);
		glfwTerminate();
		std::cout << "Success to clean up !" << "\n";
	}

	std::vector<VkExtensionProperties> Application::getSupportedInstanceExtensions()
	{
		uint32_t ExtensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionCount, nullptr);

		// 分配 VkExtensionProperties 数组来存储扩展信息
		std::vector<VkExtensionProperties> Extensions(ExtensionCount);
		// 第二次调用：获取扩展详细信息
		vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionCount, Extensions.data());

		return Extensions;
	}

	void Application::showExtensionInformation(const std::vector<VkExtensionProperties>& vExtensions)
	{
		for (const auto& Extension : vExtensions)
			std::cout << std::format("\t{} (version {})\n", Extension.extensionName, Extension.specVersion);
	}

	void Application::showExtensionInformation(const std::vector<const char*>& vExtensions)
	{
		for (const auto& Extension : vExtensions)
			std::cout << std::format("\t{} (version {})\n", Extension, Extension);
	}

	std::vector<VkExtensionProperties> Application::getSupportedDeviceExtensions(VkPhysicalDevice vPhysicalDevice)
	{
		uint32_t extensionCount = 0;
		vkEnumerateDeviceExtensionProperties(vPhysicalDevice, nullptr, &extensionCount, nullptr);

		// 分配 VkExtensionProperties 数组来存储扩展信息
		std::vector<VkExtensionProperties> extensions(extensionCount);
		// 第二次调用：获取扩展详细信息
		vkEnumerateDeviceExtensionProperties(vPhysicalDevice, nullptr, &extensionCount, extensions.data());
		return extensions;
	}

	std::vector<const char*> Application::getRequiredDeviceExtensions()
	{
		std::vector<const char*> RequiredDeviceExtensions{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
		return RequiredDeviceExtensions;
	}

	bool Application::checkRequiredDeviceExtensionsSupport(VkPhysicalDevice vPhysicalDevice, const std::vector<const char*>& vRequiredExtensions)
	{
		auto SupportedExtensions = getSupportedDeviceExtensions(vPhysicalDevice);
		std::set<std::string> RequiredExtensions(vRequiredExtensions.begin(), vRequiredExtensions.end());
		for (const auto& Extension : SupportedExtensions)
			RequiredExtensions.erase(Extension.extensionName);
		return RequiredExtensions.empty();
	}

	std::vector<const char*> Application::getRequiredIntanceExtensions()
	{
		// GLFW extentions
		uint32_t GLFWExtensionCount = 0;
		const char** GLFWExtensions;
		GLFWExtensions = glfwGetRequiredInstanceExtensions(&GLFWExtensionCount);
		std::vector<const char*> Extentions(GLFWExtensions, GLFWExtensions + GLFWExtensionCount);
		// Validation Layer extentions
		if (IsEnableValidationLayer)
			Extentions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		return Extentions;
	}

	bool Application::checkRequiredInstanceExtensionsSupport(const std::vector<const char*>& vRequiredExtensions)
	{
		auto SupportedExtensions = getSupportedInstanceExtensions();
		std::set<std::string> RequiredExtensions(vRequiredExtensions.begin(), vRequiredExtensions.end());
		for (const auto& Extension : SupportedExtensions)
			RequiredExtensions.erase(Extension.extensionName);
		return RequiredExtensions.empty();
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

	std::optional<uint32_t> Application::findQueueFamilies(VkPhysicalDevice vPhysicalDevice, VkQueueFlagBits vFlag)
	{
		std::optional<uint32_t> QueueIndice = std::nullopt;
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, queueFamilies.data());

		for (uint32_t i = 0; i < queueFamilyCount; ++i) {
			if (queueFamilies[i].queueFlags & vFlag)
				QueueIndice = i;
			if (QueueIndice.has_value())
				break;
		}
		return QueueIndice;
	}

	std::optional<uint32_t> Application::findPresentQueueFamilies(VkPhysicalDevice vPhysicalDevice)
	{
		std::optional<uint32_t> QueueIndice = std::nullopt;
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, queueFamilies.data());

		VkBool32 IsSupport = false;
		for (uint32_t i = 0; i < queueFamilyCount; ++i) {
			vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, i, m_Surface, &IsSupport);
			if (IsSupport)
				QueueIndice = i;
			if (QueueIndice.has_value())
				break;
		}
		return QueueIndice;
	}

	bool Application::checkRequiredQueueFamiliesSupport()
	{
		return m_GraphicsQueue && m_PresentQueue;
	}

	SwapChainSupportDetails Application::getSupportedSwapchainDetails(VkPhysicalDevice vPhysicalDevice, VkSurfaceKHR vSurface)
	{
		SwapChainSupportDetails SwapChainDetails;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vPhysicalDevice, vSurface, &SwapChainDetails.m_SurfaceCapabilities);
		uint32_t FormatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(vPhysicalDevice, vSurface, &FormatCount, nullptr);
		if (FormatCount > 0) {
			SwapChainDetails.m_SurfaceFormats.resize(FormatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(vPhysicalDevice, vSurface, &FormatCount, SwapChainDetails.m_SurfaceFormats.data());
		}
		uint32_t PresentModesCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(vPhysicalDevice, vSurface, &PresentModesCount, nullptr);
		if (PresentModesCount > 0) {
			SwapChainDetails.m_PresentModes.resize(PresentModesCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(vPhysicalDevice, vSurface, &PresentModesCount, SwapChainDetails.m_PresentModes.data());
		}
		return SwapChainDetails;
	}

	VkSurfaceFormatKHR Application::chooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& vSurfaceFormats)
	{
		for (const auto& SurfaceFormat : vSurfaceFormats) {
			if (SurfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
				SurfaceFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
				return SurfaceFormat;
		}
		std::cerr << "Do not support the surface format (VK_FORMAT_B8G8R8A8_SRGB & VK_COLORSPACE_SRGB_NONLINEAR_KHR), use a random format!\n";
		return vSurfaceFormats[0];
	}

	VkPresentModeKHR Application::chooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& vPresentModes)
	{
		for (const auto& PresentMode : vPresentModes) {
			if (PresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				return PresentMode;
		}
		std::cerr << "Do not support the present mode (VK_PRESENT_MODE_MAILBOX_KHR), use VK_PRESENT_MODE_FIFO_KHR instead!\n";
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D Application::chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& vCapabilities)
	{
		if (vCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return vCapabilities.currentExtent; // 当currentExtent中wide和height都为0xFFFFFFFF时currentExtent的数值才是允许修改的！
		}
		else {
			int Width, Height;
			glfwGetFramebufferSize(m_Window, &Width, &Height);
			VkExtent2D ActualExtent{
				static_cast<uint32_t>(Width),
				static_cast<uint32_t>(Height)
			};
			ActualExtent.width = std::clamp(ActualExtent.width, vCapabilities.minImageExtent.width, vCapabilities.maxImageExtent.width);
			ActualExtent.height = std::clamp(ActualExtent.height, vCapabilities.minImageExtent.height, vCapabilities.maxImageExtent.height);
			return ActualExtent;
		}
	}

	bool Application::checkSwapchainSupport(const SwapChainSupportDetails& vSwapchainDetails)
	{
		return !vSwapchainDetails.m_SurfaceFormats.empty() && !vSwapchainDetails.m_PresentModes.empty();
	}

	std::vector<char> Application::readFile(const std::filesystem::path& vPath)
	{
		std::ifstream InFileStream(vPath, std::ios_base::ate | std::ios_base::binary);
		if (!InFileStream)
			throw std::runtime_error(std::format(R"(Fail to open the file at "{0}".)", vPath.string()));
		size_t FileSize = static_cast<size_t>(InFileStream.tellg());
		std::vector<char> Code(FileSize);
		InFileStream.seekg(0);
		InFileStream.read(Code.data(), FileSize);
		InFileStream.close();
		return Code;
	}

	VkShaderModule Application::createShaderModule(const std::vector<char>& vCode)
	{
		VkShaderModuleCreateInfo ShaderModuleCreateInfo{};
		ShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		ShaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(vCode.data());
		ShaderModuleCreateInfo.codeSize = vCode.size();

		VkShaderModule ShaderModule;
		if (vkCreateShaderModule(m_LogicalDevice, &ShaderModuleCreateInfo, nullptr, &ShaderModule) != VK_SUCCESS)
			throw std::runtime_error("Failed to create shader module!");
		std::cout << "Success to create a shader module." << "\n";
		return ShaderModule;
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

		std::cout << "Available Vulkan instance extensions:\n";
		showExtensionInformation(getSupportedInstanceExtensions());
		std::cout << "Required Vulkan instance extensions:\n";
		auto RequiredInstanceExtensions = getRequiredIntanceExtensions();
		showExtensionInformation(RequiredInstanceExtensions);
		std::cout << "Satisfy the instance extensions requirements? " << std::boolalpha
			<< checkRequiredInstanceExtensionsSupport(RequiredInstanceExtensions) << std::noboolalpha << "\n";
		InstanceInfo.enabledExtensionCount = static_cast<uint32_t>(RequiredInstanceExtensions.size());
		InstanceInfo.ppEnabledExtensionNames = RequiredInstanceExtensions.data();

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
			//| VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
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

	void Application::createSurface()
	{
		std::cout << "Try to create surface of Win32 for Vulkan ..." << "\n";
		VkWin32SurfaceCreateInfoKHR CreateInfo{};
		CreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		CreateInfo.hwnd = glfwGetWin32Window(m_Window);
		CreateInfo.hinstance = GetModuleHandle(nullptr);
		if (vkCreateWin32SurfaceKHR(m_Instance, &CreateInfo, nullptr, &m_Surface) != VK_SUCCESS)
			throw std::runtime_error("Failed to create window surface!");
		std::cout << "Success to create surface of Win32 for Vulkan !" << "\n";
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

	void Application::createLogicalDevice()
	{
		std::cout << "Try to create logical device for Vulkan ..." << "\n";
		// Queues
		std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;
		std::optional<uint32_t> GraphicQueueIndice = findQueueFamilies(m_PhysicalDevice, VK_QUEUE_GRAPHICS_BIT);
		std::optional<uint32_t> PresentQueueIndice = findPresentQueueFamilies(m_PhysicalDevice);
		std::set<uint32_t> RequiredQueueFamiliesIndicies{ GraphicQueueIndice.value(), PresentQueueIndice.value() };
		float QueuePriorities = 1.0f; // 0.0f - 1.0f
		for (auto Indice : RequiredQueueFamiliesIndicies) {
			VkDeviceQueueCreateInfo QueueCreateInfo{};
			QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			QueueCreateInfo.queueCount = 1;
			QueueCreateInfo.queueFamilyIndex = Indice;
			QueueCreateInfo.pQueuePriorities = &QueuePriorities;
			QueueCreateInfos.emplace_back(QueueCreateInfo);
		}

		VkDeviceCreateInfo DeviceCreateInfo{};
		DeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		DeviceCreateInfo.pQueueCreateInfos = QueueCreateInfos.data();
		DeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueCreateInfos.size());
		VkPhysicalDeviceFeatures PhysicalDeviceFeatures{};
		DeviceCreateInfo.pEnabledFeatures = &PhysicalDeviceFeatures;

		std::cout << "Available device extensions:\n";
		showExtensionInformation(getSupportedDeviceExtensions(m_PhysicalDevice));
		std::cout << "Required device extensions:\n";
		auto RequiredDeviceExtensions = getRequiredDeviceExtensions();
		showExtensionInformation(RequiredDeviceExtensions);
		std::cout << "Satisfy the device extensions requirements? " << std::boolalpha
			<< checkRequiredDeviceExtensionsSupport(m_PhysicalDevice, RequiredDeviceExtensions) << std::noboolalpha << "\n";
		DeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(RequiredDeviceExtensions.size());
		DeviceCreateInfo.ppEnabledExtensionNames = RequiredDeviceExtensions.data();

		if (vkCreateDevice(m_PhysicalDevice, &DeviceCreateInfo, nullptr, &m_LogicalDevice) != VK_SUCCESS)
			throw std::runtime_error("Failed to create logical device!");
		vkGetDeviceQueue(m_LogicalDevice, GraphicQueueIndice.value(), 0, &m_GraphicsQueue); // 这里只有一个queue即index = 0
		vkGetDeviceQueue(m_LogicalDevice, PresentQueueIndice.value(), 0, &m_PresentQueue); // 这里只有一个queue即index = 0
		std::cout << "Statisfy the queue families requirements? " << std::boolalpha
			<< checkRequiredQueueFamiliesSupport() << std::noboolalpha << "\n";
		std::cout << "Success to create logical device for Vulkan !" << "\n";
	}

	void Application::createSwapChain()
	{
		std::cout << "Try to create swapchain for Vulkan ..." << "\n";
		SwapChainSupportDetails SwapChainSupportDetails = getSupportedSwapchainDetails(m_PhysicalDevice, m_Surface);
		VkSurfaceFormatKHR SurfaceFormat = chooseSwapchainSurfaceFormat(SwapChainSupportDetails.m_SurfaceFormats);
		VkPresentModeKHR PresentMode = chooseSwapchainPresentMode(SwapChainSupportDetails.m_PresentModes);
		VkExtent2D Extent = chooseSwapchainExtent(SwapChainSupportDetails.m_SurfaceCapabilities);
		uint32_t ImageCount = SwapChainSupportDetails.m_SurfaceCapabilities.minImageCount + 1; // +1是不想等待驱动程序完成内部操作才能获取下一张图片，增加渲染效率
		if (SwapChainSupportDetails.m_SurfaceCapabilities.minImageCount > 0
			&& ImageCount > SwapChainSupportDetails.m_SurfaceCapabilities.maxImageCount)
			ImageCount = SwapChainSupportDetails.m_SurfaceCapabilities.maxImageCount;

		VkSwapchainCreateInfoKHR SwapchainCreateInfo{};
		SwapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		SwapchainCreateInfo.surface = m_Surface;
		SwapchainCreateInfo.minImageCount = ImageCount;
		SwapchainCreateInfo.imageFormat = SurfaceFormat.format;
		SwapchainCreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
		SwapchainCreateInfo.presentMode = PresentMode;
		SwapchainCreateInfo.clipped = VK_TRUE; // true当渲染的部分在屏幕上被遮挡的部分不进行渲染，提高性能！
		SwapchainCreateInfo.imageExtent = Extent;
		SwapchainCreateInfo.imageArrayLayers = 1; // 通常都是1，除非如VR这种需要2张结果(左右眼)
		SwapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		std::optional<uint32_t> GraphicQueueIndice = findQueueFamilies(m_PhysicalDevice, VK_QUEUE_GRAPHICS_BIT);
		std::optional<uint32_t> PresentQueueIndice = findPresentQueueFamilies(m_PhysicalDevice);
		uint32_t QueueFamilyIndices[] = { GraphicQueueIndice.value(), PresentQueueIndice.value() };
		if (GraphicQueueIndice.value() != PresentQueueIndice.value()) {
			SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // 多个队列族共享
			SwapchainCreateInfo.queueFamilyIndexCount = 2;
			SwapchainCreateInfo.pQueueFamilyIndices = QueueFamilyIndices;
		}
		else {
			SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // 单个队列族独享
			SwapchainCreateInfo.queueFamilyIndexCount = 0;
			SwapchainCreateInfo.pQueueFamilyIndices = nullptr;
		}
		SwapchainCreateInfo.preTransform = SwapChainSupportDetails.m_SurfaceCapabilities.currentTransform;
		SwapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // 忽略Alpha，不需要与其他窗口物体进行混合
		SwapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(m_LogicalDevice, &SwapchainCreateInfo, nullptr, &m_Swapchain) != VK_SUCCESS)
			throw std::runtime_error("Failed to create swap chain!");

		uint32_t SwapchainImageCount = 0;
		vkGetSwapchainImagesKHR(m_LogicalDevice, m_Swapchain, &SwapchainImageCount, nullptr);
		m_SwapchainImages.resize(SwapchainImageCount);
		vkGetSwapchainImagesKHR(m_LogicalDevice, m_Swapchain, &SwapchainImageCount, m_SwapchainImages.data());
		m_SwapchainFormat = SurfaceFormat.format;
		m_SwapchainExtent = Extent;
		std::cout << "Success to create swapchain for Vulkan !" << "\n";
	}

	void Application::createImageViews()
	{
		std::cout << "Try to create swapchain image views ..." << "\n";
		m_SwapchainImageViews.resize(m_SwapchainImages.size());
		for (size_t i = 0; i < m_SwapchainImageViews.size(); ++i) {
			VkImageViewCreateInfo ImageViewCreateInfo{};
			ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			ImageViewCreateInfo.image = m_SwapchainImages[i];
			ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			ImageViewCreateInfo.format = m_SwapchainFormat;
			ImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			ImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			ImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			ImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY; // 即不进行映射置换，a通道就是image的a通道数据
			ImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
			ImageViewCreateInfo.subresourceRange.levelCount = 1;
			ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
			ImageViewCreateInfo.subresourceRange.layerCount = 1;
			if (vkCreateImageView(m_LogicalDevice, &ImageViewCreateInfo, nullptr, &m_SwapchainImageViews[i]) != VK_SUCCESS)
				throw std::runtime_error("Failed to create image views!");
			std::cout << std::format("Success to create SwapchainImageView[{0}].", i) << "\n";
		}
		std::cout << "Success to create swapchain image views !" << "\n";
	}

	void Application::createGraphicsPipeline()
	{
		std::cout << "Try to create a pipeline ..." << "\n";
		auto VertexShaderCode = readFile("resources/shaders/spir-v/09_shader_base_vert.spv");
		auto FragmentShaderCode = readFile("resources/shaders/spir-v/09_shader_base_frag.spv");
		VkShaderModule VertexShaderModule = createShaderModule(VertexShaderCode);
		VkShaderModule FragmentShaderModule = createShaderModule(FragmentShaderCode);

		vkDestroyShaderModule(m_LogicalDevice, FragmentShaderModule, nullptr);
		vkDestroyShaderModule(m_LogicalDevice, VertexShaderModule, nullptr);
		std::cout << "Success to create a pipeline !" << "\n";
	}

}