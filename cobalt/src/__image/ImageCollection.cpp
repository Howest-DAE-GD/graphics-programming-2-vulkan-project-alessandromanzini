#include <__image/ImageCollection.h>

#include <__context/DeviceSet.h>

#include <cassert>


namespace cobalt
{
    ImageCollection::ImageCollection( DeviceSet const& device, ImageCreateInfo const& create_info, size_t const n )
        : image_format_{ create_info.format }
        , image_extent_{ create_info.extent }
    {
        for ( size_t i = 0; i < n; ++i )
        {
            images_.emplace_back( device, create_info );
        }
    }


    uint32_t ImageCollection::image_count( ) const
    {
        return static_cast<uint32_t>( images_.size( ) );
    }


    VkFormat ImageCollection::image_format( ) const
    {
        return image_format_;
    }


    VkExtent2D ImageCollection::image_extent( ) const
    {
        return image_extent_;
    }


    Image& ImageCollection::image_at( size_t const index )
    {
        assert( index < images_.size( ) && "Image::image_at: index out of range!" );
        return images_[index];
    }

}
