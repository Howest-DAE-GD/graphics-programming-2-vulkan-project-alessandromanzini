#include <__image/Image.h>

#include <log.h>
#include <__buffer/CommandBuffer.h>
#include <__buffer/CommandPool.h>
#include <__context/DeviceSet.h>
#include <__image/ImageLayoutTransition.h>
#include <__meta/expect_size.h>
#include <__query/device_queries.h>
#include <__validation/dispatch.h>
#include <__validation/result.h>


namespace cobalt
{
    Image::Image( DeviceSet const& device, ImageCreateInfo const& create_info )
        : device_ref_{ device }
        , format_{ create_info.format }
        , extent_{ create_info.extent }
    {
        init_image( create_info );
        init_view( ImageViewCreateInfo{ image_, create_info.format, create_info.aspect_flags } );
    }


    Image::Image( DeviceSet const& device, VkExtent2D const extent, ImageViewCreateInfo const& create_info )
        : device_ref_{ device }
        , format_{ create_info.format }
        , extent_{ extent }
        , image_{ create_info.image }
    {
        log::logerr<Image>( "Image", "image cannot be VK_NULL_HANDLE!", create_info.image == VK_NULL_HANDLE );
        init_view( create_info );
    }


    Image::~Image( )
    {
        // 1. Destroy the dependent image view
        if ( view_ptr_ )
        {
            view_ptr_.reset( );
        }

        // 2. Destroy the image and free its memory (if explicitly created)
        if ( image_ != VK_NULL_HANDLE && image_memory_ != VK_NULL_HANDLE )
        {
            vkFreeMemory( device_ref_.logical( ), image_memory_, nullptr );
            vkDestroyImage( device_ref_.logical( ), image_, nullptr );
            image_memory_ = VK_NULL_HANDLE;
            image_        = VK_NULL_HANDLE;
        }
    }


    Image::Image( Image&& other ) noexcept
        : device_ref_{ other.device_ref_ }
        , format_{ other.format_ }
        , extent_{ other.extent_ }
        , image_{ other.image_ }
        , image_memory_{ other.image_memory_ }
        , view_ptr_{ std::move( other.view_ptr_ ) }
    {
        meta::expect_size<Image, 56u>( );
        other.image_        = VK_NULL_HANDLE;
        other.image_memory_ = VK_NULL_HANDLE;
    }


    VkImage Image::handle( ) const
    {
        return image_;
    }


    ImageView& Image::view( ) const
    {
        return *view_ptr_;
    }


    VkFormat Image::format( ) const
    {
        return format_;
    }


    VkExtent2D Image::extent( ) const
    {
        return extent_;
    }


    void Image::transition_layout( ImageLayoutTransition const& transition_info, CommandPool& cmd_pool ) const
    {
        auto const& cmd_buffer = cmd_pool.acquire( VK_COMMAND_BUFFER_LEVEL_PRIMARY );

        cmd_buffer.reset( 0 );
        auto buffer_op = cmd_buffer.command_operator( 0 );

        VkImageMemoryBarrier2 const barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = transition_info.src_stage_mask,
            .srcAccessMask = transition_info.src_access_mask,
            .dstStageMask = transition_info.dst_stage_mask,
            .dstAccessMask = transition_info.dst_access_mask,
            .oldLayout = transition_info.old_layout,
            .newLayout = transition_info.new_layout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image_,
            .subresourceRange = {
                .aspectMask = view( ).aspect_flags( ),
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };

        buffer_op.insert_barrier( VkDependencyInfo{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier
        } );
        buffer_op.end_recording( );

        device_ref_.graphics_queue( ).submit_and_wait( sync::SubmitInfo{ device_ref_.device_index( ) }.execute( cmd_buffer ) );
        cmd_buffer.unlock( );
    }


    void Image::init_image( ImageCreateInfo const& create_info )
    {
        // Create texture image
        VkImageCreateInfo image_info{};
        image_info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_info.imageType     = VK_IMAGE_TYPE_2D;
        image_info.extent.width  = create_info.extent.width;
        image_info.extent.height = create_info.extent.height;
        image_info.extent.depth  = 1;
        image_info.mipLevels     = 1;
        image_info.arrayLayers   = 1;

        // Tell vulkan what kind of texels we are going to use
        image_info.format = create_info.format;

        // The tiling field can have one of two values:
        // 1. VK_IMAGE_TILING_LINEAR: Texels are laid out in row major order like our pixels array.
        // 2. VK_IMAGE_TILING_OPTIMAL: Texels are laid out in an implementation defined order for optimal access.
        image_info.tiling = create_info.tiling;

        // There are only two possible values for the initialLayout of an image:
        // 1. VK_IMAGE_LAYOUT_UNDEFINED: Not usable by the GPU and the very first transition will discard the texels.
        // 2. VK_IMAGE_LAYOUT_PREINITIALIZED: Not usable by the GPU, but the first transition will preserve the texels.
        image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        image_info.usage       = create_info.usage;
        image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_info.samples     = VK_SAMPLE_COUNT_1_BIT;
        image_info.flags       = 0; // Optional

        validation::throw_on_bad_result( vkCreateImage( device_ref_.logical( ), &image_info, nullptr, &image_ ),
                                         "Failed to create image!" );

        VkMemoryRequirements mem_requirements;
        vkGetImageMemoryRequirements( device_ref_.logical( ), image_, &mem_requirements );

        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize  = mem_requirements.size;
        alloc_info.memoryTypeIndex = query::find_memory_type( device_ref_.physical( ),
                                                              mem_requirements.memoryTypeBits,
                                                              create_info.properties );

        validation::throw_on_bad_result(
            vkAllocateMemory( device_ref_.logical( ), &alloc_info, nullptr, &image_memory_ ),
            "Failed to allocate image memory!" );

        vkBindImageMemory( device_ref_.logical( ), image_, image_memory_, 0 );
    }


    void Image::init_view( ImageViewCreateInfo const& create_info )
    {
        if ( create_info.aspect_flags != VK_IMAGE_ASPECT_NONE )
        {
            // Create an image view for the image
            view_ptr_ = std::make_unique<ImageView>( device_ref_, create_info );
        }
    }

}
