#include "TriangleApplication.h"

// +---------------------------+
// | STANDARD HEADERS          |
// +---------------------------+
#include <cstdlib>
#include <functional>
#include <iostream>
#include <set>
#include <stdexcept>

// +---------------------------+
// | PROJECT HEADERS           |
// +---------------------------+
#include "VulkanDeviceQueries.h"


using namespace engine;

constexpr int GLFW_WINDOW_OPEN{ 0 };

std::string get_result_string( const VkResult& vulkan_result )
{
    switch ( vulkan_result )
    {
        case VK_SUCCESS:
            return "SUCCESS";
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            return "OUT OF HOST MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            return "OUT OF DEVICE MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED:
            return "INITIALIZATION FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT:
            return "LAYER NOT PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            return "EXTENSION NOT PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            return "INCOMPATIBLE DRIVER";
        default:
            return "UNKNOWN RESULT";
    }
}

template<typename vk_property_t>
bool compare_support_containers( const std::vector<const char*>& required, const std::vector<vk_property_t>& available,
                                 const std::function<const char*( const vk_property_t& )>& getName )
{
    // Refactor to use the erase method. return boolean but pass vector by reference to keep the not found items.
    for ( const auto& required_property : required )
    {
        bool found{ false };
        for ( const auto& available_property : available )
        {
            if ( strcmp( required_property, getName( available_property ) ) == 0 )
            {
                found = true;
                break;
            }
        }

        if ( !found )
        {
            return false;
        }
    }
    return true;
}

// +---------------------------+
// | PUBLIC                    |
// +---------------------------+
void TriangleApplication::run( )
{
    init_window( );
    init_vk( );

    bool keepRunning{ true };
    while ( keepRunning )
    {
        keepRunning = glfwWindowShouldClose( window_ptr_ ) == GLFW_WINDOW_OPEN;
        main_loop( );
    }
    cleanup( );
}

// +---------------------------+
// | PRIVATE                   |
// +---------------------------+
void TriangleApplication::vk_create_instance( )
{
    // This data is technically optional, but it may provide some useful information to the driver
    // in order to optimize our specific application.
    VkApplicationInfo appInfo{};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
    appInfo.pEngineName        = "No Engine";
    appInfo.engineVersion      = VK_MAKE_VERSION( 1, 0, 0 );
    appInfo.apiVersion         = VK_API_VERSION_1_0;

    // Tells the Vulkan driver which global extensions and validation layers we want to use.
    // Global here means that they apply to the entire program and not a specific device.
    VkInstanceCreateInfo createInfo{};
    createInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // GLFW has a handy built-in function that returns the extension(s) it needs to do that
    // which we can pass to the struct.
    uint32_t glfwExtensionCount{ 0 };
    const char** glfwExtensions{ glfwGetRequiredInstanceExtensions( &glfwExtensionCount ) };

    std::vector<const char*> requiredExtensions{ glfwExtensions, glfwExtensions + glfwExtensionCount };
#ifdef __APPLE__
    // VK_KHR_PORTABILITY_subset extension is necessary on macOS to allow MoltenVK to function properly.
    requiredExtensions.push_back( VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME );
    requiredExtensions.push_back( "VK_KHR_get_physical_device_properties2" );
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    if constexpr ( ENABLE_VALIDATION_LAYERS_ )
    {
        requiredExtensions.push_back( "VK_EXT_debug_utils" );
    }

    if ( !vk_check_extension_support( requiredExtensions ) )
    {
        throw std::runtime_error( "One or more extensions are not supported!" );
    }

    createInfo.enabledExtensionCount   = static_cast<uint32_t>( requiredExtensions.size( ) );
    createInfo.ppEnabledExtensionNames = requiredExtensions.data( );

    // The last two members of the struct determine the global validation layers to enable.
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if constexpr ( ENABLE_VALIDATION_LAYERS_ )
    {
        if ( !vk_check_validation_layer_support( std::vector<const char*>{
            VALIDATION_LAYERS_.begin( ), VALIDATION_LAYERS_.end( )
        } ) )
        {
            throw std::runtime_error( "Validation layers requested, but not available!" );
        }

        createInfo.enabledLayerCount   = static_cast<uint32_t>( VALIDATION_LAYERS_.size( ) );
        createInfo.ppEnabledLayerNames = VALIDATION_LAYERS_.data( );

        // We create a debug messenger specifically for the creation of the instance.
        // This will also provide validation in the destroy instance as well.
        vk_populate_debug_messenger_create_info( debugCreateInfo );
        createInfo.pNext = &debugCreateInfo;
    }
    else
    {
        // If we're not in debug mode, we can just set the count to 0.
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    // We've now specified everything Vulkan needs to create an instance and we can
    // finally issue the vkCreateInstance.
    if ( const VkResult result{ vkCreateInstance( &createInfo, nullptr, &instance_ ) }; result != VK_SUCCESS )
    {
        throw std::runtime_error( "Failed to create Vulkan instance! Error: " + get_result_string( result ) );
    }
}

void TriangleApplication::vk_setup_debug_messenger( )
{
    // Possible different configuration:
    // https://docs.vulkan.org/spec/latest/chapters/debugging.html#VK_EXT_debug_utils
    if constexpr ( ENABLE_VALIDATION_LAYERS_ )
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        vk_populate_debug_messenger_create_info( createInfo );

        if ( vk_create_debug_utils_messenger_EXT( instance_, &createInfo, nullptr, &debug_messenger_ ) != VK_SUCCESS )
        {
            throw std::runtime_error( "Failed to set up debug messenger!" );
        }
    }
}

