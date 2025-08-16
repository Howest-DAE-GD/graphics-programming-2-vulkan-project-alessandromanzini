#include <__image/CubeMapImage.h>

#include <__buffer/Buffer.h>
#include <__context/DeviceSet.h>
#include <__image/StbImageLoader.h>


namespace cobalt
{
    CubeMapImage::CubeMapImage( DeviceSet const& device, CommandPool& cmd_pool, CubeMapImageCreateInfo const& create_info )
    {
        load_hdr_image( device, cmd_pool, create_info );

        cubemap_image_ptr_ = std::make_unique<Image>(
            device,
            ImageCreateInfo{
                .extent = { hdr_image_ptr_->extent( ).width / 4u, hdr_image_ptr_->extent( ).height / 2u },
                .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                .tiling = VK_IMAGE_TILING_OPTIMAL,
                .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                .properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                .create_flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
                .aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT,
                .layers = 6,
                .view_type = VK_IMAGE_VIEW_TYPE_CUBE,
            }
        );
    }


    Image& CubeMapImage::hdr_image( ) const
    {
        return *hdr_image_ptr_;
    }


    Image& CubeMapImage::cube_image( ) const
    {
        return *cubemap_image_ptr_;
    }


    std::array<ImageView, 6> CubeMapImage::generate_cubic_views( DeviceSet const& device ) const
    {
        ImageViewCreateInfo const create_info{
            .image = cubemap_image_ptr_->handle( ),
            .format = cubemap_image_ptr_->format( ),
            .aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT,
            .view_type = VK_IMAGE_VIEW_TYPE_2D,
        };
        return {
            ImageView{ device, create_info.clone( 0u ) },
            ImageView{ device, create_info.clone( 1u ) },
            ImageView{ device, create_info.clone( 2u ) },
            ImageView{ device, create_info.clone( 3u ) },
            ImageView{ device, create_info.clone( 4u ) },
            ImageView{ device, create_info.clone( 5u ) },
        };
    }


    void CubeMapImage::load_hdr_image( DeviceSet const& device, CommandPool& cmd_pool, CubeMapImageCreateInfo const& create_info )
    {
        StbImageLoader const loader{ create_info.path_to_img, image::to_channel_count( VK_FORMAT_R32G32B32A32_SFLOAT ), true };

        hdr_image_ptr_ = std::make_unique<Image>(
            device,
            ImageCreateInfo{
                .extent = { loader.img_width( ), loader.img_height( ) },
                .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                .tiling = VK_IMAGE_TILING_OPTIMAL,
                .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                .properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                .aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT,
                .view_type = VK_IMAGE_VIEW_TYPE_2D,
            } );

        Buffer staging_buffer = buffer::make_staging_buffer( device, loader.img_size( ) );
        staging_buffer.map_memory( );
        staging_buffer.write( loader.pixels( ), staging_buffer.memory_size( ) );
        staging_buffer.unmap_memory( );

        hdr_image_ptr_->transition_layout( { VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL }, cmd_pool );
        staging_buffer.copy_to( *hdr_image_ptr_, cmd_pool );
        hdr_image_ptr_->transition_layout( { VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }, cmd_pool );
    }

}
