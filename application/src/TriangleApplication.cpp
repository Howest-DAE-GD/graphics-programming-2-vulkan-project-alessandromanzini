#include <TriangleApplication.h>

// +---------------------------+
// | STANDARD HEADERS          |
// +---------------------------+
#include <chrono>
#include <functional>
#include <iostream>
#include <set>
#include <stdexcept>

// +---------------------------+
// | PROJECT HEADERS           |
// +---------------------------+
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <ShaderModules.h>
#include <UniformBufferObject.h>
#include <VulkanDeviceQueries.h>

#define STB_IMAGE_IMPLEMENTATION
#include <SingleTimeCommand.h>
#include <stb_image.h>

#include <xos/filesystem.h>
#include <xos/info.h>

#include <filesystem>
#include <iostream>


using namespace engine;


constexpr int GLFW_WINDOW_OPEN{ 0 };


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
            if ( std::strcmp( required_property, getName( available_property ) ) == 0 )
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

    configure_relative_path( );
    init_vk( );

    bool keepRunning{ true };
    while ( keepRunning )
    {
        keepRunning = glfwWindowShouldClose( window_ptr_ ) == GLFW_WINDOW_OPEN;
        main_loop( );
    }
    // All the operations in drawFrame are asynchronous. When we exit the loop, drawing and presentation operations may
    // still be going on. We solve it by waiting for the logical device to finish operations before exiting.
    vkDeviceWaitIdle( device_ );

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
                                                              []( const VkExtensionProperties& extension )
                                                                  {
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
                                                          []( const VkLayerProperties& layer )
                                                              {
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

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures( device, &supportedFeatures );

    return indices.is_suitable( ) && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}


bool TriangleApplication::vk_check_device_extension_support( const VkPhysicalDevice device )
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
    constexpr float queuePriority{ 1.0f };
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
    deviceFeatures.samplerAnisotropy = VK_TRUE;

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

    createInfo.enabledExtensionCount   = static_cast<uint32_t>( extensions.size( ) );
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
    // internal operations before we can acquire another image to render to.
    // We aim for triple buffering.
    constexpr uint32_t bufferingAim{ 3 };
    uint32_t imageCount = std::clamp( bufferingAim, swapChainSupport.capabilities.minImageCount,
                                      swapChainSupport.capabilities.maxImageCount );

    // If maxImageCount doesn't have a cap, it will be set to 0, we adjust it to the buffering aim.
    if ( imageCount == 0 )
    {
        imageCount = bufferingAim;
    }

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

    const query::QueueFamilyIndices indices = query::find_queue_families( physical_device_, surface_ );
    const uint32_t queueFamilyIndices[]     = { indices.graphicsFamily.value( ), indices.presentFamily.value( ) };

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
    swapchain_image_format_ = surfaceFormat.format;
    swapchain_extent_       = extent;
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


void TriangleApplication::vk_recreate_swap_chain( )
{
    // In case the window gets minimized, we wait until it gets restored.
    int width = 0, height = 0;
    glfwGetFramebufferSize( window_ptr_, &width, &height );
    // perhaps use thread sleep instead of while
    while ( width == 0 || height == 0 )
    {
        glfwGetFramebufferSize( window_ptr_, &width, &height );
        glfwWaitEvents( );
    }

    // It is possible to create a new swap chain while drawing commands on an image from the old swap chain are still
    // in-flight. You need to pass the previous swap chain to the oldSwapChain field in the VkSwapchainCreateInfoKHR
    // struct and destroy the old swap chain as soon as you've finished using it.
    vkDeviceWaitIdle( device_ );

    vk_cleanup_swap_chain( );

    vk_create_swap_chain( );
    vk_create_image_views( );
    vk_create_depth_resources( );
    vk_create_frame_buffers( );
}


void TriangleApplication::vk_cleanup_swap_chain( ) const
{
    vkDestroyImageView(device_, depth_image_view_, nullptr);
    vkDestroyImage(device_, depth_image_, nullptr);
    vkFreeMemory(device_, depth_image_memory_, nullptr);

    for ( const auto framebuffer : swapchain_frame_buffers_ )
    {
        vkDestroyFramebuffer( device_, framebuffer, nullptr );
    }

    for ( const auto imageView : swapchain_image_views_ )
    {
        // Unlike images, the image views were explicitly created by us, so we need to destroy them.
        vkDestroyImageView( device_, imageView, nullptr );
    }

    vkDestroySwapchainKHR( device_, swapchain_, nullptr );
}


void TriangleApplication::vk_create_image_views( )
{
    // To use any VkImage, in the render pipeline we have to create a VkImageView object. An image view is a view into
    // an image. It describes how to access the image and which part of the image to access.
    swapchain_image_views_.resize( swapchain_images_.size( ) );

    for ( uint32_t i = 0; i < swapchain_images_.size( ); i++ )
    {
        swapchain_image_views_[i] = vk_create_image_view( swapchain_images_[i], swapchain_image_format_,
                                                          VK_IMAGE_ASPECT_COLOR_BIT );
    }
}


void TriangleApplication::vk_create_render_pass( )
{
    VkAttachmentDescription colorAttachment{};

    // The format of the color attachment should match the format of the swap chain images, and we're not doing anything
    // with multisampling yet, so we'll stick to 1 sample.
    colorAttachment.format  = swapchain_image_format_;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    // The loadOp and storeOp determine what to do with the data in the attachment before rendering and after rendering.
    // The loadOp and storeOp apply to color and depth data, and stencilLoadOp / stencilStoreOp apply to stencil data
    colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // A single render pass can consist of multiple sub-passes. Sub-passes are subsequent rendering operations that
    // depend on the contents of frame buffers in previous passes
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Depth buffer attachment
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format         = query::find_depth_format( physical_device_ );
    depthAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    // Remember that the subpasses in a render pass automatically take care of image layout transitions. These
    // transitions are controlled by subpass dependencies, which specify memory and execution dependencies between
    // subpasses.Subpass dependencies are specified in VkSubpassDependency structs.
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;

    dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;

    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Now we create a render pass object by filling in the VkRenderPassCreateInfo structure with out attachments and
    // sub-passes.
    const std::array attachments{ colorAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>( attachments.size( ) );
    renderPassInfo.pAttachments    = attachments.data( );
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = &subpass;

    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies   = &dependency;

    // Make dependency referer to the attachments
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    if ( const auto result = vkCreateRenderPass( device_, &renderPassInfo, nullptr, &render_pass_ );
        result != VK_SUCCESS )
    {
        throw std::runtime_error( "Failed to create render pass! Error: " + get_result_string( result ) );
    }
}


void TriangleApplication::vk_create_descriptor_set_layout( )
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};

    // The first two fields specify the binding used in the shader and the type of descriptor, which is a uniform buffer
    // object. It is possible for the shader variable to represent an array of uniform buffer objects, and
    // descriptorCount specifies the number of values in the array.
    uboLayoutBinding.binding         = 0;
    uboLayoutBinding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;

    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding            = 1;
    samplerLayoutBinding.descriptorCount    = 1;
    samplerLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;

    const std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

    // We need to specify the descriptor set layout during pipeline creation to tell Vulkan which descriptors the
    // shaders will be using.
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>( bindings.size( ) );
    layoutInfo.pBindings    = bindings.data( );

    if ( const auto result = vkCreateDescriptorSetLayout( device_, &layoutInfo, nullptr, &descriptor_set_layout_ );
        result != VK_SUCCESS )
    {
        throw std::runtime_error( "Failed to create descriptor set layout! " + get_result_string( result ) );
    }
}


