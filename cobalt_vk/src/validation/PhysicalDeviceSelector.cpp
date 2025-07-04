#include <validation/PhysicalDeviceSelector.h>

#include <query/queue_family.h>
#include <query/swapchain_support.h>

#include <set>


namespace cobalt_vk::validation
{
    PhysicalDeviceSelector::PhysicalDeviceSelector( VulkanInstance& instance, std::vector<const char*> requiredExtensions )
        : instance_ref_{ instance }
        , required_extensions_( std::move( requiredExtensions ) ) { }


    bool PhysicalDeviceSelector::select( VkPhysicalDevice device ) const
    {
        const query::QueueFamilyIndices indices{ query::find_queue_families( device, instance_ref_.get_surface(  ) ) };

        const bool extensionsSupported{ check_extension_support( device ) };

        // Swap chain support is sufficient for now if there is at least one supported image format and one supported
        // presentation mode.
        bool swapChainAdequate{ false };
        if ( extensionsSupported )
        {
            const query::SwapChainSupportDetails swapChainSupport{ query::query_swap_chain_support( device, instance_ref_.get_surface(  ) ) };
            swapChainAdequate = !swapChainSupport.formats.empty( ) && !swapChainSupport.presentModes.empty( );
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures( device, &supportedFeatures );

        return indices.is_suitable( ) && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }


    bool PhysicalDeviceSelector::check_extension_support( const VkPhysicalDevice device ) const
    {
        // Check if the device supports the required extensions
        uint32_t extensionCount{};
        vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount, nullptr );

        std::vector<VkExtensionProperties> availableExtensions( extensionCount );
        vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount, availableExtensions.data( ) );

        std::set<std::string> requiredExtensions( required_extensions_.begin( ), required_extensions_.end( ) );

        for ( const auto& [extensionName, specVersion] : availableExtensions )
        {
            requiredExtensions.erase( extensionName );
        }
        return requiredExtensions.empty( );
    }

}
