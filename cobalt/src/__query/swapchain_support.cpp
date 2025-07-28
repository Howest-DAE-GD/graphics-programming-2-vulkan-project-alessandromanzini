#include <__query/swapchain_support.h>

#include <__context/InstanceBundle.h>


namespace cobalt::query
{
    // +---------------------------+
    // | SWAPCHAIN SUPPORT DETAILS |
    // +---------------------------+
    bool SwapchainSupportDetails::is_adequate( ) const
    {
        return !formats.empty( ) && !present_modes.empty( );
    }


    SwapchainSupportDetails::operator bool( ) const
    {
        return is_adequate( );
    }


    // +---------------------------+
    // | QUERIES                   |
    // +---------------------------+
    SwapchainSupportDetails check_swapchain_support( VkPhysicalDevice const physical_device, InstanceBundle const& instance )
    {
        VkSurfaceKHR const surface = instance.surface( );

        // There are basically three kinds of properties we need to check:
        // 1. Basic surface capabilities ( min/max number of images in swap chain, min/max width and height of images )
        // 2. Surface formats( pixel format, color space )
        // 3. Available presentation modes
        SwapchainSupportDetails details{};

        // 1.
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR( physical_device, surface, &details.capabilities );

        // 2.
        uint32_t format_count{};
        vkGetPhysicalDeviceSurfaceFormatsKHR( physical_device, surface, &format_count, nullptr );

        if ( format_count != 0 )
        {
            details.formats.resize( format_count );
            vkGetPhysicalDeviceSurfaceFormatsKHR( physical_device, surface, &format_count, details.formats.data( ) );
        }

        // 3.
        uint32_t present_mode_count{};
        vkGetPhysicalDeviceSurfacePresentModesKHR( physical_device, surface, &present_mode_count, nullptr );

        if ( present_mode_count != 0 )
        {
            details.present_modes.resize( present_mode_count );
            vkGetPhysicalDeviceSurfacePresentModesKHR( physical_device, surface, &present_mode_count,
                                                       details.present_modes.data( ) );
        }

        return details;
    }

}
