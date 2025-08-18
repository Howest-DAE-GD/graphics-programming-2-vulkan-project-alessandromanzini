#ifndef IMAGECOLLECTION_H
#define IMAGECOLLECTION_H

#include <__memory/Resource.h>

#include <__image/Image.h>

#include <vector>


namespace cobalt
{
    class DeviceSet;
}

namespace cobalt
{
    class ImageCollection final : public memory::Resource
    {
    public:
        ImageCollection( DeviceSet const&, ImageCreateInfo const&, size_t n );
        ~ImageCollection( ) noexcept override = default;

        ImageCollection( const ImageCollection& )                = delete;
        ImageCollection( ImageCollection&& ) noexcept            = delete;
        ImageCollection& operator=( const ImageCollection& )     = delete;
        ImageCollection& operator=( ImageCollection&& ) noexcept = delete;

        [[nodiscard]] uint32_t image_count( ) const;
        [[nodiscard]] VkFormat image_format( ) const;
        [[nodiscard]] VkExtent2D image_extent( ) const;

        [[nodiscard]] Image& image_at( size_t index );

    private:
        VkFormat const image_format_;
        VkExtent2D const image_extent_;

        std::vector<Image> images_{};

    };

}


#endif //!IMAGECOLLECTION_H