void TriangleApplication::vk_create_graphics_pipeline( )
{
    const auto vertShaderCode = shader::read_file( "shaders/shader.vert.spv" );
    const auto fragShaderCode = shader::read_file( "shaders/shader.frag.spv" );

    // Shader modules are just a thin wrapper around the shader bytecode that we've previously loaded from a file and
    // the functions defined in it.
    VkShaderModule vertShaderModule = shader::create_shader_module( device_, vertShaderCode );
    VkShaderModule fragShaderModule = shader::create_shader_module( device_, fragShaderCode );

    // We assign the shaders to specific pipelines stages through the shaderStages member of the
    // VkPipelineShaderStageCreateInfo
    VkPipelineShaderStageCreateInfo shaderStages[2] = {};

    // 0. Vertex Shader
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;

    shaderStages[0].module = vertShaderModule;
    shaderStages[0].pName  = "main";

    // 1. Fragment Shader
    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;

    shaderStages[1].module = fragShaderModule;
    shaderStages[1].pName  = "main";

    // The VkPipelineVertexInputStateCreateInfo structure describes the format of the vertex data passed to the vertex
    // shader. It describes this in roughly two ways:
    // 1. Bindings: spacing between data and whether the data is per-vertex or per-instance.
    // 2. Attribute descriptions: type of the attributes passed to the vertex shader, which binding to load them from
    // and at which offset.
    auto bindingDescription    = Vertex::get_binding_description( );
    auto attributeDescriptions = Vertex::get_attribute_descriptions( );

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.pVertexBindingDescriptions      = &bindingDescription;
    vertexInputInfo.vertexBindingDescriptionCount   = 1;
    vertexInputInfo.pVertexAttributeDescriptions    = attributeDescriptions.data( );
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>( attributeDescriptions.size( ) );

    // The VkPipelineInputAssemblyStateCreateInfo struct describes two things: what kind of geometry will be drawn from
    // the vertices and if primitive restart should be enabled.
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // At last, we specify their count at pipeline creation time.
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount  = 1;

    // If we were to define the scissors and viewports statically, we would have to create a new viewportState every
    // time we need a change
    // viewportState.pViewports = &viewport;
    // viewportState.pScissors  = &scissor;

    // The rasterizer takes the geometry that is shaped by the vertices from the vertex shader and turns it into
    // fragments to be colored by the fragment shader. It also performs depth testing, face culling and the scissor
    // test.
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable        = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth               = 1.0f;
    rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace               = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable         = VK_FALSE;

    // Culling
    rasterizer.cullMode  = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    // The VkPipelineMultisampleStateCreateInfo struct configures multisampling, which is one of the ways to perform
    // antialiasing.
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable   = VK_FALSE;
    multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading      = 1.0f;     // Optional
    multisampling.pSampleMask           = nullptr;  // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable      = VK_FALSE; // Optional

    // After a fragment shader has returned a color, it needs to be combined with the color that is already in the
    // framebuffer. This transformation is known as color blending and there are two ways to do it:
    // 1. Mix the old and new value to produce a final color.
    // 2. Combine the old and new value using a bitwise operation.
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
                                          | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable         = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;      // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;      // Optional

    // The second structure references the array of structures for all the frame buffers and allows you to set blend
    // constants that you can use as blend factors.
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable     = VK_FALSE;
    colorBlending.logicOp           = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount   = 1;
    colorBlending.pAttachments      = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    // Dynamic state represents the states that can be changed without recreating the pipeline.
    // This setup will cause the configuration of these values to be ignored, and you will be able (and required) to
    // specify the data at drawing time. This results in a more flexible setup and is very common for things like
    // viewport and scissor state, which would result in a more complex setup when being baked into the pipeline state.
    std::vector dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>( dynamicStates.size( ) );
    dynamicState.pDynamicStates    = dynamicStates.data( );

    // You can use uniform values in shaders, which are globals similar to dynamic state variables that can be changed
    // at drawing time to alter the behavior of your shaders without having to recreate them. They are commonly used to
    // pass the transformation matrix to the vertex shader, or to create texture samplers in the fragment shader.
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts    = &descriptor_set_layout_;

    if ( const auto result = vkCreatePipelineLayout( device_, &pipelineLayoutInfo, nullptr, &pipeline_layout_ );
        result != VK_SUCCESS )
    {
        throw std::runtime_error( "Failed to create pipeline layout! Error: " + get_result_string( result ) );
    }

    // Depth stencil creation
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable  = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;

    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds        = 0.0f; // Optional
    depthStencil.maxDepthBounds        = 1.0f; // Optional

    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front             = {}; // Optional
    depthStencil.back              = {}; // Optional

    // We can now combine all the structures and objects to create the graphics pipeline.
    // 1. Shader stages: the shader modules that define the functionality of the programmable stages of the
    // graphics pipeline.
    // 2. Fixed-function state: all the structures that define the fixed-function stages of the pipeline, like input
    // assembly, rasterizer, viewport and color blending.
    // 3. Pipeline layout: the uniform and push values referenced by the shader that can be updated at draw time.
    // 4. Render pass: the attachments referenced by the pipeline stages and their usage.

    // 1. We start by referencing the array of VkPipelineShaderStageCreateInfo structs.
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages    = shaderStages;

    // 2. Then we reference all the structures describing the fixed-function stage.
    pipelineInfo.pVertexInputState   = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState      = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState   = &multisampling;
    pipelineInfo.pDepthStencilState  = nullptr; // Optional
    pipelineInfo.pColorBlendState    = &colorBlending;
    pipelineInfo.pDynamicState       = &dynamicState;

    // 3. After that comes the pipeline layout, which is a Vulkan handle rather than a struct pointer.
    pipelineInfo.layout = pipeline_layout_;

    // 4. And finally we have the reference to the render pass and the index of the sub pass where this graphics
    // pipeline will be used.
    pipelineInfo.renderPass = render_pass_;
    pipelineInfo.subpass    = 0;

    // There are actually two optional more parameters: basePipelineHandle and basePipelineIndex. Vulkan allows you to
    // create a new graphics pipeline by deriving from an existing pipeline. The idea of pipeline derivatives is that
    // it is less expensive to set up pipelines when they have much functionality in common with an existing pipeline
    // and switching between pipelines from the same parent can also be done quicker.
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex  = -1;             // Optional

    pipelineInfo.pDepthStencilState = &depthStencil;

    // The vkCreateGraphicsPipelines is designed to take multiple VkGraphicsPipelineCreateInfo objects and create
    // multiple VkPipeline objects in a single call.
    // The second parameter references an optional VkPipelineCache object. A pipeline cache can be used to store and
    // reuse data relevant to pipeline creation across multiple calls to vkCreateGraphicsPipelines.
    if ( const auto result = vkCreateGraphicsPipelines( device_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                                        &graphics_pipeline_ ); result != VK_SUCCESS )
    {
        throw std::runtime_error( "Failed to create graphics pipeline! Error: " + get_result_string( result ) );
    }

    // The compilation and linking of the SPIR-V bytecode to machine code for execution
    // by the GPU doesn't happen until the graphics pipeline is created. That means that we're allowed to destroy the
    // shader modules again as soon as pipeline creation is finished.
    vkDestroyShaderModule( device_, fragShaderModule, nullptr );
    vkDestroyShaderModule( device_, vertShaderModule, nullptr );
}