void TriangleApplication::vk_populate_debug_messenger_create_info( VkDebugUtilsMessengerCreateInfoEXT& info )
{
    // There are a lot more settings for the behavior of validation layers than just the flags specified in the
    // VkDebugUtilsMessengerCreateInfoEXT struct.
    // Browse to the Vulkan SDK and go to the Config directory. There you will find a vk_layer_settings.txt
    // file that explains how to configure the layers.
    // Fill in a structure with details about the messenger and its callback.
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

    // The messageSeverity field allows you to specify all the types of severities you would like
    // your callback to be called for.
    info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    // The messageType field lets you filter which types of messages your callback is notified about.
    // Currently, all types are enabled.
    info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    info.pfnUserCallback = debug_callback;
    info.pUserData       = nullptr;
}

bool TriangleApplication::vk_check_extension_support( const std::vector<const char*>& extensions )
{
    // Retrieve a list of supported extensions
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );
    std::vector<VkExtensionProperties> availableExtensions( extensionCount );
    vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, availableExtensions.data( ) );

    // Check if all the required extensions are supported
    return compare_support_containers<VkExtensionProperties>( extensions, availableExtensions,
                                                              []( const VkExtensionProperties& extension ) {
                                                                  return extension.extensionName;
                                                              } );
}

bool TriangleApplication::vk_check_validation_layer_support( const std::vector<const char*>& layers )
{
    // Retrieve a list of supported layers
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties( &layerCount, nullptr );
    std::vector<VkLayerProperties> availableLayers( layerCount );
    vkEnumerateInstanceLayerProperties( &layerCount, availableLayers.data( ) );

    return compare_support_containers<VkLayerProperties>( layers, availableLayers,
                                                          []( const VkLayerProperties& layer ) {
                                                              return layer.layerName;
                                                          } );
}

void TriangleApplication::vk_pick_physical_device( )
{
    // The physical device gets implicitly destroyed when we destroy the instance.
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices( instance_, &deviceCount, nullptr );

    if ( deviceCount == 0 )
    {
        throw std::runtime_error( "Failed to find GPUs with Vulkan support!" );
    }

    // Fetch the physical devices
    std::vector<VkPhysicalDevice> devices( deviceCount );
    vkEnumeratePhysicalDevices( instance_, &deviceCount, devices.data( ) );

    // Check if any of the physical devices meet the requirements
    for ( const auto& device : devices )
    {
        if ( vk_is_device_suitable( device ) )
        {
            physical_device_ = device;
            break;
        }
    }

    if ( physical_device_ == VK_NULL_HANDLE )
    {
        throw std::runtime_error( "Failed to find a suitable GPU!" );
    }
}

bool TriangleApplication::vk_is_device_suitable( const VkPhysicalDevice device ) const
{
    const query::QueueFamilyIndices indices{ query::find_queue_families( device, surface_ ) };

    const bool extensionsSupported{ vk_check_device_extension_support( device ) };

    // Swap chain support is sufficient for now if there is at least one supported image format and one supported
    // presentation mode.
    bool swapChainAdequate{ false };
    if ( extensionsSupported )
    {
        const query::SwapChainSupportDetails swapChainSupport{ query::query_swap_chain_support( device, surface_ ) };
        swapChainAdequate = !swapChainSupport.formats.empty( ) && !swapChainSupport.presentModes.empty( );
    }

    return indices.is_suitable( ) && extensionsSupported && swapChainAdequate;
}

