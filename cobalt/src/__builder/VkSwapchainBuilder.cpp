#include <__builder/VkSwapchainBuilder.h>

#include <__context/VkContext.h>
#include <__query/queue_family.h>
#include <__query/swapchain_support.h>

#include <algorithm>


namespace cobalt
{
    VkSwapchainBuilder::VkSwapchainBuilder( VkContext const& context )
        : VkBuilder{ context.device( ).logical( ), &vkCreateSwapchainKHR }
        , context_ref_{ context }
        , support_details_{ query::check_swapchain_support( context.device( ).physical( ), context.instance( ) ) } { }


    VkSwapchainBuilder& VkSwapchainBuilder::set_image_buffering_aim( uint32_t const aim )
    {
        image_buffering_aim_ = aim;
        return *this;
    }


    VkSwapchainBuilder& VkSwapchainBuilder::set_extent( VkExtent2D const& extent )
    {
        extent_ = extent;
        return *this;
    }


    VkSwapchainBuilder& VkSwapchainBuilder::set_extent( std::pair<int, int> const& extent )
    {
        return set_extent( VkExtent2D{ static_cast<uint32_t>( extent.first ), static_cast<uint32_t>( extent.second ) } );
    }


    VkSwapchainPopulateDetail VkSwapchainBuilder::populate_create_info( VkSwapchainCreateInfoKHR& create_info ) const
    {
        VkSwapchainPopulateDetail const detail{
            .image_count = choose_image_buffering_aim( ),
            .extent = choose_extent( support_details_.capabilities ),
            .present_mode_khr = choose_present_mode( support_details_.present_modes ),
            .format_khr = choose_surface_format( support_details_.formats )
        };

        create_info.sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create_info.surface = context_ref_.instance( ).surface( );

        create_info.minImageCount    = detail.image_count;
        create_info.imageExtent      = detail.extent;
        create_info.imageFormat      = detail.format_khr.format;
        create_info.imageColorSpace  = detail.format_khr.colorSpace;
        create_info.imageArrayLayers = 1;
        create_info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        auto const [graphics_family, present_family] =
                query::find_queue_families( context_ref_.device( ).physical( ), context_ref_.instance( ) );
        uint32_t const queue_family_indices[] = { graphics_family.value( ), present_family.value( ) };

        // The imageArrayLayers specifies the amount of layers each image consists of. This is always 1 unless you are
        // developing a stereoscopic 3D application.
        // We need to specify how to handle swap chain images that will be used across multiple queue families.
        // There are two ways to handle images that are accessed from multiple queues:
        // 1. VK_SHARING_MODE_EXCLUSIVE: An image is owned by one queue family at a time and ownership must be explicitly
        // transferred before using it in another queue family. This option offers the best performance.
        // 2. VK_SHARING_MODE_CONCURRENT: Images can be used across multiple queue families without explicit ownership
        // transfers.
        if ( graphics_family != present_family )
        {
            create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
            create_info.queueFamilyIndexCount = 2;
            create_info.pQueueFamilyIndices   = queue_family_indices;
        }
        else
        {
            create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }
        create_info.preTransform   = support_details_.capabilities.currentTransform;
        create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        create_info.presentMode    = detail.present_mode_khr;
        create_info.clipped        = VK_TRUE;
        create_info.oldSwapchain   = VK_NULL_HANDLE;

        return detail;
    }


    query::SwapchainSupportDetails const& VkSwapchainBuilder::support_details( ) const
    {
        return support_details_;
    }


    VkSwapchainBuilder& VkSwapchainBuilder::set_preferred_present_mode( VkPresentModeKHR const present_mode )
    {
        preferred_present_mode_ = present_mode;
        return *this;
    }


    VkSwapchainBuilder& VkSwapchainBuilder::set_preferred_surface_format( VkSurfaceFormatKHR const surface_format )
    {
        preferred_surface_format_ = surface_format;
        return *this;
    }


    uint32_t VkSwapchainBuilder::choose_image_buffering_aim( ) const
    {
        auto const& capabilities = support_details_.capabilities;

        // Simply sticking to the minimum image count means that we may sometimes have to wait on the driver to complete
        // internal operations before we can acquire another image to render to.
        // We aim for triple buffering.
        uint32_t image_count = std::clamp( image_buffering_aim_, capabilities.minImageCount, capabilities.maxImageCount );

        // If maxImageCount doesn't have a cap, it will be set to 0, we adjust it to the buffering aim.
        if ( image_count == 0 )
        {
            image_count = image_buffering_aim_;
        }

        // We must also make sure to not exceed the maximum number of images while doing this, where 0 is a special value
        // that means that there is no maximum specified by the standard.
        if ( capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount )
        {
            image_count = capabilities.maxImageCount;
        }
        return image_count;
    }


    VkSurfaceFormatKHR VkSwapchainBuilder::choose_surface_format( std::vector<VkSurfaceFormatKHR> const& available_formats ) const
    {
        // The format member specifies the color channels and types. For the color space we'll use SRGB if it is available,
        // because it results in more accurate perceived colors.
        // https://stackoverflow.com/questions/12524623/what-are-the-practical-differences-when-working-with-colors-in-a-linear-vs-a-no
        for ( auto const& available_format : available_formats )
        {
            if ( available_format.format == preferred_surface_format_.format &&
                 available_format.colorSpace == preferred_surface_format_.colorSpace )
            {
                return available_format;
            }
        }
        // If that fails then we can settle with the first format that is specified.
        return available_formats[0];
    }


    VkPresentModeKHR VkSwapchainBuilder::choose_present_mode( std::vector<VkPresentModeKHR> const& available_present_modes ) const
    {
        // In the VK_PRESENT_MODE_MAILBOX_KHR present mode, the swap chain is a queue where the display takes an image from
        // the front of the queue when the display is refreshed and the program inserts rendered images at the back of the
        // queue. Instead of blocking the application when the queue is full, the images that are already queued are simply
        // replaced with the newer ones.
        // VK_PRESENT_MODE_MAILBOX_KHR is a very nice trade-off if energy usage is not a concern. It allows us to avoid
        // tearing while still maintaining a fairly low latency by rendering new images that are as up-to-date as possible
        // right until the vertical blank.
        for ( auto const& available_present_mode : available_present_modes )
        {
            if ( available_present_mode == preferred_present_mode_ )
            {
                return available_present_mode;
            }
        }
        // VK_PRESENT_MODE_FIFO_KHR mode is guaranteed to be available.
        return VK_PRESENT_MODE_FIFO_KHR;
    }


    VkExtent2D VkSwapchainBuilder::choose_extent( VkSurfaceCapabilitiesKHR const& capabilities ) const
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
        return {
            .width = std::clamp( extent_.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width ),
            .height = std::clamp( extent_.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height )
        };
    }

}
