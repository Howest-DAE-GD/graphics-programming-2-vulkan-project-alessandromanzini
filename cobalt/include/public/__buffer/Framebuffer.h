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
    enum class FramebufferUsageType
    {
        RENDER_PASS
    };

    struct FramebufferCreateInfo
    {
        FramebufferUsageType usage_type{};
        VkRenderPass render_pass{};
        VkExtent2D extent{ 0, 0 };
        std::vector<VkImageView> attachments{};
    };


    class Framebuffer final : public memory::Resource
    {
    public:
        Framebuffer( DeviceSet const& device, FramebufferCreateInfo const& create_info );
        ~Framebuffer( ) override;

        Framebuffer( const Framebuffer& )                = delete;
        Framebuffer( Framebuffer&& ) noexcept;
        Framebuffer& operator=( const Framebuffer& )     = delete;
        Framebuffer& operator=( Framebuffer&& ) noexcept = delete;

        [[nodiscard]] VkFramebuffer handle( ) const;

    private:
        DeviceSet const& device_ref_;
        VkFramebuffer framebuffer_{ VK_NULL_HANDLE };

    };

}


#endif //!FRAMEBUFFER_H