bool TriangleApplication::vk_check_device_extension_support( VkPhysicalDevice device )
{
    // Check if the device supports the required extensions
    uint32_t extensionCount{};
    vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount, nullptr );

    std::vector<VkExtensionProperties> availableExtensions( extensionCount );
    vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount, availableExtensions.data( ) );

    std::set<std::string> requiredExtensions( DEVICE_EXTENSIONS_.begin( ), DEVICE_EXTENSIONS_.end( ) );

    for ( const auto& extension : availableExtensions )
    {
        requiredExtensions.erase( extension.extensionName );
    }

    return requiredExtensions.empty( );
}

void TriangleApplication::vk_create_logical_device( )
{
    // This structure describes the number of queues we want for a single queue family. Right now we're only
    // interested in a queue with graphics capabilities.
    const query::QueueFamilyIndices indices = query::find_queue_families( physical_device_, surface_ );

    // Get the unique queue families to load once.
    std::set uniqueQueueFamilies{ indices.graphicsFamily.value( ), indices.presentFamily.value( ) };

    // Vulkan lets you assign priorities to queues to influence the scheduling of command buffer execution using
    // floating point numbers between 0.0 and 1.0.
    const float queuePriority{ 1.0f };
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
    for ( uint32_t queueFamily : uniqueQueueFamilies )
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount       = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back( queueCreateInfo );
    }

    // The next information to specify is the set of device features that we'll be using.
    // These are the features that we queried support for with vkGetPhysicalDeviceFeatures.
    VkPhysicalDeviceFeatures deviceFeatures{};

    // Create the logical device
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>( queueCreateInfos.size( ) );
    createInfo.pQueueCreateInfos    = queueCreateInfos.data( );

    createInfo.pEnabledFeatures = &deviceFeatures;

    // The remainder of the information bears a resemblance to the VkInstanceCreateInfo struct and requires you
    // to specify extensions and validation layers. The difference is that these are device specific this time.
    std::vector<const char*> extensions{ DEVICE_EXTENSIONS_.begin( ), DEVICE_EXTENSIONS_.end( ) };

#ifdef __APPLE__
    // Device specific extensions are necessary on macOS to allow MoltenVK to function properly.
    extensions.push_back( "VK_KHR_portability_subset" );
#endif

    createInfo.enabledExtensionCount   = extensions.size( );
    createInfo.ppEnabledExtensionNames = extensions.data( );

    // Previous implementations of Vulkan made a distinction between instance and device specific validation layers,
    // but this is no longer the case. We're still doing it for backwards compatibility.
    if constexpr ( ENABLE_VALIDATION_LAYERS_ )
    {
        createInfo.enabledLayerCount   = static_cast<uint32_t>( VALIDATION_LAYERS_.size( ) );
        createInfo.ppEnabledLayerNames = VALIDATION_LAYERS_.data( );
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    // Create instance and validate the result
    if ( const auto result = vkCreateDevice( physical_device_, &createInfo, nullptr, &device_ ); result != VK_SUCCESS )
    {
        throw std::runtime_error( "Failed to create logical device! Error: " + get_result_string( result ) );
    }

    // Retrieve the handles for the queues
    vkGetDeviceQueue( device_, indices.graphicsFamily.value( ), 0, &graphics_queue_ );
    vkGetDeviceQueue( device_, indices.presentFamily.value( ), 0, &present_queue_ );
}

void TriangleApplication::vk_create_surface( )
{
    // To manage the output window, we need to include a parameter for the instance, surface creation details,
    // custom allocators and the variable for the surface handle to be stored in.
    // This is technically OS specific, but GLFW provides a cross-platform way to do this.
    // The glfwCreateWindowSurface function performs this operation with a different implementation for each platform.
    if ( const auto result = glfwCreateWindowSurface( instance_, window_ptr_, nullptr, &surface_ );
        result != VK_SUCCESS )
    {
        throw std::runtime_error( "Failed to create window surface! Error: " + get_result_string( result ) );
    }
}

