#ifndef RENDERER_H
#define RENDERER_H

#include <__memory/Resource.h>

#include <__descriptor/DescriptorAllocator.h>
#include <__synchronization/RenderSync.h>

#include <cstdint>
#include <vector>


namespace cobalt
{
    class DeviceSet;
    class Swapchain;
    class Image;
    class CommandPool;
    class CommandBuffer;
    class DescriptorAllocator;
}

namespace cobalt
{
    struct RendererCreateInfo
    {
        DeviceSet const* device{ nullptr };
        CommandPool* cmd_pool{ nullptr };
        Swapchain* swapchain{ nullptr };
        uint32_t max_frames_in_flight{ UINT32_MAX };
    };


    class Renderer final : public memory::Resource
    {
    public:
        using record_command_buffer_sig_t = void( CommandBuffer const&, Swapchain&, uint32_t image_index, uint32_t frame_index );
        using update_uniform_buffer_sig_t = void( uint32_t frame_index );

        explicit Renderer( RendererCreateInfo const& );
        VkResult render( ) const;

        void set_record_command_buffer_fn( std::function<record_command_buffer_sig_t> ) noexcept;
        void set_update_uniform_buffer_fn( std::function<update_uniform_buffer_sig_t> ) noexcept;

    private:
        DeviceSet const& device_ref_;
        Swapchain& swapchain_ref_;

        sync::RenderSync const render_sync_;

        uint32_t const max_frames_in_flight_{ UINT32_MAX };
        mutable uint64_t current_frame_{ 0 };

        std::function<record_command_buffer_sig_t> record_command_buffer_fn_{ nullptr };
        std::function<update_uniform_buffer_sig_t> update_uniform_buffer_fn_{ nullptr };

    };

}


#endif //!RENDERER_H
