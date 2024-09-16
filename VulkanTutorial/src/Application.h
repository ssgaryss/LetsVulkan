#pragma once
#include "Base.h"

#include <GLFW/glfw3.h>
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
		void initWindow();
		void initVulkan();
		void mainLoop();
		void cleanup();

		void createInstance();
		void setupDebugMessenger();
		void pickPhysicalDevice();
		void createLogicalDevice();
	private:
		void showExtentionInformation();
		std::vector<const char*> getRequiredExtentions();
		uint32_t ratePhysicalDevice(VkPhysicalDevice vPhysicalDevice);
		std::optional<uint32_t> findGraphicQueueFamilies(VkPhysicalDevice vPhysicalDevice);
	private:
		const uint32_t m_Width = 800;
		const uint32_t m_Height = 600;
		GLFWwindow* m_Window = nullptr;
	private:
		VkInstance m_Instance;
		VkDebugUtilsMessengerEXT m_DebugMessenger;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_LogicalDevice = VK_NULL_HANDLE;
		VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
	};

}
