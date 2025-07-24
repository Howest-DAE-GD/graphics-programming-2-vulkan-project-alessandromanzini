#include <VulkanDeviceQueries.h>
#include <__buffer/Framebuffer.h>
#include <__swapchain/Swapchain.h>

#include <__builder/VkSwapchainBuilder.h>
#include <__context/VkContext.h>
#include <__validation/result.h>


namespace cobalt
{
    Swapchain::Swapchain( VkContext const& context, Window& window, SwapchainCreateInfo const& info )
        : context_ref_{ context }
        , window_{ window }
        , create_info_{ info }
    {
        init_swapchain( );
        window.on_framebuffer_resize.bind( this, &Swapchain::recreate_swapchain );
    }


    Swapchain::~Swapchain( )
    {
        destroy_swapchain( );
        window_.on_framebuffer_resize.unbind( this );
    }


    VkSwapchainKHR Swapchain::handle( ) const
    {
        return swapchain_;
    }


    VkSwapchainKHR* Swapchain::phandle( )
    {
        return &swapchain_;
    }


    VkFormat Swapchain::image_format( ) const
    {
        return image_format_;
    }


    VkExtent2D Swapchain::extent( ) const
    {
        return swapchain_extent_;
    }


    std::vector<Image> const& Swapchain::images( ) const
    {
        return images_;
    }


    void Swapchain::init_swapchain( )
    {
        // 1. Setup the swapchain builder
        VkSwapchainBuilder builder{ context_ref_ };
        builder.set_extent( window_.extent( ) )
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
        image_format_     = details.format_khr.format;
        swapchain_extent_ = details.extent;

        // 4. Query image handles and create the views
        {
            vkGetSwapchainImagesKHR( context_ref_.device( ).logical( ), swapchain_, &details.image_count, nullptr );
            std::vector<VkImage> swc_imgs( details.image_count );
            vkGetSwapchainImagesKHR( context_ref_.device( ).logical( ), swapchain_, &details.image_count, swc_imgs.data( ) );

            for ( auto const& image : swc_imgs )
            {
                images_.emplace_back( context_ref_.device( ),
                                      ImageViewCreateInfo{ image, image_format_, VK_IMAGE_ASPECT_COLOR_BIT } );
            }
        }

        // 5. Create the depth image
        {
            // VkFormat const depth_format = query::find_depth_format( context_ref_.device( ).physical( ) );
            // depth_image_ptr_ = std::make_unique<Image>( context_ref_.device( ), ImageCreateInfo{
            //     .extent = swapchain_extent_,
            //     .format = depth_format,
            //     .tiling = VK_IMAGE_TILING_OPTIMAL,
            //     .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            //     .properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            //     .aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT
            // } );
            // vk_transition_image_layout( depth_image_, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
            //                             VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL );
        }

        // 6. Create the frame buffers
        {
            // for ( auto const& image : images_ )
            // {
            //     framebuffers_.emplace_back( context_ref_.device( ), FramebufferCreateInfo{
            //         .usage_type = FramebufferUsageType::RENDER_PASS,
            //         .render_pass = create_info_.render_pass,
            //         .extent = swapchain_extent_,
            //         .attachments = { image.view( ).handle( ), depth_image_ptr_->image_view( ).handle( ) }
            //     } );
            // }
        }
    }


    void Swapchain::destroy_swapchain( )
    {
        // 1. Destroy the frame buffers.
        // framebuffers_.clear( );

        // 2. Destroy the depth image.
        // depth_image_ptr_.reset( );

        // 3. Destroy the swapchain images.
        images_.clear( );

        // 4. Destroy the swapchain.
        vkDestroySwapchainKHR( context_ref_.device( ).logical( ), swapchain_, nullptr );
    }


    void Swapchain::recreate_swapchain( VkExtent2D const )
    {
        // In case the window gets minimized, we wait until it gets restored.
        // auto [width, height] = window_->extent( );
        // while ( width == 0 || height == 0 )
        // {
        //     auto [tempWidth, tempHeight] = window_->extent( );
        //     width                        = tempWidth;
        //     height                       = tempHeight;
        //     glfwWaitEvents( );
        // }

        // It is possible to create a new swap chain while drawing commands on an image from the old swap chain are still
        // in-flight. You need to pass the previous swap chain to the oldSwapChain field in the VkSwapchainCreateInfoKHR
        // struct and destroy the old swap chain as soon as you've finished using it.
        context_ref_.device( ).wait_idle( );
        destroy_swapchain( );
        init_swapchain( );
    }

}
