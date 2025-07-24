#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <__memory/Resource.h>

#include <vulkan/vulkan_core.h>


namespace cobalt
{
    class DeviceSet;
}

namespace cobalt
{
    struct ImageViewCreateInfo
    {
        VkImage image{ VK_NULL_HANDLE };
        VkFormat format{ VK_FORMAT_UNDEFINED };
        VkImageAspectFlags aspect_flags{ VK_IMAGE_ASPECT_NONE };
    };


    class ImageView final : public memory::Resource
    {
    public:
        ImageView( DeviceSet const& device, ImageViewCreateInfo const& create_info );
        ~ImageView( ) override;

        ImageView( const ImageView& )                = delete;
        ImageView( ImageView&& other ) noexcept      = delete;
        ImageView& operator=( const ImageView& )     = delete;
        ImageView& operator=( ImageView&& ) noexcept = delete;

        [[nodiscard]] VkImageView& handle( );

    private:
        DeviceSet const& device_ref_;
        VkImageView image_view_{ VK_NULL_HANDLE };

    };

}


#endif //!IMAGEVIEW_H
