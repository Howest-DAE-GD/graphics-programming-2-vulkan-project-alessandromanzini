#include <GLFW/glfw3.h>
#include <instance/VulkanInstance.h>

#include <validation/dispatch.h>
#include <validation/result.h>


namespace cobalt_vk
{
    VulkanInstance::VulkanInstance( const VkApplicationInfo& appInfo, const Window& window )
    {
        create_instance( appInfo, window );
        create_surface( window );
    }


    VulkanInstance::VulkanInstance( const VkApplicationInfo& appInfo, const Window& window,
                                    ValidationLayers::layers_vector_t validationLayers,
                                    ValidationLayers::debug_callback_t&& debugCallback )
        : validation_layers_ptr_{ std::make_unique<ValidationLayers>( std::move( validationLayers ), debugCallback ) }
    {
        create_instance( appInfo, window );
        validation_layers_ptr_->setup_debug_messenger( instance_ );
        create_surface( window );
    }


    VulkanInstance::~VulkanInstance( )
    {
        vkDestroyDevice( device_, nullptr );

        if ( validation_layers_ptr_ )
        {
            validation_layers_ptr_->destroy_debug_messenger( instance_ );
        }

        vkDestroySurfaceKHR( instance_, surface_, nullptr );

        vkDestroyInstance( instance_, nullptr );
    }


    VkInstance VulkanInstance::get_instance( ) const
    {
        return instance_;
    }


    VkDevice VulkanInstance::get_device( ) const
    {
        return device_;
    }


    VkSurfaceKHR VulkanInstance::get_surface( ) const
    {
        return surface_;
    }


    void VulkanInstance::set_device( VkDevice device )
    {
        // TODO: remove this
        device_ = device;
    }


    void VulkanInstance::wait_idle( ) const
    {
        vkDeviceWaitIdle( device_ );
    }


    void VulkanInstance::create_instance( const VkApplicationInfo& appInfo, const Window& window )
    {
        // Tells the Vulkan driver which global extensions and validation layers we want to use.
        // Global here means that they apply to the entire program and not a specific device.
        VkInstanceCreateInfo createInfo{};
        createInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        std::vector<const char*> requiredExtensions{ window.get_required_extensions( ) };
#ifdef __APPLE__
        // VK_KHR_PORTABILITY_subset extension is necessary on macOS to allow MoltenVK to function properly.
        requiredExtensions.push_back( VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME );
        requiredExtensions.push_back( "VK_KHR_get_physical_device_properties2" );
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

        if ( validation_layers_ptr_ )
        {
            requiredExtensions.push_back( "VK_EXT_debug_utils" );
        }

        if ( not check_extension_support( requiredExtensions ) )
        {
            validation::throw_runtime_error( "One or more extensions are not supported!" );
        }

        createInfo.enabledExtensionCount   = static_cast<uint32_t>( requiredExtensions.size( ) );
        createInfo.ppEnabledExtensionNames = requiredExtensions.data( );

        // The last two members of the struct determine the global validation layers to enable.
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if ( validation_layers_ptr_ )
        {
            validation_layers_ptr_->populate_debug_info( debugCreateInfo );

            auto& layers                   = validation_layers_ptr_->get_validation_layers( );
            createInfo.enabledLayerCount   = static_cast<uint32_t>( layers.size( ) );
            createInfo.ppEnabledLayerNames = layers.data( );

            // We create a debug messenger specifically for the creation of the instance.
            // This will also provide validation in the destroy instance as well.
            createInfo.pNext = &debugCreateInfo;
        }
        else
        {
            // If we're not in debug mode, we can just set the count to 0.
            createInfo.enabledLayerCount = 0;
            createInfo.pNext             = nullptr;
        }

        // We've now specified everything Vulkan needs to create an instance, and we can
        // finally issue the vkCreateInstance.
        validation::throw_on_bad_result( vkCreateInstance( &createInfo, nullptr, &instance_ ),
                                         "Failed to create Vulkan instance!" );
    }


    void VulkanInstance::create_surface( const Window& window )
    {
        window.create_surface( &surface_, instance_ );
    }


    bool VulkanInstance::check_extension_support( const std::vector<const char*>& extensions )
    {
        // Retrieve a list of supported extensions
        uint32_t extensionCount{};
        vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );
        std::vector<VkExtensionProperties> availableExtensions( extensionCount );
        vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, availableExtensions.data( ) );

        // Check if all the required extensions are supported
        return validation::compare_support_containers<VkExtensionProperties>( extensions, availableExtensions,
                                                                              []( const VkExtensionProperties& extension )
                                                                                  {
                                                                                      return extension.extensionName;
                                                                                  } );
    }

}
