#include <GLFW/glfw3.h>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <format>
#include <exception>
#include <cstdlib>

#include "Application.h"

int main() {
    VulkanTutorial::Application App;
    try {
        App.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}