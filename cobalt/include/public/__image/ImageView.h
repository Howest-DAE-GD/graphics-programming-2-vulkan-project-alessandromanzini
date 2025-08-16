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
        uint32_t base_layer{ 0 };
        VkImageViewType view_type{ VK_IMAGE_VIEW_TYPE_2D };
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
        [[nodiscard]] VkImageAspectFlags aspect_flags( ) const;

        [[nodiscard]] VkRenderingAttachmentInfo make_color_attachment(
            VkAttachmentLoadOp load_op, VkAttachmentStoreOp store_op, VkClearColorValue clear = { { 0.f, 0.f, 0.f, 1.f } } ) const;
        [[nodiscard]] VkRenderingAttachmentInfo make_depth_attachment(
            VkAttachmentLoadOp load_op, VkAttachmentStoreOp store_op, VkClearDepthStencilValue clear = { .depth = 1.f } ) const;

    private:
        DeviceSet const& device_ref_;

        VkImageAspectFlags aspect_flags_{ VK_IMAGE_ASPECT_NONE };

        VkImageView image_view_{ VK_NULL_HANDLE };

    };

}


#endif //!IMAGEVIEW_H
