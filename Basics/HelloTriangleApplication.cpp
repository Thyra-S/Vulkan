#include "HelloTriangleApplication.h"

// Entry point for the application
void HelloTriangleApplication::run() {
    initVulkan();
    mainLoop();
    cleanup();
}

// Initialize Vulkan instance
void HelloTriangleApplication::initVulkan() {

}

// Main rendering loop
void HelloTriangleApplication::mainLoop() {
    while (true) {
        // Poll for window events here
    }
}

// Cleanup Vulkan resources
void HelloTriangleApplication::cleanup() {

}

// Main function
int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
