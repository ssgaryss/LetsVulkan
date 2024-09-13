#pragma once
#include "Base.h"

#include <cstdint>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
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

		void showExtentionInformation();
		std::vector<const char*> getRequiredExtentions();
		void createInstance();
		void setupDebugMessenger();
	private:
		const uint32_t m_Width = 800;
		const uint32_t m_Height = 600;
		GLFWwindow* m_Window = nullptr;
	private:
		VkInstance m_Instance;
		VkDebugUtilsMessengerEXT m_DebugMessenger;
	};

}
