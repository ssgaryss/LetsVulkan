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
		glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

		m_Window = glfwCreateWindow(m_Width, m_Height, "VulkanTutorial", nullptr, nullptr);
		glfwSetWindowUserPointer(m_Window, this);
		glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
			auto App = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
			App->m_IsWindowResize = true;
			});
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
			auto App = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
			App->m_Width = static_cast<uint32_t>(width);
			App->m_Height = static_cast<uint32_t>(height);
			if (width == 0 || height == 0)
				App->m_IsMinimized = true;
			else
				App->m_IsMinimized = false;
			});
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
		createGraphicsCommandPool();
		createGraphicsCommandBuffers();
		createSyncObjects();
	}

	void Application::mainLoop()
	{
		while (!glfwWindowShouldClose(m_Window)) {
			if (!m_IsMinimized) {
				float Time = m_Timer.ellapseMilliseconds();
				float DeltaTime = Time - m_LastFrameTime;
				m_LastFrameTime = Time;

				drawFrame(DeltaTime);
			}
			glfwPollEvents();
		}
		vkDeviceWaitIdle(m_LogicalDevice); // ��Ӧ�ó�����ͣ��ֱ�������ڸ��豸��VkDevice�����ύ�����ִ����ϡ�
	}

	void Application::cleanup()
	{
		std::cout << "Try to clean up ..." << "\n";
		for (size_t i = 0; i < m_MaxFrameInFlight; ++i) {
			vkDestroyFence(m_LogicalDevice, m_InFlightFence[i], nullptr);
			vkDestroySemaphore(m_LogicalDevice, m_RenderFinishedSemaphore[i], nullptr);
			vkDestroySemaphore(m_LogicalDevice, m_ImageAvailableSemaphore[i], nullptr);
		}
		vkDestroyCommandPool(m_LogicalDevice, m_GraphicsCommandPool, nullptr);
		vkDestroyPipeline(m_LogicalDevice, m_Pipeline, nullptr);
		vkDestroyPipelineLayout(m_LogicalDevice, m_PipelineLayout, nullptr);
		vkDestroyRenderPass(m_LogicalDevice, m_RenderPass, nullptr);
		cleanupSwapchain();
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

		// ���� VkExtensionProperties �������洢��չ��Ϣ
		std::vector<VkExtensionProperties> Extensions(ExtensionCount);
		// �ڶ��ε��ã���ȡ��չ��ϸ��Ϣ
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

		// ���� VkExtensionProperties �������洢��չ��Ϣ
		std::vector<VkExtensionProperties> extensions(extensionCount);
		// �ڶ��ε��ã���ȡ��չ��ϸ��Ϣ
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
		if (Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) // �����Կ�
			Score += 1000;
		Score += Properties.limits.maxImageDimension2D;                    // Texture���size֧��
		if (!Features.geometryShader)                                      // �Ƿ�֧��Geometry Shader
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
			return vCapabilities.currentExtent; // ��currentExtent��wide��height��Ϊ0xFFFFFFFFʱcurrentExtent����ֵ���������޸ĵģ�
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

	void Application::recreateSwapchain()
	{
		std::cout << "Try to recreate swapchian ..." << "\n";
		vkDeviceWaitIdle(m_LogicalDevice);

		cleanupSwapchain();
		createSwapChain();
		createImageViews();
		// ������ʵ��Ӧ��recreate render pass(��)
		createFramebuffers();
		std::cout << "Success to recreate swapchian !" << "\n";
	}

	void Application::cleanupSwapchain()
	{
		for (const auto& SwapchainFramebuffer : m_SwapchainFramebuffers)
			vkDestroyFramebuffer(m_LogicalDevice, SwapchainFramebuffer, nullptr);
		for (const auto& View : m_SwapchainImageViews)
			vkDestroyImageView(m_LogicalDevice, View, nullptr);
		vkDestroySwapchainKHR(m_LogicalDevice, m_Swapchain, nullptr);
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

	void Application::recordCommandBuffer(VkCommandBuffer vCommandBuffer, uint32_t vImageIndex)
	{
		std::cout << "Try to record commands to a command buffer ..." << "\n";
		VkCommandBufferBeginInfo CommandBufferBeginInfo{};
		CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		CommandBufferBeginInfo.flags = 0;
		CommandBufferBeginInfo.pInheritanceInfo = nullptr; // �μ�����������Ҫָ��

		if (vkBeginCommandBuffer(vCommandBuffer, &CommandBufferBeginInfo) != VK_SUCCESS)
			throw std::runtime_error("Failed to begin recording command buffer!");
		std::cout << "cmd : vkBeginCommandBuffer" << "\n";

		VkRenderPassBeginInfo RenderPassBeginInfo{};
		RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		RenderPassBeginInfo.renderPass = m_RenderPass;
		RenderPassBeginInfo.framebuffer = m_SwapchainFramebuffers[vImageIndex];
		RenderPassBeginInfo.renderArea.offset = { 0, 0 };
		RenderPassBeginInfo.renderArea.extent = m_SwapchainExtent;
		VkClearValue ClearColor{ {{0.0f, 0.0f, 0.0f, 1.0f}} };
		RenderPassBeginInfo.clearValueCount = 1;
		RenderPassBeginInfo.pClearValues = &ClearColor;

		vkCmdBeginRenderPass(vCommandBuffer, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		std::cout << "cmd : vkCmdBeginRenderPass" << "\n";
		vkCmdBindPipeline(vCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
		std::cout << "cmd : vkCmdBindPipeline" << "\n";

		// Dynamic States settings
		VkViewport Viewport;
		Viewport.x = 0.0f;
		Viewport.y = 0.0f;
		Viewport.width = static_cast<float>(m_SwapchainExtent.width);
		Viewport.height = static_cast<float>(m_SwapchainExtent.height);
		Viewport.minDepth = 0.0f;
		Viewport.maxDepth = 1.0f;
		vkCmdSetViewport(vCommandBuffer, 0, 1, &Viewport); // �ڶ����������ӿ�index��pipline�������ӿ�
		std::cout << "cmd : vkCmdSetViewport" << "\n";

		VkRect2D Scissor;
		Scissor.offset = { 0, 0 };
		Scissor.extent = m_SwapchainExtent;
		vkCmdSetScissor(vCommandBuffer, 0, 1, &Scissor);
		std::cout << "cmd : vkCmdSetScissor" << "\n";

		vkCmdDraw(vCommandBuffer, 3, 1, 0, 0);
		std::cout << "cmd : vkCmdDraw" << "\n";

		vkCmdEndRenderPass(vCommandBuffer);
		std::cout << "cmd : vkCmdEndRenderPass" << "\n";

		if (vkEndCommandBuffer(vCommandBuffer) != VK_SUCCESS)
			throw std::runtime_error("Failed to record command buffer!");
		std::cout << "cmd : vkEndCommandBuffer" << "\n";

		std::cout << "Success to recording commands to a command buffer !" << "\n";
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
		// vkCreateDebugUtilsMessengerEXT��һ����չ��������û��Ĭ�ϵ��룬�����Ҫ��ʾ�ֶ�����
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
		vkGetDeviceQueue(m_LogicalDevice, GraphicQueueIndice.value(), 0, &m_GraphicsQueue); // ����ֻ��һ��queue��index = 0
		vkGetDeviceQueue(m_LogicalDevice, PresentQueueIndice.value(), 0, &m_PresentQueue); // ����ֻ��һ��queue��index = 0
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
		uint32_t ImageCount = SwapChainSupportDetails.m_SurfaceCapabilities.minImageCount + 1; // +1�ǲ���ȴ�������������ڲ��������ܻ�ȡ��һ��ͼƬ��������ȾЧ��
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
		SwapchainCreateInfo.clipped = VK_TRUE; // true����Ⱦ�Ĳ�������Ļ�ϱ��ڵ��Ĳ��ֲ�������Ⱦ��������ܣ�
		SwapchainCreateInfo.imageExtent = Extent;
		SwapchainCreateInfo.imageArrayLayers = 1; // ͨ������1��������VR������Ҫ2�Ž��(������)
		SwapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		std::optional<uint32_t> GraphicQueueIndice = findQueueFamilies(m_PhysicalDevice, VK_QUEUE_GRAPHICS_BIT);
		std::optional<uint32_t> PresentQueueIndice = findPresentQueueFamilies(m_PhysicalDevice);
		uint32_t QueueFamilyIndices[] = { GraphicQueueIndice.value(), PresentQueueIndice.value() };
		if (GraphicQueueIndice.value() != PresentQueueIndice.value()) {
			SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // ��������干��
			SwapchainCreateInfo.queueFamilyIndexCount = 2;
			SwapchainCreateInfo.pQueueFamilyIndices = QueueFamilyIndices;
		}
		else {
			SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // �������������
			SwapchainCreateInfo.queueFamilyIndexCount = 0;
			SwapchainCreateInfo.pQueueFamilyIndices = nullptr;
		}
		SwapchainCreateInfo.preTransform = SwapChainSupportDetails.m_SurfaceCapabilities.currentTransform;
		SwapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // ����Alpha������Ҫ����������������л��
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
			ImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY; // ��������ӳ���û���aͨ������image��aͨ������
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
		AttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;  // ����ʱ��clear
		AttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		AttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		AttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // �������ǲ�care
		AttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // VkImageLayout (dont care)
		AttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // ���ڽ�������ʾ

		// Subpasses and Attachment references
		VkAttachmentReference AttachmentReference;
		AttachmentReference.attachment = 0;
		AttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription Subpass{}; // ��������ֻ��Ҫһ��subpass��Ⱦ������
		Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		Subpass.colorAttachmentCount = 1;
		Subpass.pColorAttachments = &AttachmentReference;

		VkSubpassDependency SubpassDependecy{};
		SubpassDependecy.srcSubpass = VK_SUBPASS_EXTERNAL; // ��ʾ��������Ⱦͨ����Ĳ���������һ֡����Ⱦ���
		SubpassDependecy.dstSubpass = 0; // Ŀ����ͨ��������
		SubpassDependecy.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // ��һ�׶ε���ɫ����׶Σ����Ｔ��һ֡
		SubpassDependecy.srcAccessMask = 0;
		SubpassDependecy.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		SubpassDependecy.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		// RenderPass
		VkRenderPassCreateInfo RenderPassCreateInfo{};
		RenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		RenderPassCreateInfo.attachmentCount = 1;
		RenderPassCreateInfo.pAttachments = &AttachmentDescription;
		RenderPassCreateInfo.subpassCount = 1;
		RenderPassCreateInfo.pSubpasses = &Subpass;
		RenderPassCreateInfo.dependencyCount = 1;
		RenderPassCreateInfo.pDependencies = &SubpassDependecy;

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
		VertexShaderStageCreateInfo.pSpecializationInfo = nullptr; // ����ָ��shader�еĳ���ֵ������Ⱦʱ�ٸ�ֵ�����Ч�ʣ�
		VertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		VertexShaderStageCreateInfo.module = VertexShaderModule;
		VertexShaderStageCreateInfo.pName = "main"; // shader��������

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
		auto VertexBindingDescription = Vertex::getBindingDescription();
		VertexInputStateCreateInfo.pVertexBindingDescriptions = &VertexBindingDescription;           // �ṹ�����������Ϣ�������ʵ����Ϊ������
		VertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
		auto VertexArributeDescriptions = Vertex::getAttributeDescriptions();
		VertexInputStateCreateInfo.pVertexAttributeDescriptions = VertexArributeDescriptions.data(); //�ṹ��������������Բ��֡���ʽ��ƫ������

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
		ViewportStateCreateInfo.scissorCount = 1; // ��ЩGPU֧���ж��Viewport��Scissor
		ViewportStateCreateInfo.pViewports = nullptr;
		ViewportStateCreateInfo.pScissors = nullptr; // ���ﲻ��Ҫ���ã���Ϊ�������Ѿ�ָ��Ϊdynamic state������֮��������ʱָ��

		// Rasterizer
		VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo{};
		RasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		RasterizationStateCreateInfo.depthClampEnable = VK_FALSE; // ������׶�����Ƿ����clamp
		RasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE; // �Ƿ񲻽��й�դ��
		RasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
		RasterizationStateCreateInfo.lineWidth = 1.0f;
		RasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		RasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE; // ˳ʱ��Ϊ����
		RasterizationStateCreateInfo.depthBiasEnable = VK_FALSE; // ���������ƫ��
		RasterizationStateCreateInfo.depthBiasClamp = 0.0f;
		RasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
		RasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;
		//Depth = Depth + depthBiasConstantFactor * constant + depthBiasSlopeFactor * slope (���㹫ʽ)

		// Mutisampling
		VkPipelineMultisampleStateCreateInfo MultisamplingCreateInfo{};
		MultisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		MultisamplingCreateInfo.sampleShadingEnable = VK_FALSE; // �Ƿ񳬲���
		MultisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		MultisamplingCreateInfo.minSampleShading = 1.0f;
		MultisamplingCreateInfo.pSampleMask = nullptr;
		MultisamplingCreateInfo.alphaToCoverageEnable = VK_FALSE;
		MultisamplingCreateInfo.alphaToOneEnable = VK_FALSE;

		// Depth and Stencil testing
		VkPipelineDepthStencilStateCreateInfo DepthStencilStateCreateInfo{}; // ��ʱ����

		// Color Blending
		VkPipelineColorBlendAttachmentState ColorBlendAttachment_1;
		ColorBlendAttachment_1.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
			| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		ColorBlendAttachment_1.blendEnable = VK_TRUE;  // �����õ�alpha blending
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
		ColorBlendStateCreateInfo.blendConstants[3] = 0.0f; // ��������ֻ��VK_BLEND_FACTOR_CONSTANT_COLOR��VK_BLEND_FACTOR_CONSTANT_ALPHA����Ҫ����

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
		GraphicsPiplineCreateInfo.subpass = 0; // ֻ��һ��subpass����Ϊ0

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
			FramebufferCreateInfo.layers = 1; // ͨ��Ϊ1������3D�����������ͼ

			if (vkCreateFramebuffer(m_LogicalDevice, &FramebufferCreateInfo, nullptr, &m_SwapchainFramebuffers[i]) != VK_SUCCESS)
				throw std::runtime_error("Failed to create framebuffer!");
			std::cout << std::format("Success to create SwapchainFramebuffer[{0}].", i) << "\n";
		}
		std::cout << "Success to create swapchain framebuffers !" << "\n";
	}

	void Application::createGraphicsCommandPool()
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

	void Application::createGraphicsCommandBuffers()
	{
		std::cout << "Try to allocate a command buffers for graphics command pool ..." << "\n";
		m_GraphicsCommandBuffer.resize(m_MaxFrameInFlight);
		VkCommandBufferAllocateInfo CommandBufferAllocateInfo{};
		CommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		CommandBufferAllocateInfo.commandPool = m_GraphicsCommandPool;
		CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		CommandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(m_GraphicsCommandBuffer.size());

		if (vkAllocateCommandBuffers(m_LogicalDevice, &CommandBufferAllocateInfo, m_GraphicsCommandBuffer.data()) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate command buffers!");
		std::cout << "Success to allocate a command buffers for graphics command pool !" << "\n";
	}

	void Application::createSyncObjects()
	{
		std::cout << "Try to create required synchronized objects ..." << "\n";
		m_ImageAvailableSemaphore.resize(m_MaxFrameInFlight);
		m_RenderFinishedSemaphore.resize(m_MaxFrameInFlight);
		m_InFlightFence.resize(m_MaxFrameInFlight);
		VkSemaphoreCreateInfo SemaphoreCreateInfo{};
		SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo FenceCreateInfo{};
		FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		FenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // fence��ʼ��Ϊ���źţ������һ֡�޷���ʼ

		std::cout << "Required synchronized objects:" << "\n";
		std::cout << "\tImage available semaphore" << "\n";
		std::cout << "\tRender finished semaphore" << "\n";
		std::cout << "\tIn flight fence" << "\n";
		for (size_t i = 0; i < m_MaxFrameInFlight; ++i) {
			if (vkCreateSemaphore(m_LogicalDevice, &SemaphoreCreateInfo, nullptr, &m_ImageAvailableSemaphore[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_LogicalDevice, &SemaphoreCreateInfo, nullptr, &m_RenderFinishedSemaphore[i]) != VK_SUCCESS ||
				vkCreateFence(m_LogicalDevice, &FenceCreateInfo, nullptr, &m_InFlightFence[i]) != VK_SUCCESS)
				throw std::runtime_error("Failed to create all synchronized objects!");
		}
		std::cout << "Success to create required synchronized objects !" << "\n";
	}

	void Application::drawFrame(float vDeltaTime)
	{
		std::cout << "Begin Frame ..." << "\n";
		std::cout << std::format("Frame time: {:.2f} ms", vDeltaTime) << "\n";
		std::cout << std::format("Current frame index: {}", m_CurrentFrame) << "\n";
		vkWaitForFences(m_LogicalDevice, 1, &m_InFlightFence[m_CurrentFrame], VK_TRUE, UINT64_MAX);

		uint32_t SwapchainImageIndex;
		if (VkResult Result = vkAcquireNextImageKHR(m_LogicalDevice, m_Swapchain, UINT64_MAX,
			m_ImageAvailableSemaphore[m_CurrentFrame], VK_NULL_HANDLE, &SwapchainImageIndex);
			Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR || m_IsWindowResize) {
			m_IsWindowResize = false;
			recreateSwapchain();
			return;
		}
		else if (Result != VK_SUCCESS) {
			throw std::runtime_error("Failed to acquire swap chain image!");
		}
		vkResetFences(m_LogicalDevice, 1, &m_InFlightFence[m_CurrentFrame]);

		// Record Command Buffer
		vkResetCommandBuffer(m_GraphicsCommandBuffer[m_CurrentFrame], 0);
		recordCommandBuffer(m_GraphicsCommandBuffer[m_CurrentFrame], SwapchainImageIndex);
		// Submit
		VkSubmitInfo SubmitInfo{};
		SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		VkSemaphore WaitSemaphores[] = { m_ImageAvailableSemaphore[m_CurrentFrame] };
		VkPipelineStageFlags WaitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		SubmitInfo.signalSemaphoreCount = 1;
		SubmitInfo.pSignalSemaphores = WaitSemaphores;
		SubmitInfo.pWaitDstStageMask = WaitStages;     // ����������һһ��Ӧ
		SubmitInfo.commandBufferCount = 1;
		SubmitInfo.pCommandBuffers = &m_GraphicsCommandBuffer[m_CurrentFrame];
		VkSemaphore SignalSemaphores[] = { m_RenderFinishedSemaphore[m_CurrentFrame] };
		SubmitInfo.signalSemaphoreCount = 1;
		SubmitInfo.pSignalSemaphores = SignalSemaphores;

		if (vkQueueSubmit(m_GraphicsQueue, 1, &SubmitInfo, m_InFlightFence[m_CurrentFrame]) != VK_SUCCESS)
			throw std::runtime_error("Failed to submit draw command buffer!");

		VkPresentInfoKHR PresentInfo{};
		PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		PresentInfo.waitSemaphoreCount = 1;
		PresentInfo.pWaitSemaphores = SignalSemaphores;
		VkSwapchainKHR Swapchains[] = { m_Swapchain };
		PresentInfo.swapchainCount = 1;
		PresentInfo.pSwapchains = Swapchains;
		PresentInfo.pImageIndices = &SwapchainImageIndex;
		PresentInfo.pResults = nullptr; // ����Swapchainû��Ҫ

		if (VkResult Result = vkQueuePresentKHR(m_PresentQueue, &PresentInfo);
			Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR || m_IsWindowResize) {
			m_IsWindowResize = false;
			recreateSwapchain();
		}
		else if (Result != VK_SUCCESS) {
			throw std::runtime_error("Failed to present swap chain image!");
		}

		m_CurrentFrame = (m_CurrentFrame + 1) % m_MaxFrameInFlight;
		std::cout << "End Frame" << "\n";
	}

}