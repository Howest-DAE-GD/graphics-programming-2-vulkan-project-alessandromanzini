#include <validation/PhysicalDeviceSelector.h>

#include <__context/InstanceBundle.h>
#include <__query/queue_family.h>
#include <__query/swapchain_support.h>

#include <set>


namespace cobalt::validation
{
    PhysicalDeviceSelector::PhysicalDeviceSelector( InstanceBundle const& instance,
                                                    std::vector<char const*> const& required_extensions )
        : instance_ref_{ instance }
        , required_extensions_( required_extensions ) { }


    bool PhysicalDeviceSelector::select( VkPhysicalDevice device ) const
    {
        query::QueueFamilyIndices const indices{ query::find_queue_families( device, instance_ref_ ) };

        bool const extensions_supported{ check_extension_support( device ) };

        // Swap chain support is sufficient for now if there is at least one supported image format and one supported
        // presentation mode.
        bool swap_chain_adequate{ false };
        if ( extensions_supported )
        {
            query::SwapChainSupportDetails const swap_chain_support{ query::check_swap_chain_support( device, instance_ref_ ) };
            swap_chain_adequate = !swap_chain_support.formats.empty( ) && !swap_chain_support.present_modes.empty( );
        }

        VkPhysicalDeviceFeatures supported_features;
        vkGetPhysicalDeviceFeatures( device, &supported_features );

        return indices.is_suitable( ) && extensions_supported && swap_chain_adequate && supported_features.samplerAnisotropy;
    }


    bool PhysicalDeviceSelector::check_extension_support( VkPhysicalDevice const device ) const
    {
        // Check if the device supports the required extensions
        uint32_t extension_count{};
        vkEnumerateDeviceExtensionProperties( device, nullptr, &extension_count, nullptr );

        std::vector<VkExtensionProperties> available_extensions( extension_count );
        vkEnumerateDeviceExtensionProperties( device, nullptr, &extension_count, available_extensions.data( ) );

        std::set<std::string> required_extensions( required_extensions_.begin( ), required_extensions_.end( ) );

        for ( auto const& [extensionName, specVersion] : available_extensions )
        {
            required_extensions.erase( extensionName );
        }
        return required_extensions.empty( );
    }

}
