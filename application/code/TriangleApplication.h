#ifndef TRIANGLEAPPLICATION_H
#define TRIANGLEAPPLICATION_H

#include <CobaltVK.h>

#include <filesystem>
#include <vector>

#include <__buffer/Buffer.h>
#include <__buffer/CommandPool.h>
#include <__context/Window.h>
#include <__image/TextureImage.h>
#include <__model/Model.h>
#include <__pipeline/GraphicsPipeline.h>
#include <__renderer/DescriptorAllocator.h>


namespace cobalt
{
    class TriangleApplication final
    {
    public:
        TriangleApplication( );
        ~TriangleApplication( ) noexcept = default;

        TriangleApplication( TriangleApplication const& )                = delete;
        TriangleApplication( TriangleApplication&& ) noexcept            = delete;
        TriangleApplication& operator=( TriangleApplication const& )     = delete;
        TriangleApplication& operator=( TriangleApplication&& ) noexcept = delete;

        void run( );

    private:
        static constexpr uint32_t WIDTH_{ 800 };
        static constexpr uint32_t HEIGHT_{ 600 };

        // In general, we don't want more than 2 frames in flight at a time. That might cause the CPU to get ahead of the GPU.
        static constexpr uint32_t MAX_FRAMES_IN_FLIGHT_{ 2 };

        static constexpr std::string_view TEXTURE_PATH_{ "resources/viking_room.png" };
        static constexpr std::string_view MODEL_PATH_{ "resources/viking_room.obj" };

        // 1.
        WindowHandle window_{};

        // 2.
        VkContextHandle context_{};

        // 4.
        std::unique_ptr<Swapchain> swapchain_ptr_{};

        // 5.
        std::unique_ptr<DescriptorAllocator> descriptor_allocator_ptr_{};
        std::unique_ptr<GraphicsPipeline> graphics_pipeline_ptr_{};

        // 6.
        std::unique_ptr<CommandPool> command_pool_ptr_{};
        std::array<CommandBuffer*, MAX_FRAMES_IN_FLIGHT_> command_buffers_{};

        // 7.
        std::unique_ptr<TextureImage> texture_image_ptr_{};

        // 8.
        ModelHandle model_{};
        std::unique_ptr<Buffer> index_buffer_ptr_{};
        std::unique_ptr<Buffer> vertex_buffer_ptr_{};

        // 9.
        std::vector<Buffer> uniform_buffers_{};

        // ............
        mutable uint64_t current_frame_{ 0 };

        std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT_> image_available_semaphores_{ VK_NULL_HANDLE };
        std::array<VkSemaphore, 3> render_finished_semaphores_{ VK_NULL_HANDLE };
        std::array<VkFence, MAX_FRAMES_IN_FLIGHT_> in_flight_fences_{ VK_NULL_HANDLE };

        void create_graphics_pipeline( );


        // ............
        void vk_create_sync_objects( );
        // ............


        // ............
        void record_command_buffer( CommandBuffer& buffer, uint32_t image_index ) const;
        void update_uniform_buffer( uint32_t current_image ) const;
        void draw_frame( );
        // ............


        void init_vk( );
        void cleanup( );

        static void configure_relative_path( );

    };

}


#endif //TRIANGLEAPPLICATION_H
