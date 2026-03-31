#ifndef HelloTriangleApplication_h
#define HelloTriangleApplication_h

#define NOMINMAX
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <vector>
#include <limits>

#define VULKAN_HPP_NO_CONSTRUCTORS
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES) 
	#include <vulkan/vulkan_raii.hpp>
#else 
	import vulkan_hpp;
#endif

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

class HelloTriangleApplication {
public:
	// entry point for the application
    void run();

private:
	/*---------- GLOBAL VARIABLES ----------*/
	GLFWwindow* window = nullptr;

	const uint32_t WIDTH = 2400;
	const uint32_t HEIGHT = 1800;

	vk::raii::Context				 context;
	vk::raii::Instance				 instance = nullptr; 
	vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
	vk::raii::PhysicalDevice		 physicalDevice = nullptr;
	vk::raii::Device				 device = nullptr;
	uint32_t queueIndex = ~0;
	vk::raii::Queue					 graphicsQueue = nullptr;
	vk::raii::SurfaceKHR			 surface = nullptr;
	vk::raii::SwapchainKHR           swapChain = nullptr;
	std::vector<vk::Image>           swapChainImages;
	vk::SurfaceFormatKHR             swapChainSurfaceFormat;
	vk::Extent2D                     swapChainExtent;
	std::vector<vk::raii::ImageView> swapChainImageViews;

	vk::raii::PipelineLayout pipelineLayout = nullptr;
	vk::raii::Pipeline       graphicsPipeline = nullptr;

	vk::raii::CommandPool commandPool = nullptr;
	vk::raii::CommandBuffer commandBuffer = nullptr;

	vk::raii::Semaphore presentCompleteSemaphore = nullptr;
	vk::raii::Semaphore renderFinishedSemaphore = nullptr;
	vk::raii::Fence drawFence = nullptr;

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

	void createSyncObjects();

	// create Vulkan instance
	void createInstance();

	// Gets required extensions for the vulkan instance
	std::vector<const char*> getRequiredInstanceExtensions();

	// initialize GLFW window
	void initWindow();

	// Create a Vulkan surface for rendering
	void createSurface();

	void createSwapChain();

	/*---------- PHYSICAL DEVICE METHODS ----------*/

	// Pick a physical device (GPU) that supports Vulkan
	void pickPhysicalDevice();

	// Check if a physical device is suitable for our needs
	bool isDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice);

	// Creates the logical device from the selected physical device, and retrieves the graphics queue
	void createLogicalDevice();

	/*---------- SWAPCHAIN METHODS ----------*/

	// Choose the best surface format for the swapchain from the available options
	vk::SurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& availableFormats);

	// Choose the best presentation mode for the swapchain from the available options
	vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& availablePresentModes);

	// Choose the resolution of the swap chains.
	vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const& capabilities);

	// Choose the number of images in the swapchain based on the surface capabilities and our needs
	uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surfaceCapabilities);

	/*---------- IMAGE VIEWS ----------*/
	
	// Create image views for the swapchain images, which describe how to access the images and their properties.
	void createImageViews();

	/*---------- GRAPHICS PIPELINE METHODS ----------*/
	// Create the graphics pipeline for rendering, which includes shader stages, fixed-function stages, and pipeline layout.
	void createGraphicsPipeline();

	// Helper function to read a file into a byte array, used for loading shader code.
	static std::vector<char> readFile(const std::string& filename);

	// Helper function to create a shader module from the given shader code.
	[[nodiscard]] vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const;

	/*---------- RENDERING METHODS ----------*/

	// Create a command pool to manage the memory that is used to store the buffers and command buffers are allocated from them.
	void createCommandPool();

	// Create command buffers, which are used to record drawing commands that will be submitted to the graphics queue for execution.
	void createCommandBuffer();

	// Record commands into the command buffer, which will be executed by the graphics queue
	void recordCommandBuffer(uint32_t imageIndex);

	void transition_image_layout(
		uint32_t                imageIndex,
		vk::ImageLayout         old_layout,
		vk::ImageLayout         new_layout,
		vk::AccessFlags2        src_access_mask,
		vk::AccessFlags2        dst_access_mask,
		vk::PipelineStageFlags2 src_stage_mask,
		vk::PipelineStageFlags2 dst_stage_mask);

	// main rendering loop
	void mainLoop();

	// Draw a frame by acquiring an image from the swapchain, executing the command buffer with that image as attachment in the framebuffer, and returning the image to the swapchain for presentation.
	void drawFrame();

	/*---------- CLEANUP METHODS ----------*/

	// cleanup Vulkan resources
	void cleanup();

	/*---------- VALIDATION LAYERS / DEBUG ----------*/

	// Setup the debug messenger for validation layers
	void setupDebugMessenger();

	// Callback function for debug messages from validation layers
	static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, 
	vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void*);

	
};

#endif HelloTriangleApplication_h