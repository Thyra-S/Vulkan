#ifndef HelloTriangleApplication_h
#define HelloTriangleApplication_h

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <optional>

using namespace std;

class HelloTriangleApplication {
public:
	// entry point for the application
    void run();

private:
	/*---------- GLOBAL VARIABLES ----------*/
	VkInstance instance;

	GLFWwindow* window;
	const uint32_t WIDTH = 2400;
	const uint32_t HEIGHT = 1800;

	VkDebugUtilsMessengerEXT debugMessenger;

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		bool isComplete() {
			return graphicsFamily.has_value();
		}
	};

	/*---------- INITIALIZATION / CLEANUP METHODS ----------*/

	// initialize Vulkan instance
    void initVulkan();

	// pick a physical device (GPU) that supports Vulkan
	void pickPhysicalDevice();

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	// check if a physical device is suitable for our needs
	bool isDeviceSuitable(VkPhysicalDevice device);

	// Rate device suitability based on its features and properties, higher score is better
	//int rateDeviceSuitability(VkPhysicalDevice device);

	// create Vulkan instance
	void createInstance();

	// initialize GLFW window
	void initWindow();

	// cleanup Vulkan resources
	void cleanup();

	/*---------- VALIDATION LAYERS / DEBUG ----------*/

	const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
	};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

	// Checks what validation layers are supported by device
	bool checkValidationLayerSupport();

	// Return a list of the required extensions.
	std::vector<const char*> getRequiredExtensions();

	// Setup the debug messenger for validation layers
	void setupDebugMessenger();

	// Create the debug messenger (extension function), Destroying the debug messenger is handled in cleanup() method using external method.
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

	// Helper method to populate the VkDebugUtilsMessengerCreateInfoEXT structure with the desired settings for the debug messenger
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	// Helper method to load the function pointer for destroying the debug messenger, since it's an extension function and not automatically loaded by Vulkan
	static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	// Callback function for debug messages from validation layers
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	/*---------- RENDERING METHODS ----------*/

	// main rendering loop
    void mainLoop();	
};

#endif /* HelloTriangleApplication_h */