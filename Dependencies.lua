-- VulkanTutorial Dependencies

VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["glfw"] = "%{wks.location}/VulkanTutorial/vendor/glfw/include"
IncludeDir["glm"] = "%{wks.location}/VulkanTutorial/vendor/glm"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/include"


Library = {}
Library["Vulkan"] = "%{VULKAN_SDK}/Lib/vulkan-1.lib"