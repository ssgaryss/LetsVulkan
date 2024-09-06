#pragma once
#include <cstdint>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

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
		void createInstance();
	private:
		const uint32_t m_Width = 800;
		const uint32_t m_Height = 600;
		GLFWwindow* m_Window = nullptr;
	private:
		VkInstance m_Instance;
	};

}
