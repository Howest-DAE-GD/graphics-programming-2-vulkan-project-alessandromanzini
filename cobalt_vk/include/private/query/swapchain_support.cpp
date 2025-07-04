#include <query/swapchain_support.h>


namespace cobalt_vk::query
{
    SwapChainSupportDetails query_swap_chain_support( const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface )
    {
        // There are basically three kinds of properties we need to check:
        // 1. Basic surface capabilities ( min/max number of images in swap chain, min/max width and height of images )
        // 2. Surface formats( pixel format, color space )
        // 3. Available presentation modes
        SwapChainSupportDetails details{};

        // 1.
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR( physicalDevice, surface, &details.capabilities );

        // 2.
        uint32_t formatCount{};
        vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice, surface, &formatCount, nullptr );

        if ( formatCount != 0 )
        {
            details.formats.resize( formatCount );
            vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice, surface, &formatCount, details.formats.data( ) );
        }

        // 3.
        uint32_t presentModeCount{};
        vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice, surface, &presentModeCount, nullptr );

        if ( presentModeCount != 0 )
        {
            details.presentModes.resize( presentModeCount );
            vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice, surface, &presentModeCount,
                                                       details.presentModes.data( ) );
        }

        return details;
    }

}
