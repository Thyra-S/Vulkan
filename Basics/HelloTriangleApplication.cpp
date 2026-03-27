#include "HelloTriangleApplication.h"

/*---------- INTIALIZATION / CLEANUP METHODS ----------*/

// Initialize GLFW window
void HelloTriangleApplication::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

// Initialize Vulkan instance
void HelloTriangleApplication::initVulkan()
{
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createGraphicsPipeline();
}

// Create Vulkan instance
void HelloTriangleApplication::createInstance()
{
	// Application info (optional, but may provide some optimization opportunities for driver)
	constexpr vk::ApplicationInfo appInfo
	{
		.pApplicationName	= "Hello Triangle",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName		= "No Engine",
		.engineVersion		= VK_MAKE_VERSION(1, 0, 0),
		.apiVersion			= vk::ApiVersion14
	};

	// Get the required layers
	std::vector<char const*> requiredLayers;
	if (enableValidationLayers)
	{
		requiredLayers.assign(validationLayers.begin(), validationLayers.end());
	}

	// Check if the required layers are supported by the Vulkan implementation.
	auto layerProperties = context.enumerateInstanceLayerProperties();
	auto unsupportedLayerIt = std::ranges::find_if(requiredLayers, [&layerProperties](auto const& requiredLayer)
		{
			return std::ranges::none_of(layerProperties, [requiredLayer](auto const& layerProperty)
				{
					return strcmp(layerProperty.layerName, requiredLayer) == 0;
				});
		});
	if (unsupportedLayerIt != requiredLayers.end())
	{
		throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));
	}

	// Get the required extensions.
	auto requiredExtensions = getRequiredInstanceExtensions();

	// Check if the required extensions are supported by the Vulkan implementation.
	auto extensionProperties = context.enumerateInstanceExtensionProperties();
	auto unsupportedPropertyIt = std::ranges::find_if(requiredExtensions, [extensionProperties](auto const& requiredExtension)
		{
			return std::ranges::none_of(extensionProperties, [requiredExtension](auto const& extensionProperty)
				{
					return strcmp(extensionProperty.extensionName, requiredExtension) == 0;
				});
		});

	if (unsupportedPropertyIt != requiredExtensions.end())
	{
		throw std::runtime_error("Required extension not supported: " + std::string(*unsupportedPropertyIt));
	}

	vk::InstanceCreateInfo createInfo
	{
		.pApplicationInfo		 = &appInfo,
		.enabledLayerCount		 = static_cast<uint32_t>(requiredLayers.size()),
		.ppEnabledLayerNames	 = requiredLayers.data(),
		.enabledExtensionCount	 = static_cast<uint32_t>(requiredExtensions.size()),
		.ppEnabledExtensionNames = requiredExtensions.data()
	};

	instance = vk::raii::Instance(context, createInfo);
}

// Gets required extensions for the Vulkan instance
std::vector<const char*> HelloTriangleApplication::getRequiredInstanceExtensions()
{
	uint32_t glfwExtensionCount = 0;
	auto     glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	if (enableValidationLayers)
	{
		extensions.push_back(vk::EXTDebugUtilsExtensionName);
	}

	return extensions;
}

// Create a Vulkan surface for rendering
void HelloTriangleApplication::createSurface() {
	VkSurfaceKHR _surface = nullptr;
	if (glfwCreateWindowSurface(*instance, window, nullptr, &_surface) != 0) {
		throw std::runtime_error("failed to create window surface!");
	}
	surface = vk::raii::SurfaceKHR(instance, _surface);
}

/*---------- PHYSICAL DEVICE METHODS ----------*/

// Pick a physical device (GPU) that supports Vulkan
void HelloTriangleApplication::pickPhysicalDevice()
{
	std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();
	auto const devIter = std::ranges::find_if(physicalDevices, [&](auto const& physicalDevice)
		{
			return isDeviceSuitable(physicalDevice);
		});

	if (devIter == physicalDevices.end())
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}

	physicalDevice = *devIter;
}

