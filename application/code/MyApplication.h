#ifndef MYAPPLICATION_H
#define MYAPPLICATION_H

#include <__memory/handle/handle_aliases.h>

#include <vulkan/vulkan_core.h>

#include <filesystem>
#include <vector>


namespace cobalt
{
    class CommandBuffer;
    class Image;
}

class MyApplication final
{
public:
    MyApplication( );
    ~MyApplication( ) noexcept;

    MyApplication( MyApplication const& )                = delete;
    MyApplication( MyApplication&& ) noexcept            = delete;
    MyApplication& operator=( MyApplication const& )     = delete;
    MyApplication& operator=( MyApplication&& ) noexcept = delete;

    void run( );

private:
    static constexpr uint32_t WIDTH_{ 800 };
    static constexpr uint32_t HEIGHT_{ 600 };

    // In general, we don't want more than 2 frames in flight at a time. That might cause the CPU to get ahead of the GPU.
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT_{ 2 };

    static constexpr std::string_view TEXTURE_PATH_{ "resources/viking_room.png" };
    static constexpr std::string_view MODEL_PATH_{ "resources/viking_room.obj" };

    bool running_{ false };

    cobalt::WindowHandle window_{};
    cobalt::VkContextHandle context_{};

    cobalt::SwapchainHandle swapchain_{};

    cobalt::CommandPoolHandle command_pool_{};
    cobalt::DescriptorAllocatorHandle descriptor_allocator_{};
    cobalt::GraphicsPipelineHandle graphics_pipeline_{};

    cobalt::RendererHandle renderer_{};

    cobalt::TextureImageHandle texture_image_{};

    cobalt::ModelHandle model_{};
    cobalt::BufferHandle index_buffer_{};
    cobalt::BufferHandle vertex_buffer_{};

    std::vector<cobalt::BufferHandle> uniform_buffers_{};

    // .CREATION
    void create_graphics_pipeline( );

    // .RENDERING
    void record_command_buffer( cobalt::CommandBuffer const&, cobalt::Image const&, VkDescriptorSet desc_set ) const;
    void update_uniform_buffer( uint32_t current_image ) const;

    static void configure_relative_path( );

};


#endif //MYAPPLICATION_H
