#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <vector>
#include <__memory/Resource.h>

#include <vulkan/vulkan_core.h>


namespace cobalt
{
    class DeviceSet;
}

namespace cobalt
{
    enum class FrameBufferUsageType
    {
        RENDER_PASS
    };

    struct FrameBufferCreateInfo
    {
        FrameBufferUsageType usage_type{};
        VkRenderPass render_pass{};
        VkExtent2D extent{ 0, 0 };
        std::vector<VkImageView> attachments{};
    };


    class FrameBuffer final : public memory::Resource
    {
    public:
        FrameBuffer( DeviceSet const& device, FrameBufferCreateInfo const& create_info );
        ~FrameBuffer( ) override;

        FrameBuffer( const FrameBuffer& )                = delete;
        FrameBuffer( FrameBuffer&& ) noexcept;
        FrameBuffer& operator=( const FrameBuffer& )     = delete;
        FrameBuffer& operator=( FrameBuffer&& ) noexcept = delete;

        [[nodiscard]] VkFramebuffer handle( ) const;

    private:
        DeviceSet const& device_ref_;
        VkFramebuffer framebuffer_{ VK_NULL_HANDLE };

    };

}


#endif //!FRAMEBUFFER_H