void TriangleApplication::vk_create_buffer( const VkDeviceSize size, const VkBufferUsageFlags usage,
                                            const VkMemoryPropertyFlags properties, VkBuffer& buffer,
                                            VkDeviceMemory& bufferMemory ) const
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size        = size;
    bufferInfo.usage       = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if ( const auto result = vkCreateBuffer( device_, &bufferInfo, nullptr, &buffer ); result != VK_SUCCESS )
    {
        throw std::runtime_error( "Failed to create buffer! " + get_result_string( result ) );
    }

    // The first step of allocating memory for the buffer is to query its memory requirements.
    // The VkMemoryRequirements struct has three fields:
    // 1. size: The size of the required amount of memory in bytes, may differ from bufferInfo.size.
    // 2. alignment: The offset in bytes where the buffer begins in the allocated region of memory, depends on
    //    bufferInfo.usage and bufferInfo.flags
    // 3. memoryTypeBits: Bit field of the memory types that are suitable for the buffer.
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements( device_, buffer, &memRequirements );

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize  = memRequirements.size;
    allocInfo.memoryTypeIndex = query::find_memory_type( physical_device_, memRequirements.memoryTypeBits, properties );

    if ( const auto result = vkAllocateMemory( device_, &allocInfo, nullptr, &bufferMemory ); result != VK_SUCCESS )
    {
        throw std::runtime_error( "Failed to allocate buffer memory! " + get_result_string( result ) );
    }

    // If memory allocation was successful, then we can now associate this memory with the buffer.
    // Since this memory is allocated specifically for this vertex buffer, the offset is simply 0. If the offset is
    // non-zero, then it is required to be divisible by memRequirements.alignment.
    vkBindBufferMemory( device_, buffer, bufferMemory, 0 );
}


void TriangleApplication::vk_copy_buffer( const VkBuffer srcBuffer, const VkBuffer dstBuffer,
                                          const VkDeviceSize size ) const
{
    VkCommandBuffer commandBuffer{ begin_single_time_commands( device_, command_pool_ ) };

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer( commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion );

    end_single_time_commands( device_, command_pool_, commandBuffer, graphics_queue_ );
}


