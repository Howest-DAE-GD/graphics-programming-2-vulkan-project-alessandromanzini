#ifndef TRIANGLEAPPLICATION_H
#define TRIANGLEAPPLICATION_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <CobaltVK.h>

#include <filesystem>
#include <vector>

#include <assets/Window.h>
#include <__buffer/Framebuffer.h>
#include <__context/VkContext.h>
#include <__image/Image.h>
#include <__model/Model.h>
#include <__renderer/GraphicsPipeline.h>

#include "Vertex.h"


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
#ifdef NDEBUG
        static inline std::vector<char const*> validation_layers_{};
#else
        static inline std::vector<char const*> validation_layers_{ "VK_LAYER_KHRONOS_validation" };
#endif

        static constexpr uint32_t WIDTH_{ 800 };
        static constexpr uint32_t HEIGHT_{ 600 };

        // In general, we don't want more than 2 frames in flight at a time. That might cause the CPU to get ahead of the GPU.
        static constexpr uint8_t MAX_FRAMES_IN_FLIGHT_{ 2 };

        static constexpr std::string_view TEXTURE_PATH_{ "resources/viking_room.png" };
        static constexpr std::string_view MODEL_PATH_{ "resources/viking_room.obj" };

        WindowHandle window_{};
        VkContextHandle context_{};

        std::unique_ptr<Swapchain> swapchain_ptr_{};
        std::unique_ptr<GraphicsPipeline> graphics_pipeline_ptr_{};

        VkDescriptorSetLayout descriptor_set_layout_{ VK_NULL_HANDLE };

        ModelHandle model_{};

        VkBuffer index_buffer_{ VK_NULL_HANDLE };
        VkDeviceMemory index_buffer_memory_{ VK_NULL_HANDLE };
        std::vector<VkBuffer> uniform_buffers_{};
        std::vector<VkDeviceMemory> uniform_buffers_memory_{};
        std::vector<void*> uniform_buffers_mapped_{};

        VkBuffer staging_buffer_{ VK_NULL_HANDLE };
        VkDeviceMemory staging_buffer_memory_{ VK_NULL_HANDLE };

        std::unique_ptr<Image> texture_image_ptr_{ nullptr };
        VkSampler texture_sampler_{ VK_NULL_HANDLE };

        VkDescriptorPool descriptor_pool_{ VK_NULL_HANDLE };
        std::vector<VkDescriptorSet> descriptor_sets_{};

        VkCommandPool command_pool_{ VK_NULL_HANDLE };
        std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT_> command_buffers_{ VK_NULL_HANDLE };
        mutable uint64_t current_frame_{ 0 };

        std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT_> image_available_semaphores_{ VK_NULL_HANDLE };
        std::array<VkSemaphore, 3> render_finished_semaphores_{ VK_NULL_HANDLE };
        std::array<VkFence, MAX_FRAMES_IN_FLIGHT_> in_flight_fences_{ VK_NULL_HANDLE };

        void vk_create_descriptor_set_layout( );

        void vk_create_graphics_pipeline( );

        void vk_create_image( uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                              VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image,
                              VkDeviceMemory& image_memory ) const;

        void vk_create_texture_image( );
        void vk_create_texture_sampler( );
        void vk_transition_image_layout( VkImage image, VkFormat format, VkImageLayout old_layout,
                                         VkImageLayout new_layout ) const;
        void vk_copy_buffer_to_image( VkBuffer buffer, VkImage image, uint32_t width, uint32_t height ) const;

        void load_model( );
        void vk_create_index_buffer( );

        void vk_create_uniform_buffers( );

        void vk_create_descriptor_pool( );
        void vk_create_descriptor_sets( );

        void vk_create_command_pool( );
        void vk_create_command_buffers( );

        void vk_create_sync_objects( );

        static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback( VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                              VkDebugUtilsMessageTypeFlagsEXT message_type,
                                                              VkDebugUtilsMessengerCallbackDataEXT const* p_callback_data,
                                                              void* p_user_data );

        void record_command_buffer( VkCommandBuffer command_buffer, uint32_t image_index ) const;

        void update_uniform_buffer( uint32_t current_image ) const;

        void draw_frame( );

        void init_window( );
        void init_vk( );
        void configure_relative_path( ) const;
        void main_loop( );
        void cleanup( );

    };

}


#endif //TRIANGLEAPPLICATION_H
