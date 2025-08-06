#include <__buffer/FrameBuffer.h>

#include <log.h>
#include <__context/DeviceSet.h>
#include <__meta/expect_size.h>
#include <__validation/result.h>


namespace cobalt
{
    FrameBuffer::FrameBuffer( DeviceSet const& device, FrameBufferCreateInfo const& create_info )
        : device_ref_{ device }
    {
        VkFramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

        switch ( create_info.usage_type )
        {
            case FrameBufferUsageType::RENDER_PASS:
                // We first need to specify with which renderPass the framebuffer needs to be compatible.
                framebuffer_info.renderPass = create_info.render_pass;
                break;

            default:
                log::logerr<FrameBuffer>( "Framebuffer", "Unsupported usage type!" );
        }

        // The attachmentCount and pAttachments parameters specify the VkImageView objects that should be bound to the
        // respective attachment descriptions in the render pass pAttachment array.
        framebuffer_info.attachmentCount = static_cast<uint32_t>( create_info.attachments.size( ) );
        framebuffer_info.pAttachments    = create_info.attachments.data( );

        framebuffer_info.width  = create_info.extent.width;
        framebuffer_info.height = create_info.extent.height;
        framebuffer_info.layers = 1;

        validation::throw_on_bad_result(
            vkCreateFramebuffer( device_ref_.logical( ), &framebuffer_info, nullptr, &framebuffer_ ),
            "Failed to create framebuffer!" );
    }


    FrameBuffer::~FrameBuffer( )
    {
        if ( framebuffer_ != VK_NULL_HANDLE )
        {
            vkDestroyFramebuffer( device_ref_.logical( ), framebuffer_, nullptr );
            framebuffer_ = VK_NULL_HANDLE;
        }
    }


    FrameBuffer::FrameBuffer( FrameBuffer&& other ) noexcept
        : device_ref_{ other.device_ref_ }
        , framebuffer_{ other.framebuffer_ }
    {
        meta::expect_size<FrameBuffer, 24u>( );
        other.framebuffer_ = VK_NULL_HANDLE;
    }


    VkFramebuffer FrameBuffer::handle( ) const
    {
        return framebuffer_;
    }

}
