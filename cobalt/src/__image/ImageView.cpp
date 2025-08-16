#include <log.h>
#include <__image/ImageView.h>

#include <__context/DeviceSet.h>
#include <__validation/result.h>


namespace cobalt
{
    ImageView::ImageView( DeviceSet const& device, ImageViewCreateInfo const& create_info )
        : device_ref_{ device }
        , aspect_flags_{ create_info.aspect_flags }
    {
        VkImageViewCreateInfo image_view_info{};

        image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_info.image = create_info.image;

        // The viewType and format fields specify how the image data should be interpreted.
        image_view_info.viewType = create_info.view_type;
        image_view_info.format   = create_info.format;

        // The components field allows you to swizzle the color channels around.
        image_view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        // The subresourceRange field describes what the image's purpose is and which part of the image should be accessed.
        image_view_info.subresourceRange.aspectMask     = create_info.aspect_flags;
        image_view_info.subresourceRange.baseMipLevel   = 0;
        image_view_info.subresourceRange.levelCount     = 1;
        image_view_info.subresourceRange.baseArrayLayer = create_info.base_layer;
        image_view_info.subresourceRange.layerCount     = create_info.view_type == VK_IMAGE_VIEW_TYPE_CUBE ? 6u : 1u;

        validation::throw_on_bad_result( vkCreateImageView( device_ref_.logical( ), &image_view_info, nullptr, &image_view_ ),
                                         "Failed to create image view!" );
    }


    ImageView::~ImageView( )
    {
        vkDestroyImageView( device_ref_.logical( ), image_view_, nullptr );
    }


    VkImageView& ImageView::handle( )
    {
        return image_view_;
    }


    VkImageAspectFlags ImageView::aspect_flags( ) const
    {
        return aspect_flags_;
    }


    VkRenderingAttachmentInfo ImageView::make_color_attachment(
        VkAttachmentLoadOp const load_op, VkAttachmentStoreOp const store_op, VkClearColorValue const clear ) const
    {
        return {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = image_view_,
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = load_op,
            .storeOp = store_op,
            .clearValue = VkClearValue{ .color = clear }
        };
    }


    VkRenderingAttachmentInfo ImageView::make_depth_attachment(
        VkAttachmentLoadOp const load_op, VkAttachmentStoreOp const store_op, VkClearDepthStencilValue const clear ) const
    {
        return {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = image_view_,
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .loadOp = load_op,
            .storeOp = store_op,
            .clearValue = VkClearValue{ .depthStencil = clear }
        };
    }

}
