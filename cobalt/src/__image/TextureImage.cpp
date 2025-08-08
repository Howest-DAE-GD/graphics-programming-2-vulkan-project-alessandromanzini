#include <__image/TextureImage.h>

#include <log.h>
#include <__buffer/Buffer.h>
#include <__context/DeviceSet.h>
#include <__image/Image.h>
#include <__image/StbImageLoader.h>
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
    TextureImage::TextureImage( DeviceSet const& device, CommandPool& cmd_pool, TextureImageCreateInfo const& image_create_info,
                                TextureSamplerCreateInfo const& sampler_create_info )
        : device_ref_{ device }
    {
        // 1. Create the image from the provided path and format.
        create_image( image_create_info, cmd_pool );

        // 2. Create the sampler for the image.
        create_sampler( sampler_create_info );
    }


    TextureImage::~TextureImage( ) noexcept
    {
        // 1. Destroy the sampler
        vkDestroySampler( device_ref_.logical( ), texture_sampler_, nullptr );

        // 2. Destroy the image
        texture_image_ptr_.reset( );
    }


    Image const& TextureImage::image( ) const
    {
        return *texture_image_ptr_;
    }


    VkSampler TextureImage::sampler( ) const
    {
        return texture_sampler_;
    }


    void TextureImage::create_image( TextureImageCreateInfo const& create_info, CommandPool& cmd_pool )
    {
        StbImageLoader const loader{ create_info.path_to_img, to_channel_count( create_info.image_format ) };

        Buffer staging_buffer = buffer::make_staging_buffer( device_ref_, loader.img_size( ) );
        staging_buffer.map_memory( );
        memcpy( staging_buffer.data( ), loader.pixels( ), staging_buffer.memory_size( ) );
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


    void TextureImage::create_sampler( TextureSamplerCreateInfo const& sampler_info )
    {
        VkPhysicalDeviceProperties2 properties{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2
        };
        vkGetPhysicalDeviceProperties2( device_ref_.physical( ), &properties );

        VkSamplerCreateInfo const create_info{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,

            .magFilter = sampler_info.filter,
            .minFilter = sampler_info.filter,

            .addressModeU = sampler_info.address_mode,
            .addressModeV = sampler_info.address_mode,
            .addressModeW = sampler_info.address_mode,

            .anisotropyEnable = device_ref_.has_feature( DeviceFeatureFlags::ANISOTROPIC_SAMPLING ),
            .maxAnisotropy = properties.properties.limits.maxSamplerAnisotropy,

            .borderColor = sampler_info.border_color,

            .unnormalizedCoordinates = sampler_info.unnormalized_coordinates,

            .compareEnable = sampler_info.compare_enable,
            .compareOp = sampler_info.compare_op,

            .mipmapMode = sampler_info.mipmap_mode,
            .mipLodBias = 0.f,
            .minLod = 0.f,
            .maxLod = 0.f
        };

        validation::throw_on_bad_result(
            vkCreateSampler( device_ref_.logical( ), &create_info, nullptr, &texture_sampler_ ),
            "failed to create texture sampler!" );
    }
}
