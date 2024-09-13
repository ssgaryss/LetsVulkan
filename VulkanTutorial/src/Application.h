#pragma once
#include "Base.h"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

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
	private:
		void showExtentionInformation();
		std::vector<const char*> getRequiredExtentions();
		uint32_t ratePhysicalDevice(VkPhysicalDevice vPhysicalDevice);
	private:
		const uint32_t m_Width = 800;
		const uint32_t m_Height = 600;
		GLFWwindow* m_Window = nullptr;
	private:
		VkInstance m_Instance;
		VkDebugUtilsMessengerEXT m_DebugMessenger;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	};

}
