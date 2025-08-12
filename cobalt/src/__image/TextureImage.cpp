#include <__image/TextureImage.h>

#include <log.h>
#include <__buffer/Buffer.h>
#include <__context/DeviceSet.h>
#include <__image/Image.h>
#include <__image/StbImageLoader.h>
#include <__meta/expect_size.h>
#include <__validation/result.h>


namespace cobalt
{
    // +---------------------------+
    // | FORMAT UTILITY            |
    // +---------------------------+
    uint32_t to_channel_count( VkFormat const format )
    {
        switch ( format )
        {
            case VK_FORMAT_R8_UNORM:
            case VK_FORMAT_R8_SRGB:
                return 1;

            case VK_FORMAT_R8G8_UNORM:
            case VK_FORMAT_R8G8_SRGB:
                return 2;

            case VK_FORMAT_R8G8B8_UNORM:
            case VK_FORMAT_R8G8B8_SRGB:
            case VK_FORMAT_B8G8R8_UNORM:
            case VK_FORMAT_B8G8R8_SRGB:
                return 3;

            case VK_FORMAT_R8G8B8A8_UNORM:
            case VK_FORMAT_R8G8B8A8_SRGB:
            case VK_FORMAT_B8G8R8A8_UNORM:
            case VK_FORMAT_B8G8R8A8_SRGB:
                return 4;

            default:
                log::logerr( "to_channel_count", std::format( "unsupported format: {}", static_cast<int>( format ) ) );
                return 0;
        }
    }


    // +---------------------------+
    // | TEXTURE IMAGE             |
    // +---------------------------+
    TextureImage::TextureImage( DeviceSet const& device, CommandPool& cmd_pool, TextureImageCreateInfo const& create_info )
        : device_ref_{ device }
    {
        StbImageLoader const loader{ create_info.path_to_img, to_channel_count( create_info.image_format ) };

        Buffer staging_buffer = buffer::make_staging_buffer( device_ref_, loader.img_size( ) );
        staging_buffer.map_memory( );
        staging_buffer.write( loader.pixels( ), staging_buffer.memory_size( ) );
        staging_buffer.unmap_memory( );

        texture_image_ptr_ = std::make_unique<Image>(
            device_ref_,
            ImageCreateInfo{
                .extent = { loader.img_width( ), loader.img_height( ) },
                .format = create_info.image_format,
                .tiling = VK_IMAGE_TILING_OPTIMAL,
                .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                .properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                .aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT
            } );
        texture_image_ptr_->transition_layout( { VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL }, cmd_pool );
        staging_buffer.copy_to( *texture_image_ptr_, cmd_pool );
        texture_image_ptr_->transition_layout(
            { VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }, cmd_pool );
    }


    TextureImage::~TextureImage( ) noexcept
    {
        texture_image_ptr_.reset( );
    }


    TextureImage::TextureImage( TextureImage&& other ) noexcept
        : device_ref_{ other.device_ref_ }
        , texture_image_ptr_{ std::move( other.texture_image_ptr_ ) }
    {
        meta::expect_size<TextureImage, 24>( );
    }


    Image const& TextureImage::image( ) const
    {
        return *texture_image_ptr_;
    }

}