// Check if a physical device is suitable for our needs
bool HelloTriangleApplication::isDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice)
{
	// Check if the device supports Vulkan 1.3
	bool supportsVulkan1_3 = physicalDevice.getProperties().apiVersion >= vk::ApiVersion13;

	// Check if the device has a queue family that supports graphics operations
	auto queueFamilies = physicalDevice.getQueueFamilyProperties();
	bool supportsGraphics = std::ranges::any_of(queueFamilies, [](auto const& qfp) { return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics); });

	// Check if the device supports all required extensions
	auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
	bool supportsAllRequiredExtensions =
		std::ranges::all_of(requiredDeviceExtension, [&availableDeviceExtensions](auto const& requiredDeviceExtension)
			{
				return std::ranges::any_of(availableDeviceExtensions, [requiredDeviceExtension](auto const& availableDeviceExtension)
					{
						return strcmp(availableDeviceExtension.extensionName, requiredDeviceExtension) == 0;
					});
			});

	//	Check if the device supports the required features for our application
	auto features = physicalDevice .template getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
	bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
		features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

	// The device is suitable if it supports Vulkan 1.3, has a graphics queue family, supports all required extensions, and supports the required features.
	return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
}

// Create a logical device from the selected physical device, and retrieve the graphics queue.
void HelloTriangleApplication::createLogicalDevice()
{
	// find a queue family that supports both graphics and present operations
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

	uint32_t queueIndex = ~0;
	for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
	{
		if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
			physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface))
		{
			// found a queue family that supports both graphics and present
			queueIndex = qfpIndex;
			break;
		}
	}
	if (queueIndex == ~0)
	{
		throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
	}

	// query for Vulkan 1.3 features
	vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain =
	{
		{},                                   // vk::PhysicalDeviceFeatures2
		{.dynamicRendering	   = true},           // vk::PhysicalDeviceVulkan13Features
		{.extendedDynamicState = true}        // vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
	};

	// Create the logical device with the required features and extensions, and retrieve the graphics queue.
	float queuePriority = 0.5f;
	vk::DeviceQueueCreateInfo deviceQueueCreateInfo
	{
		.queueFamilyIndex = queueIndex,
		.queueCount		  = 1,
		.pQueuePriorities = &queuePriority,
	};
	vk::DeviceCreateInfo deviceCreateInfo
	{
		.pNext					 = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
		.queueCreateInfoCount	 = 1,
		.pQueueCreateInfos		 = &deviceQueueCreateInfo,
		.enabledExtensionCount	 = static_cast<uint32_t>(requiredDeviceExtension.size()),
		.ppEnabledExtensionNames = requiredDeviceExtension.data()
	};

	device = vk::raii::Device(physicalDevice, deviceCreateInfo);
	graphicsQueue = vk::raii::Queue(device, queueIndex, 0);
}

/*---------- SWAPCHAIN METHODS ----------*/

// Create the swapchain for rendering
void HelloTriangleApplication::createSwapChain()
{
	// Query the surface capabilities, formats, and presentation modes to determine the best settings for the swapchain.
	vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
	swapChainExtent = chooseSwapExtent(surfaceCapabilities);
	uint32_t minImageCount								  = chooseSwapMinImageCount(surfaceCapabilities);

	std::vector<vk::SurfaceFormatKHR> availableFormats	  = physicalDevice.getSurfaceFormatsKHR(*surface);
	swapChainSurfaceFormat								  = chooseSwapSurfaceFormat(availableFormats);

	std::vector<vk::PresentModeKHR> availablePresentModes = physicalDevice.getSurfacePresentModesKHR(*surface);
	vk::PresentModeKHR presentMode						  = chooseSwapPresentMode(availablePresentModes);

	// Set the chosen info for the swapchain creation.
	vk::SwapchainCreateInfoKHR swapChainCreateInfo
	{
		.surface		  = *surface,
		.minImageCount	  = minImageCount,
		.imageFormat	  = swapChainSurfaceFormat.format,
		.imageColorSpace  = swapChainSurfaceFormat.colorSpace,
		.imageExtent	  = swapChainExtent,
		.imageArrayLayers = 1,
		.imageUsage		  = vk::ImageUsageFlagBits::eColorAttachment,
		.imageSharingMode = vk::SharingMode::eExclusive,
		.preTransform	  = surfaceCapabilities.currentTransform,
		.compositeAlpha	  = vk::CompositeAlphaFlagBitsKHR::eOpaque,
		.presentMode	  = chooseSwapPresentMode(availablePresentModes),
		.clipped		  = true
	};

	// Creates the swapchain and retrieves the swapchain images.
	swapChain = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
	swapChainImages = swapChain.getImages();
}

// Choose the number of images in the swapchain based on the surface capabilities and our needs
uint32_t HelloTriangleApplication::chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surfaceCapabilities)
{
	auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
	if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount))
	{
		minImageCount = surfaceCapabilities.maxImageCount;
	}
	return minImageCount;
}