void TriangleApplication::vk_create_swap_chain( )
{
    query::SwapChainSupportDetails swapChainSupport = query::query_swap_chain_support( physical_device_, surface_ );

    VkSurfaceFormatKHR surfaceFormat = vk_choose_swap_surface_format( swapChainSupport.formats );
    VkPresentModeKHR presentMode     = vk_choose_swap_present_mode( swapChainSupport.presentModes );
    VkExtent2D extent                = vk_choose_swap_extent( swapChainSupport.capabilities );

    // Simply sticking to the minimum image count means that we may sometimes have to wait on the driver to complete
    // internal operations before we can acquire another image to render to. Therefore it's recommended to set the
    // image count to 1 more than the minimum.
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    // We must also make sure to not exceed the maximum number of images while doing this, where 0 is a special value
    // that means that there is no maximum specified by the standard.
    if ( swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount )
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface_;

    createInfo.minImageCount    = imageCount;
    createInfo.imageFormat      = surfaceFormat.format;
    createInfo.imageColorSpace  = surfaceFormat.colorSpace;
    createInfo.imageExtent      = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    query::QueueFamilyIndices indices   = query::find_queue_families( physical_device_, surface_ );
    const uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value( ), indices.presentFamily.value( ) };

    // The imageArrayLayers specifies the amount of layers each image consists of. This is always 1 unless you are
    // developing a stereoscopic 3D application.
    if ( indices.graphicsFamily != indices.presentFamily )
    {
        createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices   = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;       // Optional
        createInfo.pQueueFamilyIndices   = nullptr; // Optional
    }

    // We need to specify how to handle swap chain images that will be used across multiple queue families.
    // There are two ways to handle images that are accessed from multiple queues:
    // 1. VK_SHARING_MODE_EXCLUSIVE: An image is owned by one queue family at a time and ownership must be explicitly
    // transferred before using it in another queue family. This option offers the best performance.
    // 2. VK_SHARING_MODE_CONCURRENT: Images can be used across multiple queue families without explicit ownership
    // transfers.
    createInfo.preTransform   = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode    = presentMode;
    createInfo.clipped        = VK_TRUE;
    createInfo.oldSwapchain   = VK_NULL_HANDLE;

    if ( const auto result = vkCreateSwapchainKHR( device_, &createInfo, nullptr, &swapchain_ ); result != VK_SUCCESS )
    {
        throw std::runtime_error( "Failed to create swap chain! Error: " + get_result_string( result ) );
    }

    // Query image handles
    vkGetSwapchainImagesKHR( device_, swapchain_, &imageCount, nullptr );
    swapchain_images_.resize( imageCount );
    vkGetSwapchainImagesKHR( device_, swapchain_, &imageCount, swapchain_images_.data( ) );

    // Store the swap chain image format and extent for later use
    swapchain_image_format = surfaceFormat.format;
    swapchain_extent       = extent;
}

VkSurfaceFormatKHR TriangleApplication::vk_choose_swap_surface_format(
    const std::vector<VkSurfaceFormatKHR>& availableFormats ) const
{
    // The format member specifies the color channels and types. For the color space we'll use SRGB if it is available,
    // because it results in more accurate perceived colors.
    // https://stackoverflow.com/questions/12524623/what-are-the-practical-differences-when-working-with-colors-in-a-linear-vs-a-no
    for ( const auto& availableFormat : availableFormats )
    {
        if ( availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace ==
             VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
        {
            return availableFormat;
        }
    }

    // If that fails then we can settle with the first format that is specified.
    return availableFormats[0];
}

VkPresentModeKHR TriangleApplication::vk_choose_swap_present_mode(
    const std::vector<VkPresentModeKHR>& availablePresentModes ) const
{
    // In the VK_PRESENT_MODE_MAILBOX_KHR present mode, the swap chain is a queue where the display takes an image from
    // the front of the queue when the display is refreshed and the program inserts rendered images at the back of the
    // queue. Instead of blocking the application when the queue is full, the images that are already queued are simply
    // replaced with the newer ones.
    // VK_PRESENT_MODE_MAILBOX_KHR is a very nice trade-off if energy usage is not a concern. It allows us to avoid
    // tearing while still maintaining a fairly low latency by rendering new images that are as up-to-date as possible
    // right until the vertical blank.
    for ( const auto& availablePresentMode : availablePresentModes )
    {
        if ( availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR )
        {
            return availablePresentMode;
        }
    }

    // VK_PRESENT_MODE_FIFO_KHR mode is guaranteed to be available.
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D TriangleApplication::vk_choose_swap_extent( const VkSurfaceCapabilitiesKHR& capabilities ) const
{
    // The swap extent is the resolution of the swap chain images, and it's almost always exactly equal to the
    // resolution of the window that we're drawing to in pixels.
    // We solve this by assuming automatic clamp to the window dimensions if the specified currentExtent is equal to
    // the maximum value of uint32_t. In that case we'll pick the resolution that best matches the window within the
    // minImageExtent and maxImageExtent bounds.
    if ( capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max( ) )
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize( window_ptr_, &width, &height );

        VkExtent2D actualExtent = {
            static_cast<uint32_t>( width ),
            static_cast<uint32_t>( height )
        };

        actualExtent.width = std::clamp( actualExtent.width, capabilities.minImageExtent.width,
                                         capabilities.maxImageExtent.width );
        actualExtent.height = std::clamp( actualExtent.height, capabilities.minImageExtent.height,
                                          capabilities.maxImageExtent.height );

        return actualExtent;
    }
}

void TriangleApplication::vk_create_image_views( )
{
    // To use any VkImage, in the render pipeline we have to create a VkImageView object. An image view is a view into
    // an image. It describes how to access the image and which part of the image to access.
    swapchain_image_views_.resize( swapchain_images_.size( ) );
    for ( size_t i = 0; i < swapchain_images_.size( ); i++ )
    {
        // Populate a create info struct for every image view
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapchain_images_[i];

        // The viewType and format fields specify how the image data should be interpreted.
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format   = swapchain_image_format;

        // The components field allows you to swizzle the color channels around.
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        // The subresourceRange field describes what the image's purpose is and which part of the image should be
        // accessed.
        createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel   = 0;
        createInfo.subresourceRange.levelCount     = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount     = 1;

        if ( const auto result = vkCreateImageView( device_, &createInfo, nullptr, &swapchain_image_views_[i] );
            result != VK_SUCCESS )
        {
            throw std::runtime_error( "Failed to create image views! Error: " + get_result_string( result ) );
        }
    }
}

VkBool32 TriangleApplication::debug_callback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                              VkDebugUtilsMessageTypeFlagsEXT messageType,
                                              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                              void* pUserData )
{
    std::string severityColor{};
    constexpr std::string resetColor{ "\e[0m" };

    switch ( messageSeverity )
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            severityColor = "\033[34m";
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            severityColor = "\033[37m";
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            severityColor = "\033[33m";
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            severityColor = "\033[31m";
            break;
        default:
            severityColor = "\033[36m";
            break;
    }

    std::cerr << severityColor << "validation layer: " << pCallbackData->pMessage << resetColor << std::endl;
    return VK_FALSE;
}

