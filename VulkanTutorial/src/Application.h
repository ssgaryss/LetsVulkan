#pragma once
#include "Base.h"
#include "Timer.h"
#include "Primitive.h"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>
#include <optional>
#include <filesystem>


namespace VulkanTutorial {

	const std::vector<Vertex> Vertices = {
		{{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
		{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
		{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
	};

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
		void createRenderPass();
		void createGraphicsPipeline();
		void createFramebuffers();
		void createGraphicsCommandPool();
		void createVertexBuffer();
		void createGraphicsCommandBuffers();
		void createSyncObjects();
		// mainLoop
		void drawFrame(float vDeltaTime);
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
		void recreateSwapchain();
		void cleanupSwapchain();
	private:
		// Shader
		std::vector<char> readFile(const std::filesystem::path& vPath);
		VkShaderModule createShaderModule(const std::vector<char>& vCode);
	private:
		// Command
		void recordCommandBuffer(VkCommandBuffer vCommandBuffer, uint32_t vImageIndex);
	private:
		// Buffer
		uint32_t findMemoryType(uint32_t vTypeFilter, VkMemoryPropertyFlags vProperties);
		void createBuffer(VkDeviceSize vSize, VkBufferUsageFlags vUsage, VkMemoryPropertyFlags vFlags,
			VkBuffer& vBuffer, VkDeviceMemory& vBufferMemory); // 创建Buffer并分配Memory
		void copyBuffer(VkBuffer vDestination, VkBuffer vSource, VkDeviceSize vSize);
	public:
		uint32_t m_Width = 800;
		uint32_t m_Height = 600;
		const uint32_t m_MaxFrameInFlight = 2; // 即CPU最多领先GPU一帧画面，通常2是合理的
		uint32_t m_CurrentFrame = 0;
		bool m_IsWindowResize = false;
		bool m_IsMinimized = false;
		GLFWwindow* m_Window = nullptr;
		Timer m_Timer = {};
		float m_LastFrameTime = 0.0f;
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
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_Pipeline = VK_NULL_HANDLE;
		std::vector<VkFramebuffer> m_SwapchainFramebuffers;
		VkCommandPool m_GraphicsCommandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> m_GraphicsCommandBuffer;
		std::vector<VkSemaphore> m_ImageAvailableSemaphore;
		std::vector<VkSemaphore> m_RenderFinishedSemaphore;
		std::vector<VkFence> m_InFlightFence;

		VkBuffer m_VertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_VertexBufferMemory = VK_NULL_HANDLE;

		VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
		VkQueue m_PresentQueue = VK_NULL_HANDLE;
	};

}
