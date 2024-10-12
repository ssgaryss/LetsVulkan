#pragma once
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <array>

namespace VulkanTutorial {

	struct Vertex
	{
		glm::vec2 m_Position = glm::vec2(0.0f, 0.0f);
		glm::vec3 m_Color = glm::vec3(0.0f, 0.0f, 0.0f);

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription VertexInputBindingDescription{};
			VertexInputBindingDescription.binding = 0;
			VertexInputBindingDescription.stride = sizeof(Vertex);
			VertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return VertexInputBindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 2> VertexInputAttributeDescriptions;
			VertexInputAttributeDescriptions[0].binding = 0;
			VertexInputAttributeDescriptions[0].location = 0;                         // location 0
			VertexInputAttributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;     // vec2
			VertexInputAttributeDescriptions[0].offset = offsetof(Vertex, m_Position);

			VertexInputAttributeDescriptions[1].binding = 0;
			VertexInputAttributeDescriptions[1].location = 1;                         // location 1
			VertexInputAttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;  // vec3
			VertexInputAttributeDescriptions[1].offset = offsetof(Vertex, m_Color);
			return VertexInputAttributeDescriptions;
		}
	};

}