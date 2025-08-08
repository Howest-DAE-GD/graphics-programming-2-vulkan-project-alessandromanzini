#include <__swapchain/Swapchain.h>

#include <__builder/VkSwapchainBuilder.h>
#include <__context/VkContext.h>
#include <__init/InitWizard.h>
#include <__query/device_queries.h>
#include <__validation/result.h>


namespace cobalt
{
    Swapchain::Swapchain( SwapchainWizard wizard )
        : context_ref_{ wizard.get<decltype(context_ref_)>( ) }
        , window_ref_{ wizard.get<decltype(window_ref_)>( ) }
        , create_info_{ wizard.get<decltype(create_info_)>( ) }
    {
        init_swapchain( );
        window_ref_.on_framebuffer_resize.bind( this, &Swapchain::recreate_swapchain );
    }


    Swapchain::~Swapchain( )
    {
        destroy_swapchain( );
        window_ref_.on_framebuffer_resize.unbind( this );
    }


    VkSwapchainKHR Swapchain::handle( ) const
    {
        return swapchain_;
    }


    VkSwapchainKHR const* Swapchain::handle_ptr( ) const
    {
        return &swapchain_;
    }


    VkFormat Swapchain::image_format( ) const
    {
        return image_format_;
    }


    VkExtent2D Swapchain::extent( ) const
    {
        return extent_;
    }


    std::size_t Swapchain::image_count( ) const
    {
        return images_.size( );
    }


    Image const& Swapchain::image_at( size_t const index ) const
    {
        assert( index < images_.size( ) && "index out of bounds!" );
        return images_[index];
    }


    Image const& Swapchain::depth_image( ) const
    {
        return *depth_image_ptr_;
    }


    uint32_t Swapchain::acquire_next_image( VkSemaphore const semaphore, uint64_t const timeout )
    {
        uint32_t image_index{};

        // The first two parameters of vkAcquireNextImageKHR are the logical device and the swap chain from which we wish
        // to acquire an image.
        auto const result = vkAcquireNextImageKHR( context_ref_.device( ).logical( ), swapchain_,
                                                   timeout, semaphore, VK_NULL_HANDLE, &image_index );

        // If viewport changes, recreate the swapchain.
        // - VK_ERROR_OUT_OF_DATE_KHR: The swapchain has become incompatible with the surface and can no longer be used for
        // rendering. Usually happens after a window resize.
        // - VK_SUBOPTIMAL_KHR: The swapchain can still be used to successfully present to the surface, but the surface
        // properties are no longer matched exactly.
        switch ( result )
        {
            case VK_SUCCESS:
            case VK_SUBOPTIMAL_KHR:
                return image_index;

            case VK_ERROR_OUT_OF_DATE_KHR:
                recreate_swapchain( window_ref_.extent( ) );
                return UINT32_MAX;

            default:
                validation::throw_on_bad_result( result, "Failed to acquire next swapchain image!" );
                return UINT32_MAX;
        }
    }


    void Swapchain::init_swapchain( )
    {
        // 1. Setup the swapchain builder
        VkSwapchainBuilder builder{ context_ref_ };
        builder.set_extent( window_ref_.extent( ) )
               .set_image_buffering_aim( create_info_.image_count )
               .set_preferred_present_mode( create_info_.present_mode )
               .set_preferred_surface_format( create_info_.surface_format );

        // 2. Populate the create info structure
        VkSwapchainCreateInfoKHR create_info{};
        auto details = builder.populate_create_info( create_info );
        validation::throw_on_bad_result(
            vkCreateSwapchainKHR( context_ref_.device( ).logical( ), &create_info, nullptr, &swapchain_ ),
            "Failed to create swap chain!" );

        // 3. Store the swap chain image format and extent for later use
        image_format_ = details.format_khr.format;
        extent_       = details.extent;

        // 4. Query image handles and create the views
        {
            vkGetSwapchainImagesKHR( context_ref_.device( ).logical( ), swapchain_, &details.image_count, nullptr );
            std::vector<VkImage> swc_imgs( details.image_count );
            vkGetSwapchainImagesKHR( context_ref_.device( ).logical( ), swapchain_, &details.image_count, swc_imgs.data( ) );

            for ( auto const& image : swc_imgs )
            {
                images_.emplace_back(
                    context_ref_.device( ), extent_,
                    ImageViewCreateInfo{ image, image_format_, VK_IMAGE_ASPECT_COLOR_BIT } );
            }
        }

        // 5. Create the depth image
        if ( create_info_.create_depth_image )
        {
            VkFormat const depth_format = query::find_depth_format( context_ref_.device( ).physical( ) );
            depth_image_ptr_            = std::make_unique<Image>( context_ref_.device( ),
                                                        ImageCreateInfo{
                                                            .extent = extent_,
                                                            .format = depth_format,
                                                            .tiling = VK_IMAGE_TILING_OPTIMAL,
                                                            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                            .properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                            .aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT
                                                        } );
        }
    }


    void Swapchain::destroy_swapchain( )
    {
        // 1. Destroy the depth image.
        depth_image_ptr_.reset( );

        // 2. Destroy the swapchain images.
        images_.clear( );

        // 3. Destroy the swapchain.
        vkDestroySwapchainKHR( context_ref_.device( ).logical( ), swapchain_, nullptr );
    }


    void Swapchain::recreate_swapchain( VkExtent2D const extent )
    {
        // In case the window gets minimized, or the extent is the same as the current swapchain extent, we don't recreate
        // the swapchain.
        if ( window_ref_.is_minimized( ) || ( extent.width == extent_.width && extent.height == extent_.height ) )
        {
            return;
        }

        // It is possible to create a new swapchain while drawing commands on an image from the old swap chain are still
        // in-flight. You need to pass the previous swap chain to the oldSwapChain field in the VkSwapchainCreateInfoKHR
        // struct and destroy the old swap chain as soon as you've finished using it.
        context_ref_.device( ).wait_idle( );
        destroy_swapchain( );
        init_swapchain( );
    }

}
