#include <log.h>
#include <__image/StbImageLoader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


namespace cobalt
{
    StbImageLoader::StbImageLoader( std::filesystem::path const& path, uint32_t const channels )
    {
        load_image( path, channels );
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


    uint32_t StbImageLoader::img_size( ) const noexcept
    {
        return img_width_ * img_height_ * img_channels_;
    }


    void StbImageLoader::load_image( std::filesystem::path const& path, uint32_t const desired_channels )
    {
        int found_channels;
        pixels_ptr_ = stbi_load( path.string( ).c_str( ),
                                 reinterpret_cast<int*>( &img_width_ ), reinterpret_cast<int*>( &img_height_ ),
                                 &found_channels,desired_channels );
        img_channels_ = desired_channels != 0 ? desired_channels : found_channels;
    }

}