void TriangleApplication::vk_create_image( const uint32_t width, const uint32_t height, const VkFormat format,
                                           const VkImageTiling tiling, const VkImageUsageFlags usage,
                                           const VkMemoryPropertyFlags properties,
                                           VkImage& image, VkDeviceMemory& imageMemory ) const
{
    // Create texture image
    VkImageCreateInfo imageInfo{};
    imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType     = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width  = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth  = 1;
    imageInfo.mipLevels     = 1;
    imageInfo.arrayLayers   = 1;

    // Tell vulkan what kind of texels we are going to use
    imageInfo.format = format;

    // The tiling field can have one of two values:
    // 1. VK_IMAGE_TILING_LINEAR: Texels are laid out in row major order like our pixels array.
    // 2. VK_IMAGE_TILING_OPTIMAL: Texels are laid out in an implementation defined order for optimal access.
    imageInfo.tiling = tiling;

    // There are only two possible values for the initialLayout of an image:
    // 1. VK_IMAGE_LAYOUT_UNDEFINED: Not usable by the GPU and the very first transition will discard the texels.
    // 2. VK_IMAGE_LAYOUT_PREINITIALIZED: Not usable by the GPU, but the first transition will preserve the texels.
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    imageInfo.usage       = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples     = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags       = 0; // Optional

    if ( const auto result = vkCreateImage( device_, &imageInfo, nullptr, &image ); result != VK_SUCCESS )
    {
        throw std::runtime_error( "Failed to create image! " + get_result_string( result ) );
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements( device_, image, &memRequirements );

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize  = memRequirements.size;
    allocInfo.memoryTypeIndex = query::find_memory_type( physical_device_, memRequirements.memoryTypeBits,
                                                         properties );

    if ( vkAllocateMemory( device_, &allocInfo, nullptr, &imageMemory ) != VK_SUCCESS )
    {
        throw std::runtime_error( "failed to allocate image memory!" );
    }

    vkBindImageMemory( device_, image, imageMemory, 0 );
}


VkImageView TriangleApplication::vk_create_image_view( const VkImage image, const VkFormat format,
                                                       const VkImageAspectFlags aspectFlags ) const
{
    // Populate a create info struct for every image view
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;

    // The viewType and format fields specify how the image data should be interpreted.
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format   = format;

    // The components field allows you to swizzle the color channels around.
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    // The subresourceRange field describes what the image's purpose is and which part of the image should be
    // accessed.
    createInfo.subresourceRange.aspectMask     = aspectFlags;
    createInfo.subresourceRange.baseMipLevel   = 0;
    createInfo.subresourceRange.levelCount     = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount     = 1;

    VkImageView imageView;
    if ( const auto result = vkCreateImageView( device_, &createInfo, nullptr, &imageView ); result != VK_SUCCESS )
    {
        throw std::runtime_error( "Failed to create texture image view! Result: " + get_result_string( result ) );
    }

    return imageView;
}


void TriangleApplication::vk_create_texture_image( )
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load( "resources/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha );
    const VkDeviceSize imageSize = texWidth * texHeight * 4;

    if ( not pixels )
    {
        throw std::runtime_error( "Failed to load texture image!" );
    }

    vk_create_buffer( imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer_,
                      staging_buffer_memory_ );

    void* data;
    vkMapMemory( device_, staging_buffer_memory_, 0, imageSize, 0, &data );
    memcpy( data, pixels, static_cast<size_t>( imageSize ) );
    vkUnmapMemory( device_, staging_buffer_memory_ );

    // Cleanup original pixel data
    stbi_image_free( pixels );

    vk_create_image( static_cast<uint32_t>( texWidth ), static_cast<uint32_t>( texHeight ), VK_FORMAT_R8G8B8A8_SRGB,
                     VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture_image_, texture_image_memory_ );

    vk_transition_image_layout( texture_image_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );
    vk_copy_buffer_to_image( staging_buffer_, texture_image_, static_cast<uint32_t>( texWidth ),
                             static_cast<uint32_t>( texHeight ) );
    vk_transition_image_layout( texture_image_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

    vkDestroyBuffer( device_, staging_buffer_, nullptr );
    vkFreeMemory( device_, staging_buffer_memory_, nullptr );
}


void TriangleApplication::vk_create_texture_image_view( )
{
    texture_image_view_ = vk_create_image_view( texture_image_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT );
}


void TriangleApplication::vk_create_texture_sampler( )
{
    VkSamplerCreateInfo samplerInfo{};

    // The magFilter and minFilter fields specify how to interpolate texels that are magnified or minified.
    samplerInfo.sType     = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;

    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties( physical_device_, &properties );

    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy    = properties.limits.maxSamplerAnisotropy;

    // samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable           = VK_FALSE;
    samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod     = 0.0f;
    samplerInfo.maxLod     = 0.0f;

    if ( const auto result = vkCreateSampler( device_, &samplerInfo, nullptr, &texture_sampler_ );
        result != VK_SUCCESS )
    {
        throw std::runtime_error( "Failed to create texture sampler! Error: " + get_result_string( result ) );
    }
}


void TriangleApplication::vk_transition_image_layout( const VkImage image, const VkFormat /* format */,
                                                      const VkImageLayout oldLayout,
                                                      const VkImageLayout newLayout ) const
{
    const VkCommandBuffer commandBuffer = begin_single_time_commands( device_, command_pool_ );

    VkImageMemoryBarrier barrier{};
    barrier.sType     = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image                           = image;
    barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = 1;

    barrier.srcAccessMask = 0; // TODO
    barrier.dstAccessMask = 0; // TODO

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if ( oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL )
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if ( oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout ==
              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL )
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if ( oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL )
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else
    {
        throw std::invalid_argument( "Unsupported layout transition!" );
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    end_single_time_commands( device_, command_pool_, commandBuffer, graphics_queue_ );
}


void TriangleApplication::vk_copy_buffer_to_image( const VkBuffer buffer, const VkImage image, const uint32_t width,
                                                   const uint32_t height ) const
{
    const VkCommandBuffer commandBuffer = begin_single_time_commands( device_, command_pool_ );

    VkBufferImageCopy region{};
    region.bufferOffset      = 0;
    region.bufferRowLength   = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    end_single_time_commands( device_, command_pool_, commandBuffer, graphics_queue_ );
}


void TriangleApplication::vk_create_vertex_buffer( )
{
    const VkDeviceSize bufferSize = sizeof( vertices_[0] ) * vertices_.size( );
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    // We're now using a new stagingBuffer with stagingBufferMemory for mapping and copying the vertex data. This will
    // load vertex data from high performance memory.
    // - VK_BUFFER_USAGE_TRANSFER_SRC_BIT: Buffer can be used as source in a memory transfer operation.
    // - VK_BUFFER_USAGE_TRANSFER_DST_BIT: Buffer can be used as destination in a memory transfer operation.
    vk_create_buffer( bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      stagingBuffer, stagingBufferMemory );

    // It is now time to copy the vertex data to the buffer. This is done by mapping the buffer memory into CPU
    // accessible memory. memcpy the vertex data to the mapped memory and unmap it again.
    void* data;
    vkMapMemory( device_, stagingBufferMemory, 0, bufferSize, 0, &data );

    // ReSharper disable once CppRedundantCastExpression
    memcpy( data, vertices_.data( ), static_cast<size_t>( bufferSize ) );

    vkUnmapMemory( device_, stagingBufferMemory );

    vk_create_buffer( bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertex_buffer_, vertex_buffer_memory_ );

    vk_copy_buffer( stagingBuffer, vertex_buffer_, bufferSize );

    // We can now destroy the staging buffer and free its memory.
    vkDestroyBuffer( device_, stagingBuffer, nullptr );
    vkFreeMemory( device_, stagingBufferMemory, nullptr );
}


void TriangleApplication::vk_create_index_buffer( )
{
    const VkDeviceSize bufferSize = sizeof( indices_[0] ) * indices_.size( );

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    vk_create_buffer( bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                      stagingBufferMemory );

    void* data;
    vkMapMemory( device_, stagingBufferMemory, 0, bufferSize, 0, &data );

    // ReSharper disable once CppRedundantCastExpression
    memcpy( data, indices_.data( ), static_cast<size_t>( bufferSize ) );

    vkUnmapMemory( device_, stagingBufferMemory );

    vk_create_buffer( bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, index_buffer_, index_buffer_memory_ );

    vk_copy_buffer( stagingBuffer, index_buffer_, bufferSize );

    vkDestroyBuffer( device_, stagingBuffer, nullptr );
    vkFreeMemory( device_, stagingBufferMemory, nullptr );
}


void TriangleApplication::vk_create_frame_buffers( )
{
    swapchain_frame_buffers_.resize( swapchain_image_views_.size( ) );

    for ( size_t i = 0; i < swapchain_image_views_.size( ); i++ )
    {
        std::array attachments = {
            swapchain_image_views_[i],
            depth_image_view_
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

        // We first need to specify with which renderPass the framebuffer needs to be compatible.
        framebufferInfo.renderPass = render_pass_;

        // The attachmentCount and pAttachments parameters specify the VkImageView objects that should be bound to the
        // respective attachment descriptions in the render pass pAttachment array.
        framebufferInfo.attachmentCount = static_cast<uint32_t>( attachments.size( ) );
        framebufferInfo.pAttachments    = attachments.data( );

        framebufferInfo.width  = swapchain_extent_.width;
        framebufferInfo.height = swapchain_extent_.height;
        framebufferInfo.layers = 1;

        if ( const auto result = vkCreateFramebuffer( device_, &framebufferInfo, nullptr, &swapchain_frame_buffers_[i] )
            ;
            result != VK_SUCCESS )
        {
            throw std::runtime_error( "Failed to create framebuffer! Error: " + get_result_string( result ) );
        }
    }
}


void TriangleApplication::vk_create_uniform_buffers( )
{
    uniform_buffers_.resize( MAX_FRAMES_IN_FLIGHT_ );
    uniform_buffers_memory_.resize( MAX_FRAMES_IN_FLIGHT_ );
    uniform_buffers_mapped_.resize( MAX_FRAMES_IN_FLIGHT_ );

    // We map the buffer right after creation using vkMapMemory to get a pointer to which we can write the data later on.
    // The buffer stays mapped to this pointer for the application's whole lifetime. This technique is called
    // "persistent mapping" and works on all Vulkan implementations.
    for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT_; i++ )
    {
        constexpr VkDeviceSize bufferSize = sizeof( UniformBufferObject );
        vk_create_buffer( bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                          uniform_buffers_[i], uniform_buffers_memory_[i] );

        vkMapMemory( device_, uniform_buffers_memory_[i], 0, bufferSize, 0, &uniform_buffers_mapped_[i] );
    }
}


void TriangleApplication::vk_create_descriptor_pool( )
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>( MAX_FRAMES_IN_FLIGHT_ );
    poolSizes[1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>( MAX_FRAMES_IN_FLIGHT_ );

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>( poolSizes.size( ) );
    poolInfo.pPoolSizes    = poolSizes.data( );

    poolInfo.maxSets = static_cast<uint32_t>( MAX_FRAMES_IN_FLIGHT_ );

    if ( const auto result = vkCreateDescriptorPool( device_, &poolInfo, nullptr, &descriptor_pool_ );
        result != VK_SUCCESS )
    {
        throw std::runtime_error( "Failed to create descriptor pool! Error: " + get_result_string( result ) );
    }
}


void TriangleApplication::vk_create_descriptor_sets( )
{
    const std::vector<VkDescriptorSetLayout> layouts( MAX_FRAMES_IN_FLIGHT_, descriptor_set_layout_ );
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool     = descriptor_pool_;
    allocInfo.descriptorSetCount = static_cast<uint32_t>( MAX_FRAMES_IN_FLIGHT_ );
    allocInfo.pSetLayouts        = layouts.data( );

    descriptor_sets_.resize( MAX_FRAMES_IN_FLIGHT_ );
    if ( const auto result = vkAllocateDescriptorSets( device_, &allocInfo, descriptor_sets_.data( ) );
        result != VK_SUCCESS )
    {
        throw std::runtime_error( "Failed to allocate descriptor sets! " + get_result_string( result ) );
    }

    for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT_; i++ )
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniform_buffers_[i];
        bufferInfo.offset = 0;
        bufferInfo.range  = sizeof( UniformBufferObject );

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView   = texture_image_view_;
        imageInfo.sampler     = texture_sampler_;

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
        descriptorWrites[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet          = descriptor_sets_[i];
        descriptorWrites[0].dstBinding      = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo     = &bufferInfo;

        descriptorWrites[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet          = descriptor_sets_[i];
        descriptorWrites[1].dstBinding      = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo      = &imageInfo;

        vkUpdateDescriptorSets( device_, static_cast<uint32_t>( descriptorWrites.size( ) ),
                                descriptorWrites.data( ), 0, nullptr );
    }
}


void TriangleApplication::vk_create_command_pool( )
{
    const auto queueFamilyIndices = query::find_queue_families( physical_device_, surface_ );

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

    // There are two possible flags for command pools:
    // 1. VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: Hint that command buffers are rerecorded with new commands very often.
    // 2. VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: Allow command buffers to be rerecorded individually, without
    // this flag they all have to be reset together.
    // We will be recording a command buffer every frame, so we want to be able to reset and rerecord over it.
    poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value( );

    if ( const auto result = vkCreateCommandPool( device_, &poolInfo, nullptr, &command_pool_ ); result != VK_SUCCESS )
    {
        throw std::runtime_error( "Failed to create command pool! Error: " + get_result_string( result ) );
    }
}


void TriangleApplication::vk_create_command_buffers( )
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = command_pool_;

    // The level parameter specifies if the allocated command buffers are primary or secondary command buffers.
    // - VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for execution, but cannot be called from other
    // command buffers.
    // - VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly, but can be called from primary command buffers
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>( command_buffers_.size() );

    if ( const auto result = vkAllocateCommandBuffers( device_, &allocInfo, command_buffers_.data( ) );
        result != VK_SUCCESS )
    {
        throw std::runtime_error( "Failed to allocate command buffers! Error: " + get_result_string( result ) );
    }
}


void TriangleApplication::vk_create_sync_objects( )
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    // The VK_FENCE_CREATE_SIGNALED_BIT flag specifies that the fence object is created in the signaled state.
    // This allows us to skip the first render frame, or we would otherwise be stuck waiting at the start.
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT_; i++ )
    {
        if ( const auto result = vkCreateSemaphore( device_, &semaphoreInfo, nullptr, &image_available_semaphores_[i] );
            result != VK_SUCCESS )
        {
            throw std::runtime_error( "Failed to create image semaphore! Error: " + get_result_string( result ) );
        }
        if ( const auto result = vkCreateSemaphore( device_, &semaphoreInfo, nullptr, &render_finished_semaphores_[i] );
            result != VK_SUCCESS )
        {
            throw std::runtime_error( "Failed to create render semaphore! Error: " + get_result_string( result ) );
        }
        if ( const auto result = vkCreateFence( device_, &fenceInfo, nullptr, &in_flight_fences_[i] );
            result != VK_SUCCESS )
        {
            throw std::runtime_error( "Failed to create fence! Error: " + get_result_string( result ) );
        }
    }
}


