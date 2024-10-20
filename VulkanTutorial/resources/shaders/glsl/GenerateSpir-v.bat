@echo off
echo Try to compile shaders to spir-v ...
call C:/VulkanSDK/1.3.290.0/Bin/glslc.exe 09_shader_base.vert -o ../spir-v/09_shader_base_vert.spv
call C:/VulkanSDK/1.3.290.0/Bin/glslc.exe 09_shader_base.frag -o ../spir-v/09_shader_base_frag.spv
call C:/VulkanSDK/1.3.290.0/Bin/glslc.exe 18_shader_vertexbuffer.vert -o ../spir-v/18_shader_vertexbuffer_vert.spv
call C:/VulkanSDK/1.3.290.0/Bin/glslc.exe 18_shader_vertexbuffer.frag -o ../spir-v/18_shader_vertexbuffer_frag.spv
call C:/VulkanSDK/1.3.290.0/Bin/glslc.exe 22_shader_ubo.vert -o ../spir-v/22_shader_ubo_vert.spv
call C:/VulkanSDK/1.3.290.0/Bin/glslc.exe 22_shader_ubo.frag -o ../spir-v/22_shader_ubo_frag.spv
echo Success to convert shaders to spir-v at path ../spir-v/
pause