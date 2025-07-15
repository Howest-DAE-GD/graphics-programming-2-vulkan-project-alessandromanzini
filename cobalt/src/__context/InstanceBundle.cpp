#include <__context/InstanceBundle.h>

#include <assets/Window.h>
#include <validation/dispatch.h>
#include <validation/result.h>
#include <__context/ValidationLayers.h>
#include <__query/extension_support.h>


namespace cobalt
{
    InstanceBundle::InstanceBundle( VkApplicationInfo const& app_info, Window const& window,
                                    ValidationLayers const* validation )
    {
        create_instance( app_info, window.get_required_extensions( ), validation );
        window.create_surface( &surface_, instance_ );
    }


    InstanceBundle::~InstanceBundle( )
    {
        vkDestroySurfaceKHR( instance_, surface_, nullptr );
        vkDestroyInstance( instance_, nullptr );
    }


    VkInstance InstanceBundle::get_instance( ) const
    {
        return instance_;
    }


    VkSurfaceKHR InstanceBundle::get_surface( ) const
    {
        return surface_;
    }


    void InstanceBundle::create_instance( VkApplicationInfo const& app_info, std::vector<const char*> extensions,
                                          ValidationLayers const* validation )
    {
        bool const require_validation = ( validation != nullptr );

        // Tells the Vulkan driver which global extensions and validation layers we want to use.
        // Global here means that they apply to the entire program and not a specific device.
        VkInstanceCreateInfo create_info{};
        create_info.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;

#ifdef __APPLE__
        // VK_KHR_PORTABILITY_subset extension is necessary on macOS to allow MoltenVK to function properly.
        extensions.push_back( VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME );
        extensions.push_back( "VK_KHR_get_physical_device_properties2" );
        create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
        if ( require_validation )
        {
            extensions.push_back( "VK_EXT_debug_utils" );
        }

        // Add the required extensions to the create info.
        if ( not query::check_instance_extensions_support( extensions ) )
        {
            validation::throw_runtime_error( "One or more extensions are not supported!" );
        }
        create_info.enabledExtensionCount   = static_cast<uint32_t>( extensions.size( ) );
        create_info.ppEnabledExtensionNames = extensions.data( );

        // The last two members of the struct determine the global validation layers to enable.
        VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
        create_info.pNext = &debug_create_info;

        // If we do not require validation layers, we set the count to 0 and create the instance.
        if ( not require_validation )
        {
            create_info.enabledLayerCount = 0;
            create_info.pNext             = nullptr;
        }
        else
        {
            // We create a debug messenger specifically for the creation of the instance.
            // This will also provide validation in the destroy instance as well.
            validation->populate_messanger_debug_info( debug_create_info );
            validation->populate_create_info( create_info );
        }
        validation::throw_on_bad_result( vkCreateInstance( &create_info, nullptr, &instance_ ),
                                         "Failed to create Vulkan instance!" );
    }

}
