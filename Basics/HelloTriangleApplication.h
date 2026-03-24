#ifndef HelloTriangleApplication_h
#define HelloTriangleApplication_h

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#define VULKAN_HPP_NO_CONSTRUCTORS
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES) 
	#include <vulkan/vulkan_raii.hpp>
#else 
	import vulkan_hpp;
#endif

#define GLFW_INCLUDE_VULKAN        // REQUIRED only for GLFW CreateWindowSurface.
#include <GLFW/glfw3.h>

class HelloTriangleApplication {
public:
	// entry point for the application
    void run();

private:
	/*---------- GLOBAL VARIABLES ----------*/
	GLFWwindow* window = nullptr;

	vk::raii::Context context;
	vk::raii::Instance instance = nullptr; 
	vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;

	const uint32_t WIDTH = 2400;
	const uint32_t HEIGHT = 1800;

	std::vector<const char*> requiredDeviceExtension = { vk::KHRSwapchainExtensionName };

	const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

	#ifdef NDEBUG
		const bool enableValidationLayers = false;
	#else
		const bool enableValidationLayers = true;
	#endif

	/*---------- INITIALIZATION METHODS ----------*/

	// initialize Vulkan instance
    void initVulkan();

	// Rate device suitability based on its features and properties, higher score is better
	//int rateDeviceSuitability(VkPhysicalDevice device);

	// create Vulkan instance
	void createInstance();

	// Gets required extensions for the vulkan instance
	std::vector<const char*> getRequiredInstanceExtensions();

	// initialize GLFW window
	void initWindow();

	/*---------- PHYSICAL DEVICE METHODS ----------*/

	// Pick a physical device (GPU) that supports Vulkan
	void pickPhysicalDevice();

	// Check if a physical device is suitable for our needs
	bool isDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice);

	/*---------- CLEANUP METHODS ----------*/

	// cleanup Vulkan resources
	void cleanup();

	/*---------- VALIDATION LAYERS / DEBUG ----------*/

	// Setup the debug messenger for validation layers
	void setupDebugMessenger();

	// Callback function for debug messages from validation layers
	static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, 
	vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void*);

	/*---------- RENDERING METHODS ----------*/

	// main rendering loop
    void mainLoop();	
};

#endif HelloTriangleApplication_h