void TriangleApplication::vk_create_depth_resources( )
{
    const VkFormat depthFormat = query::find_depth_format( physical_device_ );

    vk_create_image( swapchain_extent_.width, swapchain_extent_.height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depth_image_,
                     depth_image_memory_ );
    depth_image_view_ = vk_create_image_view( depth_image_, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT );

    // vk_transition_image_layout( depth_image_, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
    //                             VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL );
}


VkBool32 TriangleApplication::debug_callback( const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                              VkDebugUtilsMessageTypeFlagsEXT /* messageType */,
                                              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                              void* /* pUserData */ )
{
    std::string severityColor{};

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    constexpr std::string_view resetColor{ "\e[0m" };
#pragma GCC diagnostic pop
#else
#pragma warning(push, 0)
    constexpr std::string_view resetColor{ "\e[0m" };
#pragma warning(pop)
#endif

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


void TriangleApplication::frame_buffer_size_callback( GLFWwindow* pWindow, int /* width */, int /* height */ )
{
    auto* app                  = static_cast<TriangleApplication*>( glfwGetWindowUserPointer( pWindow ) );
    app->frame_buffer_resized_ = true;
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
    const auto func =
            reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>( vkGetInstanceProcAddr(
                instance, "vkDestroyDebugUtilsMessengerEXT" ) );
    if ( func != nullptr )
    {
        func( instance, debugMessenger, pAllocator );
    }
}


void TriangleApplication::record_command_buffer( const VkCommandBuffer commandBuffer, // NOLINT(*-misplaced-const)
                                                 const uint32_t imageIndex ) const
{
    // We always begin recording a command buffer by calling vkBeginCommandBuffer with a small VkCommandBufferBeginInfo
    // structure as argument that specifies some details about the usage of this specific command buffer.
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    // The flags parameter specifies how we're going to use the command buffer. The following values are available:
    // - VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will be rerecorded right after executing it
    // once.
    // - VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: This is a secondary command buffer that will be entirely
    // within a single render pass.
    // - VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: The command buffer can be resubmitted while it is also already
    // pending execution.
    // None of the flags are applicable for now.
    beginInfo.flags            = 0;       // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if ( const auto result = vkBeginCommandBuffer( commandBuffer, &beginInfo ); result != VK_SUCCESS )
    {
        throw std::runtime_error( "Failed to begin recording command buffer! Error: " + get_result_string( result ) );
    }

    // Drawing starts by beginning the render pass with vkCmdBeginRenderPass. The render pass is configured using some
    // parameters in a VkRenderPassBeginInfo struct.
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass  = render_pass_;
    renderPassInfo.framebuffer = swapchain_frame_buffers_[imageIndex];

    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = swapchain_extent_;

    // The range of depths in the depth buffer is 0.0 to 1.0 in Vulkan, where 1.0 lies at the far view plane and 0.0 at
    // the near view plane.
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color        = { { 0.0f, 0.0f, 0.0f, 1.0f } };
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>( clearValues.size( ) );
    renderPassInfo.pClearValues    = clearValues.data( );

    // The first parameter for every command is always the command buffer to record the command to. The second parameter
    // specifies the details of the render pass we've just provided. The final parameter controls how the drawing
    // commands within the render pass will be provided. It can have one of two values:
    // 1. VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be embedded in the primary command buffer itself
    // and no secondary command buffers will be executed.
    // 2. VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: The render pass commands will be executed from secondary
    // command buffers.
    vkCmdBeginRenderPass( commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

    // We can now bind the graphics pipeline. The second parameter specifies if the pipeline object is a graphics or
    // compute pipeline.
    vkCmdBindPipeline( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_ );

    // We did specify viewport and scissor state for this pipeline to be dynamic. So we need to set them in the command
    // buffer before issuing our draw command.
    VkViewport viewport{};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = static_cast<float>( swapchain_extent_.width );
    viewport.height   = static_cast<float>( swapchain_extent_.height );
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport( commandBuffer, 0, 1, &viewport );

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapchain_extent_;
    vkCmdSetScissor( commandBuffer, 0, 1, &scissor );

    const VkBuffer vertexBuffers[]{ vertex_buffer_ };
    constexpr VkDeviceSize offsets[]{ 0 };
    vkCmdBindVertexBuffers( commandBuffer, 0, 1, vertexBuffers, offsets );

    vkCmdBindIndexBuffer( commandBuffer, index_buffer_, 0, VK_INDEX_TYPE_UINT32 );

    // Bind the right descriptor set for each frame to the descriptors in the shader.
    vkCmdBindDescriptorSets( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, 0, 1,
                             &descriptor_sets_[current_frame_], 0, nullptr );

    // We now issue the draw command. The number of indices represents the number of vertices that will be passed to
    // the vertex shader.
    vkCmdDrawIndexed( commandBuffer, static_cast<uint32_t>( indices_.size( ) ), 1, 0, 0, 0 );

    // After we've finished recording the command buffer, we end the render pass with vkCmdEndRenderPass.
    vkCmdEndRenderPass( commandBuffer );

    if ( const auto result = vkEndCommandBuffer( commandBuffer ); result != VK_SUCCESS )
    {
        throw std::runtime_error( "Failed to record command buffer! Error: " + get_result_string( result ) );
    }
}


void TriangleApplication::update_uniform_buffer( const uint32_t currentImage ) const
{
    static auto startTime = std::chrono::high_resolution_clock::now( );

    const auto currentTime = std::chrono::high_resolution_clock::now( );
    const float time = std::chrono::duration<float, std::chrono::seconds::period>( currentTime - startTime ).count( );

    UniformBufferObject ubo{};
    ubo.model = glm::rotate( glm::mat4( 1.0f ), time * glm::radians( 90.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    ubo.view  = glm::lookAt( glm::vec3( 2.0f, 2.0f, 2.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ),
                             glm::vec3( 0.0f, 0.0f, 1.0f ) );
    ubo.proj = glm::perspective( glm::radians( 45.0f ),
                                 swapchain_extent_.width / static_cast<float>( swapchain_extent_.height ), 0.1f,
                                 10.0f );
    ubo.proj[1][1] *= -1;

    memcpy( uniform_buffers_mapped_[currentImage], &ubo, sizeof( ubo ) );
}


void TriangleApplication::draw_frame( )
{
    // At a high level, rendering a frame in Vulkan consists of a common set of steps:
    // 1. Wait for the previous frame to finish.
    // 2. Acquire an image from the swap chain.
    // 3. Record a command buffer which draws the scene onto that image.
    // 4. Submit the recorded command buffer.
    // 5. Present the swap chain image.

    // A core design philosophy in Vulkan is that synchronization of execution on the GPU is explicit. The order of
    // operations is up to us to define using various synchronization primitives.

    // A semaphore is used to add order between queue operations. Queue operations refer to the work we submit to a
    // queue, either in a command buffer or from within a function. The queueing happens on the CPU, but the execution
    // and the waiting happen on the GPU.

    // A fence has a similar purpose, in that it is used to synchronize execution, but it is for ordering the execution
    // on the CPU. Simply put, if the host needs to know when the GPU has finished something, we use a fence.

    // In summary, semaphores are used to specify the execution order of operations on the GPU while fences are used to
    // keep the CPU and GPU in sync with each-other.

    // We'll need one semaphore to signal that an image has been acquired from the swapchain and is ready for rendering,
    // another one to signal that rendering has finished and presentation can happen, and a fence to make sure only one
    // frame is rendering at a time.

    // The VK_TRUE we pass here indicates that we want to wait for all fences, but in the case of a single one it
    // doesn't matter. This function also has a timeout parameter set to UINT64_MAX, which effectively disables the
    // timeout.
    vkWaitForFences( device_, 1, &in_flight_fences_[current_frame_], VK_TRUE, UINT64_MAX );

    // The first two parameters of vkAcquireNextImageKHR are the logical device and the swap chain from which we wish
    // to acquire an image.
    uint32_t imageIndex{};
    const auto acquireImageResult = vkAcquireNextImageKHR( device_, swapchain_, UINT64_MAX,
                                                           image_available_semaphores_[current_frame_],
                                                           VK_NULL_HANDLE,
                                                           &imageIndex );

    // If viewport changes, recreate the swap chain.
    // - VK_ERROR_OUT_OF_DATE_KHR: The swap chain has become incompatible with the surface and can no longer be used for
    // rendering. Usually happens after a window resize.
    // - VK_SUBOPTIMAL_KHR: The swap chain can still be used to successfully present to the surface, but the surface
    // properties are no longer matched exactly.
    if ( acquireImageResult == VK_ERROR_OUT_OF_DATE_KHR )
    {
        vk_recreate_swap_chain( );
        return;
    }
    if ( acquireImageResult != VK_SUCCESS && acquireImageResult != VK_SUBOPTIMAL_KHR )
    {
        throw std::runtime_error( "Failed to acquire image result! Error: " + get_result_string( acquireImageResult ) );
    }

    // After waiting, we need to manually reset the fence to the unsignaled state with the vkResetFences call.
    // We reset the fence only if there's work to do, which is why we're doing it after the resize check.
    // You should not return before work is completed or DEADLOCK will occur.
    vkResetFences( device_, 1, &in_flight_fences_[current_frame_] );

    // With the imageIndex specifying the swap chain image to use in hand, we can now record the command buffer.
    vkResetCommandBuffer( command_buffers_[current_frame_], 0 );
    record_command_buffer( command_buffers_[current_frame_], imageIndex );

    // We need to submit the recorded command buffer to the graphics queue before submitting the image to the swap chain.
    update_uniform_buffer( current_frame_ );

    // Queue submission and synchronization is configured through parameters in the VkSubmitInfo structure.
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    constexpr VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount               = 1;
    submitInfo.pWaitSemaphores                  = &image_available_semaphores_[current_frame_];
    submitInfo.pWaitDstStageMask                = waitStages;

    // Set which command buffers to actually submit for execution.
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &command_buffers_[current_frame_];

    // The signalSemaphoreCount and pSignalSemaphores parameters specify which semaphores to signal once the command
    // buffer(s) have finished execution
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &render_finished_semaphores_[current_frame_];

    // The last parameter references an optional fence that will be signaled when the command buffers finish execution.
    // This allows us to know when it is safe for the command buffer to be reused, thus we want to give it inFlightFence.
    if ( const auto result = vkQueueSubmit( graphics_queue_, 1, &submitInfo, in_flight_fences_[current_frame_] );
        result != VK_SUCCESS )
    {
        throw std::runtime_error( "Failed to submit draw command buffer! Error: " + get_result_string( result ) );
    }

    // The last step of drawing a frame is submitting the result back to the swap chain to have it eventually show up
    // on the screen. Presentation is configured through a VkPresentInfoKHR structure.
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    // These two parameters specify which semaphores to wait on before presentation can happen.
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = &render_finished_semaphores_[current_frame_];

    // These two parameters specify the swap chains to present images to and the index of the image for each swap chain.
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains    = &swapchain_;
    presentInfo.pImageIndices  = &imageIndex;

    // There is one last optional parameter called pResults. It allows you to specify an array of VkResult values to
    // check for every individual swap chain if presentation was successful.
    presentInfo.pResults = nullptr; // Optional

    // The vkQueuePresentKHR function submits the request to present an image to the queue.

    // We check for the callback boolean after the queue presentation to avoid the semaphore to be signaled.
    if ( const auto queuePresentResult = vkQueuePresentKHR( present_queue_, &presentInfo );
        queuePresentResult == VK_ERROR_OUT_OF_DATE_KHR || queuePresentResult == VK_SUBOPTIMAL_KHR ||
        frame_buffer_resized_ )
    {
        frame_buffer_resized_ = false;
        vk_recreate_swap_chain( );
    }

    // Next frame
    current_frame_ = ( current_frame_ + 1 ) % MAX_FRAMES_IN_FLIGHT_;
}


void TriangleApplication::init_window( )
{
    // Initializes the GLFW library
    glfwInit( );

    // Disable OpenGL context creation
    glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );

    // Create the window
    window_ptr_ = glfwCreateWindow( WIDTH_, HEIGHT_, "Vulkan Rasterizer", nullptr, nullptr );
    glfwSetWindowUserPointer( window_ptr_, this );
    glfwSetFramebufferSizeCallback( window_ptr_, frame_buffer_size_callback );
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

    vk_create_render_pass( );

    vk_create_descriptor_set_layout( );

    vk_create_graphics_pipeline( );

    vk_create_command_pool( );

    vk_create_depth_resources( );

    vk_create_frame_buffers( );

    vk_create_texture_image( );
    vk_create_texture_image_view( );
    vk_create_texture_sampler( );

    vk_create_vertex_buffer( );
    vk_create_index_buffer( );
    vk_create_uniform_buffers( );

    vk_create_descriptor_pool( );
    vk_create_descriptor_sets( );

    vk_create_command_buffers( );

    vk_create_sync_objects( );
}


void TriangleApplication::configure_relative_path( ) const
{
    xos::info::log_info( std::clog );
    xos::filesystem::configure_relative_path( );
}


void TriangleApplication::main_loop( )
{
    glfwPollEvents( );

    draw_frame( );
}


void TriangleApplication::cleanup( ) const
{
    if constexpr ( ENABLE_VALIDATION_LAYERS_ )
    {
        vk_destroy_debug_utils_messenger_EXT( instance_, debug_messenger_, nullptr );
    }

    // Allocation and de-allocation functions in Vulkan have an optional allocator callback
    // that we'll ignore by passing nullptr.
    for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT_; i++ )
    {
        vkDestroySemaphore( device_, image_available_semaphores_[i], nullptr );
        vkDestroySemaphore( device_, render_finished_semaphores_[i], nullptr );
        vkDestroyFence( device_, in_flight_fences_[i], nullptr );
    }

    vkDestroyCommandPool( device_, command_pool_, nullptr );

    vkDestroyPipeline( device_, graphics_pipeline_, nullptr );
    vkDestroyPipelineLayout( device_, pipeline_layout_, nullptr );

    vkDestroyRenderPass( device_, render_pass_, nullptr );

    vk_cleanup_swap_chain( );

    vkDestroySampler( device_, texture_sampler_, nullptr );
    vkDestroyImageView( device_, texture_image_view_, nullptr );

    vkDestroyImage( device_, texture_image_, nullptr );
    vkFreeMemory( device_, texture_image_memory_, nullptr );

    for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT_; i++ )
    {
        vkDestroyBuffer( device_, uniform_buffers_[i], nullptr );
        vkFreeMemory( device_, uniform_buffers_memory_[i], nullptr );
    }

    vkDestroyDescriptorPool( device_, descriptor_pool_, nullptr );
    vkDestroyDescriptorSetLayout( device_, descriptor_set_layout_, nullptr );

    vkDestroyBuffer( device_, index_buffer_, nullptr );
    vkFreeMemory( device_, index_buffer_memory_, nullptr );

    vkDestroyBuffer( device_, vertex_buffer_, nullptr );
    vkFreeMemory( device_, vertex_buffer_memory_, nullptr );

    vkDestroyDevice( device_, nullptr );

    vkDestroySurfaceKHR( instance_, surface_, nullptr );

    vkDestroyInstance( instance_, nullptr );

    glfwDestroyWindow( window_ptr_ );

    glfwTerminate( );
}
