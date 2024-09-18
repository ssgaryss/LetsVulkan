#pragma once
#include "Base.h"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>
#include <optional>
#include <filesystem>


namespace VulkanTutorial {

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR m_SurfaceCapabilities;
		std::vector<VkSurfaceFormatKHR> m_SurfaceFormats;
		std::vector<VkPresentModeKHR> m_PresentModes;
	};

	class Application
	{
	public:
		void run();

	private:
		// critical
		void initWindow();
		void initVulkan();
		void mainLoop();
		void cleanup();
	private:
		// initVulkan
		void createInstance();
		void setupDebugMessenger();
		void createSurface();
		void pickPhysicalDevice();
		void createLogicalDevice();
		void createSwapChain();
		void createImageViews();
		void createGraphicsPipeline();
	private:
		// Extensions
		void showExtensionInformation(const std::vector<VkExtensionProperties>& vExtensions);
		void showExtensionInformation(const std::vector<const char*>& vExtensions);

		std::vector<VkExtensionProperties> getSupportedInstanceExtensions();
		std::vector<const char*> getRequiredIntanceExtensions();
		bool checkRequiredInstanceExtensionsSupport(const std::vector<const char*>& vRequiredExtensions);

		std::vector<VkExtensionProperties> getSupportedDeviceExtensions(VkPhysicalDevice vPhysicalDevice);
		std::vector<const char*> getRequiredDeviceExtensions();
		bool checkRequiredDeviceExtensionsSupport(VkPhysicalDevice vPhysicalDevice, const std::vector<const char*>& vRequiredExtensions);
	private:
		// Divece and Queue families
		uint32_t ratePhysicalDevice(VkPhysicalDevice vPhysicalDevice);
		std::optional<uint32_t> findQueueFamilies(VkPhysicalDevice vPhysicalDevice, VkQueueFlagBits vFlag);
		std::optional<uint32_t> findPresentQueueFamilies(VkPhysicalDevice vPhysicalDevice);  // 查询支持present的QueueFamily
		bool checkRequiredQueueFamiliesSupport();  // All Queue required available ?
	private:
		// SwapChain
		SwapChainSupportDetails getSupportedSwapchainDetails(VkPhysicalDevice vPhysicalDevice, VkSurfaceKHR vSurface);
		VkSurfaceFormatKHR chooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& vSurfaceFormats);
		VkPresentModeKHR chooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& vPresentModes);
		VkExtent2D chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& vCapabilities);
		bool checkSwapchainSupport(const SwapChainSupportDetails& vSwapchainDetails);
	private:
		// Shader
		std::vector<char> readFile(const std::filesystem::path& vPath);
		VkShaderModule createShaderModule(const std::vector<char>& vCode);
	private:
		const uint32_t m_Width = 800;
		const uint32_t m_Height = 600;
		GLFWwindow* m_Window = nullptr;
	private:
		VkInstance m_Instance;
		VkDebugUtilsMessengerEXT m_DebugMessenger;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_LogicalDevice = VK_NULL_HANDLE;
		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
		std::vector<VkImage> m_SwapchainImages;
		std::vector<VkImageView> m_SwapchainImageViews;
		VkFormat m_SwapchainFormat = VK_FORMAT_UNDEFINED;
		VkExtent2D m_SwapchainExtent;
		VkPipeline m_Pipeline = VK_NULL_HANDLE;

		VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
		VkQueue m_PresentQueue = VK_NULL_HANDLE;
	};

}
