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
}

// Create Vulkan instance
void HelloTriangleApplication::createInstance()
{
	// Application info (optional, but may provide some optimization opportunities for driver)
	constexpr vk::ApplicationInfo appInfo
	{ 
		.pApplicationName = "Hello Triangle",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = vk::ApiVersion14 
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
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
		.ppEnabledLayerNames = requiredLayers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
		.ppEnabledExtensionNames = requiredExtensions.data() 
	};

	instance = vk::raii::Instance(context, createInfo);
}

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
	bool queueFamilies = physicalDevice.getQueueFamilyProperties();
	bool supportsGraphics = std::ranges::any_of(queueFamilies, [](auto const& qfp) { return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics); });

	// Check if the device supports all required extensions
	auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
	bool supportsAllRequiredExtensions =
		std::ranges::all_of(requiredDeviceExtension,[&availableDeviceExtensions](auto const& requiredDeviceExtension)
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

/*---------- CLEANUP METHODS ----------*/

// Cleanup Vulkan resources
void HelloTriangleApplication::cleanup()
{
	glfwDestroyWindow(window);

	glfwTerminate();
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
