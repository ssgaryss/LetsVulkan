#pragma once
#include "Base.h"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>
#include <optional>


namespace VulkanTutorial {

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
		// others
		uint32_t ratePhysicalDevice(VkPhysicalDevice vPhysicalDevice);
		std::optional<uint32_t> findQueueFamilies(VkPhysicalDevice vPhysicalDevice, VkQueueFlagBits vFlag);
		std::optional<uint32_t> findPresentQueueFamilies(VkPhysicalDevice vPhysicalDevice);  // 查询支持present的QueueFamily
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

		VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
		VkQueue m_PresentQueue = VK_NULL_HANDLE;
	};

}