// Choose the best surface format for the swapchain from the available options
vk::SurfaceFormatKHR HelloTriangleApplication::chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& availableFormats)
{
	/*
	 * Look for the preferred format, B8G8R8A8_SRGB(8 bit SRGB),
	 * and color space, SRGB nonlinear color space, in the list of available formats.
	 */
	const auto formatIt = std::ranges::find_if(availableFormats, [](const auto& format)
		{
			return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
		});

	// If the preferred format is available, return it. Otherwise, return the first available format.
	return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
}

/*
 *  Choose the best surface format for the swapchain from the available options
 *
 *  vk::PresentModeKHR::eImmediate	 - Images submitted by your application are transferred to the screen right away, which may result in tearing.
 *
 *  vk::PresentModeKHR::eFifo		 - V-Sync, Swap Chain is a queue, when queue full, wait.
 *
 *	vk::PresentModeKHR::eFifoRelaxed - If queue was empty, send next image ASAP instead of waiting.
 *
 *  vk::PresentModeKHR::eMailbox	 - Triple Buffering V-Sync, when queue full, replace queued images with newer ones.
 */
vk::PresentModeKHR HelloTriangleApplication::chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& availablePresentModes)
{
	// Does the fall back/default mode (eFifo) exist, if not exit
	assert(std::ranges::any_of(availablePresentModes, [](auto presentMode)
		{
			return presentMode == vk::PresentModeKHR::eFifo;
		}));

	// If eMailbox is supported, use it, else fall back to using eFifo
	return std::ranges::any_of(availablePresentModes, [](const vk::PresentModeKHR value)
		{
			return vk::PresentModeKHR::eMailbox == value;
		}) ? vk::PresentModeKHR::eMailbox : vk::PresentModeKHR::eFifo;
}

// Choose the resolution of the swapchain
vk::Extent2D HelloTriangleApplication::chooseSwapExtent(vk::SurfaceCapabilitiesKHR const& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}

	int width = 0;
	int height = 0;
	glfwGetFramebufferSize(window, &width, &height);

	// Creates an Extend2D with the width and height of the framebuffer, but clamped 
	// to the min and max image extent supported by the surface capabilities.
	return 
	{
		std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
		std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
	};
}

/*---------- IMAGE VIEWS ----------*/

// Create image views for the swapchain images, which describe how to access the images and their properties.
void HelloTriangleApplication::createImageViews()
{
	assert(swapChainImageViews.empty());

	// The image view creation info is mostly the same for all swapchain images, except for the image itself, which is set inside the loop.
	vk::ImageViewCreateInfo imageViewCreateInfo
	{
		.viewType		  = vk::ImageViewType::e2D,
		.format			  = swapChainSurfaceFormat.format,
		.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
	};

	for (auto& image : swapChainImages)
	{
		imageViewCreateInfo.image = image;
		swapChainImageViews.emplace_back(device, imageViewCreateInfo);
	}
}

/*---------- GRAPHICS PIPELINE METHODS ----------*/

void HelloTriangleApplication::createGraphicsPipeline()
{

}

/*---------- RENDERING METHODS ----------*/

// Main rendering loop
void HelloTriangleApplication::mainLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}
}

/*---------- CLEANUP METHODS ----------*/

// Cleanup Vulkan resources
void HelloTriangleApplication::cleanup()
{
	glfwDestroyWindow(window);

	glfwTerminate();
}

/*---------- VALIDATION LAYERS / DEBUG ----------*/

// Setup the debug messenger for validation layers
void HelloTriangleApplication::setupDebugMessenger()
{
	if (!enableValidationLayers) // If validation layers are not enabled,  don't setup the debug messenger
		return;

	vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
		| vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

	vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
		vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
		| vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
		| vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

	vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT
	{
		.messageSeverity = severityFlags,
		.messageType = messageTypeFlags,
		.pfnUserCallback = &debugCallback
	};

	debugMessenger = instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
}

// Callback function for debug messages from validation layers
VKAPI_ATTR vk::Bool32 VKAPI_CALL HelloTriangleApplication::debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
	vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void*)
{
	if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
		|| severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
	{
		std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
	}

	return vk::False;
}

/*---------- MAIN METHOD / ENTRY POINT ----------*/

// Entry point for the application
void HelloTriangleApplication::run()
{
	initWindow();
	initVulkan();
	mainLoop();
	cleanup();
}

// Main function
int main()
{
	try
	{
		HelloTriangleApplication app;
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
