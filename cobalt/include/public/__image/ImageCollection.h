#ifndef IMAGECOLLECTION_H
#define IMAGECOLLECTION_H

#include <__memory/Resource.h>

#include <__context/DeviceSet.h>
#include <__image/Image.h>

#include <array>
#include <cassert>


namespace cobalt
{
    template <size_t N>
    class ImageCollection final : public memory::Resource
    {
    public:
        ImageCollection( DeviceSet const&, ImageCreateInfo const& );
        ~ImageCollection( ) noexcept override = default;

        ImageCollection( const ImageCollection& )                = delete;
        ImageCollection( ImageCollection&& ) noexcept            = delete;
        ImageCollection& operator=( const ImageCollection& )     = delete;
        ImageCollection& operator=( ImageCollection&& ) noexcept = delete;

        [[nodiscard]] Image& image_at( size_t index )
        {
            assert( index < N && "Image::image_at: index out of range!" );
            if ( index >= N )
            {
                throw std::out_of_range( "Index out of range in ImageCollection" );
            }
            return *images_[index];
        }

    private:
        std::array<std::unique_ptr<Image>, N> images_{};

    };


    template <size_t N>
    ImageCollection<N>::ImageCollection( DeviceSet const& device, ImageCreateInfo const& create_info )
    {
        for ( auto& image : images_ )
        {
            image = std::make_unique<Image>( device, create_info );
        }
    }

}


#endif //!IMAGECOLLECTION_H
