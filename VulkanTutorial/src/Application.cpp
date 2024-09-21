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
		createRenderPass();
		createGraphicsPipeline();
		createFramebuffers();
		createCommandPool();
		createCommandBuffer();
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
		vkDestroyCommandPool(m_LogicalDevice, m_GraphicsCommandPool, nullptr);
		for (const auto& SwapchainFramebuffer : m_SwapchainFramebuffers)
			vkDestroyFramebuffer(m_LogicalDevice, SwapchainFramebuffer, nullptr);
		vkDestroyPipeline(m_LogicalDevice, m_Pipeline, nullptr);
		vkDestroyPipelineLayout(m_LogicalDevice, m_PipelineLayout, nullptr);
		vkDestroyRenderPass(m_LogicalDevice, m_RenderPass, nullptr);
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

	void Application::createRenderPass()
	{
		std::cout << "Try to create a render pass ..." << "\n";
		VkAttachmentDescription AttachmentDescription{};
		AttachmentDescription.format = m_SwapchainFormat;
		AttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
		AttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;  // 加载时先clear
		AttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		AttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		AttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // 这里我们不care
		AttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // VkImageLayout (dont care)
		AttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // 用于交换链显示

		// Subpasses and Attachment references
		VkAttachmentReference AttachmentReference;
		AttachmentReference.attachment = 0;
		AttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription Subpass{}; // 这里我们只需要一个subpass渲染三角形
		Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		Subpass.colorAttachmentCount = 1;
		Subpass.pColorAttachments = &AttachmentReference;

		// RenderPass
		VkRenderPassCreateInfo RenderPassCreateInfo{};
		RenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		RenderPassCreateInfo.attachmentCount = 1;
		RenderPassCreateInfo.pAttachments = &AttachmentDescription;
		RenderPassCreateInfo.subpassCount = 1;
		RenderPassCreateInfo.pSubpasses = &Subpass;

		if (vkCreateRenderPass(m_LogicalDevice, &RenderPassCreateInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
			throw std::runtime_error("Failed to create render pass!");

		std::cout << "Success to create a render pass !" << "\n";
	}

	void Application::createGraphicsPipeline()
	{
		std::cout << "Try to create a pipeline ..." << "\n";
		auto VertexShaderCode = readFile("resources/shaders/spir-v/09_shader_base_vert.spv");
		auto FragmentShaderCode = readFile("resources/shaders/spir-v/09_shader_base_frag.spv");
		VkShaderModule VertexShaderModule = createShaderModule(VertexShaderCode);
		VkShaderModule FragmentShaderModule = createShaderModule(FragmentShaderCode);

		VkPipelineShaderStageCreateInfo VertexShaderStageCreateInfo{};
		VertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		VertexShaderStageCreateInfo.pSpecializationInfo = nullptr; // 可以指定shader中的常量值避免渲染时再赋值，提高效率！
		VertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		VertexShaderStageCreateInfo.module = VertexShaderModule;
		VertexShaderStageCreateInfo.pName = "main"; // shader主函数名

		VkPipelineShaderStageCreateInfo FragmentShaderStageCreateInfo{};
		FragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		FragmentShaderStageCreateInfo.pSpecializationInfo = nullptr;
		FragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		FragmentShaderStageCreateInfo.module = FragmentShaderModule;
		FragmentShaderStageCreateInfo.pName = "main";

		VkPipelineShaderStageCreateInfo ShaderStageCreateInfos[] = { VertexShaderStageCreateInfo, FragmentShaderStageCreateInfo };

		// VertexInput
		VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo{};
		VertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		VertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
		VertexInputStateCreateInfo.pVertexBindingDescriptions = nullptr; // 结构体包含：绑定信息，顶点或实例作为步长等
		VertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
		VertexInputStateCreateInfo.pVertexAttributeDescriptions = nullptr; //结构体包含：顶点属性布局、格式、偏移量等

		// Input Assembly
		VkPipelineInputAssemblyStateCreateInfo InputAssemblyStateCreateInfo{};
		InputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		InputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		InputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

		// Dynamic State
		std::vector<VkDynamicState> RequiredDynamicStates{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo DynamicStateCreateInfo{};
		DynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		DynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(RequiredDynamicStates.size());
		DynamicStateCreateInfo.pDynamicStates = RequiredDynamicStates.data();

		// Viewport and Scissors
		VkViewport Viewport;
		Viewport.x = 0;
		Viewport.y = 0;
		Viewport.width = static_cast<float>(m_SwapchainExtent.width);
		Viewport.height = static_cast<float>(m_SwapchainExtent.height);
		Viewport.minDepth = 0.0f;
		Viewport.maxDepth = 1.0f;
		VkRect2D Scissor;
		Scissor.offset = VkOffset2D(0, 0);
		Scissor.extent = m_SwapchainExtent;
		VkPipelineViewportStateCreateInfo ViewportStateCreateInfo{};
		ViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		ViewportStateCreateInfo.pScissors = &Scissor;
		ViewportStateCreateInfo.viewportCount = 1;
		ViewportStateCreateInfo.scissorCount = 1; // 有些GPU支持有多个Viewport和Scissor
		ViewportStateCreateInfo.pViewports = nullptr;
		ViewportStateCreateInfo.pScissors = nullptr; // 这里不需要设置，因为这两个已经指明为dynamic state，可以之后在运行时指定

		// Rasterizer
		VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo{};
		RasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		RasterizationStateCreateInfo.depthClampEnable = VK_FALSE; // 超出视锥部分是否进行clamp
		RasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE; // 是否不进行光栅化
		RasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
		RasterizationStateCreateInfo.lineWidth = 1.0f;
		RasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		RasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // 逆时针为正面
		RasterizationStateCreateInfo.depthBiasEnable = VK_FALSE; // 不进行深度偏移
		RasterizationStateCreateInfo.depthBiasClamp = 0.0f;
		RasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
		RasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;
		//Depth = Depth + depthBiasConstantFactor * constant + depthBiasSlopeFactor * slope (计算公式)

		// Mutisampling
		VkPipelineMultisampleStateCreateInfo MultisamplingCreateInfo{};
		MultisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		MultisamplingCreateInfo.sampleShadingEnable = VK_FALSE; // 是否超采样
		MultisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		MultisamplingCreateInfo.minSampleShading = 1.0f;
		MultisamplingCreateInfo.pSampleMask = nullptr;
		MultisamplingCreateInfo.alphaToCoverageEnable = VK_FALSE;
		MultisamplingCreateInfo.alphaToOneEnable = VK_FALSE;

		// Depth and Stencil testing
		VkPipelineDepthStencilStateCreateInfo DepthStencilStateCreateInfo{}; // 暂时不用

		// Color Blending
		VkPipelineColorBlendAttachmentState ColorBlendAttachment_1;
		ColorBlendAttachment_1.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
			| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		ColorBlendAttachment_1.blendEnable = VK_TRUE;  // 这里用的alpha blending
		ColorBlendAttachment_1.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		ColorBlendAttachment_1.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		ColorBlendAttachment_1.colorBlendOp = VK_BLEND_OP_ADD;
		ColorBlendAttachment_1.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		ColorBlendAttachment_1.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		ColorBlendAttachment_1.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo ColorBlendStateCreateInfo{};
		ColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		ColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
		ColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
		ColorBlendStateCreateInfo.attachmentCount = 1;
		ColorBlendStateCreateInfo.pAttachments = &ColorBlendAttachment_1;
		ColorBlendStateCreateInfo.blendConstants[0] = 0.0f;
		ColorBlendStateCreateInfo.blendConstants[1] = 0.0f;
		ColorBlendStateCreateInfo.blendConstants[2] = 0.0f;
		ColorBlendStateCreateInfo.blendConstants[3] = 0.0f; // 这里无用只有VK_BLEND_FACTOR_CONSTANT_COLOR或VK_BLEND_FACTOR_CONSTANT_ALPHA才需要设置

		// Pipeline Layout
		VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo{};
		PipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		PipelineLayoutCreateInfo.setLayoutCount = 0;
		PipelineLayoutCreateInfo.pSetLayouts = nullptr;
		PipelineLayoutCreateInfo.pushConstantRangeCount = 0;
		PipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(m_LogicalDevice, &PipelineLayoutCreateInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
			throw std::runtime_error("Failed to create pipeline layout!");

		// Pipline !
		VkGraphicsPipelineCreateInfo GraphicsPiplineCreateInfo{};
		GraphicsPiplineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		GraphicsPiplineCreateInfo.stageCount = 2;
		GraphicsPiplineCreateInfo.pStages = ShaderStageCreateInfos;
		GraphicsPiplineCreateInfo.pVertexInputState = &VertexInputStateCreateInfo;
		GraphicsPiplineCreateInfo.pInputAssemblyState = &InputAssemblyStateCreateInfo;
		GraphicsPiplineCreateInfo.pDynamicState = &DynamicStateCreateInfo;
		GraphicsPiplineCreateInfo.pViewportState = &ViewportStateCreateInfo;
		GraphicsPiplineCreateInfo.pRasterizationState = &RasterizationStateCreateInfo;
		GraphicsPiplineCreateInfo.pMultisampleState = &MultisamplingCreateInfo;
		GraphicsPiplineCreateInfo.pDepthStencilState = &DepthStencilStateCreateInfo;
		GraphicsPiplineCreateInfo.pColorBlendState = &ColorBlendStateCreateInfo;
		GraphicsPiplineCreateInfo.layout = m_PipelineLayout;
		GraphicsPiplineCreateInfo.renderPass = m_RenderPass;
		GraphicsPiplineCreateInfo.subpass = 0; // 只有一个subpass索引为0

		GraphicsPiplineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		GraphicsPiplineCreateInfo.basePipelineIndex = -1;

		if (vkCreateGraphicsPipelines(m_LogicalDevice, VK_NULL_HANDLE, 1,
			&GraphicsPiplineCreateInfo, nullptr, &m_Pipeline) != VK_SUCCESS)
			throw std::runtime_error("Failed to create graphics pipeline!");

		vkDestroyShaderModule(m_LogicalDevice, FragmentShaderModule, nullptr);
		vkDestroyShaderModule(m_LogicalDevice, VertexShaderModule, nullptr);
		std::cout << "Success to create a pipeline !" << "\n";
	}

	void Application::createFramebuffers()
	{
		std::cout << "Try to create swapchain framebuffers ..." << "\n";
		m_SwapchainFramebuffers.resize(m_SwapchainImageViews.size());
		for (size_t i = 0; i < m_SwapchainImageViews.size(); ++i) {
			VkImageView FramebufferImageViews[] = {
				m_SwapchainImageViews[i]
			};

			VkFramebufferCreateInfo FramebufferCreateInfo{};
			FramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			FramebufferCreateInfo.renderPass = m_RenderPass;
			FramebufferCreateInfo.attachmentCount = sizeof(FramebufferImageViews) / sizeof(VkImageView);
			FramebufferCreateInfo.pAttachments = FramebufferImageViews;
			FramebufferCreateInfo.width = m_SwapchainExtent.width;
			FramebufferCreateInfo.height = m_SwapchainExtent.height;
			FramebufferCreateInfo.layers = 1; // 通常为1，除非3D纹理或立体贴图

			if (vkCreateFramebuffer(m_LogicalDevice, &FramebufferCreateInfo, nullptr, &m_SwapchainFramebuffers[i]) != VK_SUCCESS)
				throw std::runtime_error("Failed to create framebuffer!");
			std::cout << std::format("Success to create SwapchainFramebuffer[{0}].", i) << "\n";
		}
		std::cout << "Success to create swapchain framebuffers !" << "\n";
	}

	void Application::createCommandPool()
	{
		std::cout << "Try to create a graphics command pool ..." << "\n";
		std::optional<uint32_t> GraphicsQueueIndice = findQueueFamilies(m_PhysicalDevice, VK_QUEUE_GRAPHICS_BIT);

		VkCommandPoolCreateInfo CommandPoolCreateInfo{};
		CommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		CommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		CommandPoolCreateInfo.queueFamilyIndex = GraphicsQueueIndice.value();

		if (vkCreateCommandPool(m_LogicalDevice, &CommandPoolCreateInfo, nullptr, &m_GraphicsCommandPool) != VK_SUCCESS)
			throw std::runtime_error("Failed to create command pool!");
		std::cout << "Success to create a graphics command pool !" << "\n";
	}

	void Application::createCommandBuffer()
	{
		std::cout << "Try to create a command buffer for graphics command pool ..." << "\n";
		VkCommandBufferAllocateInfo CommandBufferAllocateInfo{};
		CommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		CommandBufferAllocateInfo.commandPool = m_GraphicsCommandPool;
		CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		CommandBufferAllocateInfo.commandBufferCount = 1;
		std::cout << "Success to create a command buffer for graphics command pool !" << "\n";
	}

}