VkResult TriangleApplication::vk_create_debug_utils_messenger_EXT( VkInstance instance,
                                                                   const VkDebugUtilsMessengerCreateInfoEXT*
                                                                   pCreateInfo,
                                                                   const VkAllocationCallbacks* pAllocator,
                                                                   VkDebugUtilsMessengerEXT* pDebugMessenger )
{
    const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>( vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT" ) );
    if ( func != nullptr )
    {
        return func( instance, pCreateInfo, pAllocator, pDebugMessenger );
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void TriangleApplication::vk_destroy_debug_utils_messenger_EXT( VkInstance instance,
                                                                VkDebugUtilsMessengerEXT debugMessenger,
                                                                const VkAllocationCallbacks* pAllocator )
{
    const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>( vkGetInstanceProcAddr(
        instance, "vkDestroyDebugUtilsMessengerEXT" ) );
    if ( func != nullptr )
    {
        func( instance, debugMessenger, pAllocator );
    }
}

void TriangleApplication::init_window( )
{
    // Initializes the GLFW library
    glfwInit( );

    // Disable OpenGL context creation
    glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );

    // Disable window resizing
    // Needs special handling in Vulkan and will deal with it later
    glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

    // Create the window
    window_ptr_ = glfwCreateWindow( WIDTH_, HEIGHT_, "Vulkan Rasterizer", nullptr, nullptr );
}

void TriangleApplication::init_vk( )
{
    vk_create_instance( );

    vk_setup_debug_messenger( );

    vk_create_surface( );

    vk_pick_physical_device( );

    vk_create_logical_device( );

    vk_create_swap_chain( );

    vk_create_image_views( );
}

void TriangleApplication::main_loop( )
{
    glfwPollEvents( );
}

void TriangleApplication::cleanup( ) const
{
    if constexpr ( ENABLE_VALIDATION_LAYERS_ )
    {
        vk_destroy_debug_utils_messenger_EXT( instance_, debug_messenger_, nullptr );
    }

    // Allocation and de-allocation functions in Vulkan have an optional allocator callback
    // that we'll ignore by passing nullptr.
    for ( const auto imageView : swapchain_image_views_ )
    {
        // Unlike images, the image views were explicitly created by us, so we need to destroy them.
        vkDestroyImageView( device_, imageView, nullptr );
    }

    vkDestroySwapchainKHR( device_, swapchain_, nullptr );

    vkDestroyDevice( device_, nullptr );

    vkDestroySurfaceKHR( instance_, surface_, nullptr );

    vkDestroyInstance( instance_, nullptr );

    glfwDestroyWindow( window_ptr_ );

    glfwTerminate( );
}
