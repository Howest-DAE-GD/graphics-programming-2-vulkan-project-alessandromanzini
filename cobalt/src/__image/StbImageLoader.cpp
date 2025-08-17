#include <log.h>
#include <__image/StbImageLoader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


namespace cobalt
{
    // +---------------------------+
    // | UTILITY                   |
    // +---------------------------+
    uint32_t image::to_channel_count( VkFormat const format )
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
            case VK_FORMAT_R32G32B32_SFLOAT:
                return 3;

            case VK_FORMAT_R8G8B8A8_UNORM:
            case VK_FORMAT_R8G8B8A8_SRGB:
            case VK_FORMAT_B8G8R8A8_UNORM:
            case VK_FORMAT_B8G8R8A8_SRGB:
            case VK_FORMAT_R32G32B32A32_SFLOAT:
                return 4;

            default:
                log::logerr( "to_channel_count", std::format( "unsupported format: {}", static_cast<int>( format ) ) );
                return 0;
        }
    }


    bool image::is_float_texel( VkFormat const format )
    {
        switch ( format )
        {
            case VK_FORMAT_R16G16B16A16_SFLOAT:
            case VK_FORMAT_R32G32B32A32_SFLOAT:
            case VK_FORMAT_R16G16B16_SFLOAT:
            case VK_FORMAT_R16G16_SFLOAT:
            case VK_FORMAT_D32_SFLOAT:
            case VK_FORMAT_R32_SFLOAT:
            case VK_FORMAT_R16_SFLOAT:
                return true;

            default:
                return false;
        }
    }


    // +---------------------------+
    // | STB IMAGE LOADER          |
    // +---------------------------+
    StbImageLoader::StbImageLoader( std::filesystem::path const& path, uint32_t const channels, bool const f_load )
    {
        if ( f_load )
        {
            load_image_float( path, channels );
        }
        else
        {
            load_image( path, channels );
        }

        if ( not pixels_ptr_ )
        {
            log::logerr<StbImageLoader>( "StbImageLoader", std::format( "Failed to load texture image: {}", path.string( ) ) );
        }
    }


    StbImageLoader::~StbImageLoader( ) noexcept
    {
        stbi_image_free( pixels_ptr_ );
    }


    void* StbImageLoader::pixels( ) const noexcept
    {
        return pixels_ptr_;
    }


    uint32_t StbImageLoader::img_width( ) const noexcept
    {
        return img_width_;
    }


    uint32_t StbImageLoader::img_height( ) const noexcept
    {
        return img_height_;
    }


    uint64_t StbImageLoader::img_size( ) const noexcept
    {
        return img_width_ * img_height_ * img_channels_ * img_channel_size_;
    }


    void StbImageLoader::load_image( std::filesystem::path const& path, uint32_t const desired_channels )
    {
        load_image_impl( reinterpret_cast<load_fn_t>( stbi_load ), path, desired_channels );
        img_channel_size_ = sizeof( stbi_uc );
    }


    void StbImageLoader::load_image_float( std::filesystem::path const& path, uint32_t const desired_channels )
    {
        load_image_impl( reinterpret_cast<load_fn_t>( stbi_loadf ), path, desired_channels );
        img_channel_size_ = sizeof( float );
    }


    void StbImageLoader::load_image_impl( load_fn_t const fn, std::filesystem::path const& path, uint32_t const desired_channels )
    {
        int found_channels;
        pixels_ptr_ = fn( path.string( ).c_str( ), reinterpret_cast<int*>( &img_width_ ), reinterpret_cast<int*>( &img_height_ ),
                          &found_channels, desired_channels );
        img_channels_ = desired_channels != 0 ? desired_channels : found_channels;
    }

}
