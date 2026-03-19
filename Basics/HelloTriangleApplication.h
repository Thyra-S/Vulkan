#ifndef HelloTriangleApplication_h
#define HelloTriangleApplication_h

#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

class HelloTriangleApplication {
public:
	// entry point for the application
    void run();

private:
	// initialize Vulkan instance
    void initVulkan();

	// main rendering loop
    void mainLoop();

	// cleanup Vulkan resources
    void cleanup();
};
#endif /* HelloTriangleApplication_h */