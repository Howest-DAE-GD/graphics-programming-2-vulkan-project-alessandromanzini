#ifndef TRIANGLEAPPLICATION_H
#define TRIANGLEAPPLICATION_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "Vertex.h"


namespace engine
{
    class TriangleApplication final
    {
    public:
        TriangleApplication( )           = default;
        ~TriangleApplication( ) noexcept = default;

        TriangleApplication( const TriangleApplication& )                = delete;
        TriangleApplication( TriangleApplication&& ) noexcept            = delete;
        TriangleApplication& operator=( const TriangleApplication& )     = delete;
        TriangleApplication& operator=( TriangleApplication&& ) noexcept = delete;

        void run( );

    private:
        static constexpr uint32_t WIDTH_  = 800;
        static constexpr uint32_t HEIGHT_ = 600;

#ifdef NDEBUG
        static constexpr bool ENABLE_VALIDATION_LAYERS_{ false };
        static constexpr std::array<const char*, 0> VALIDATION_LAYERS_{ };
#else
        static constexpr bool ENABLE_VALIDATION_LAYERS_{ true };
        static constexpr std::array<const char*, 1> VALIDATION_LAYERS_{ "VK_LAYER_KHRONOS_validation" };
#endif
        static constexpr std::array<const char*, 1> DEVICE_EXTENSIONS_{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

        // In general, we don't want more than 2 frames in flight at a time. That might cause the cpu to get the CPU
        // ahead of the GPU, causing latency.
        static constexpr uint8_t MAX_FRAMES_IN_FLIGHT_{ 2 };

        GLFWwindow* window_ptr_{ nullptr };
        VkInstance instance_{ VK_NULL_HANDLE };
        VkDebugUtilsMessengerEXT debug_messenger_{ VK_NULL_HANDLE };

        VkPhysicalDevice physical_device_{ VK_NULL_HANDLE };
        VkDevice device_{ VK_NULL_HANDLE };

        VkQueue graphics_queue_{ VK_NULL_HANDLE };
        VkQueue present_queue_{ VK_NULL_HANDLE };

        VkSurfaceKHR surface_{ VK_NULL_HANDLE };

        VkSwapchainKHR swapchain_{ VK_NULL_HANDLE };
        std::vector<VkImage> swapchain_images_{};
        VkFormat swapchain_image_format_{};
        VkExtent2D swapchain_extent_{};

        std::vector<VkImageView> swapchain_image_views_{};
        std::vector<VkFramebuffer> swapchain_frame_buffers_{};

        VkDescriptorSetLayout descriptor_set_layout_{ VK_NULL_HANDLE };

        VkRenderPass render_pass_{ VK_NULL_HANDLE };
        VkPipelineLayout pipeline_layout_{ VK_NULL_HANDLE };
        VkPipeline graphics_pipeline_{ VK_NULL_HANDLE };

        VkBuffer vertex_buffer_{ VK_NULL_HANDLE };
        VkDeviceMemory vertex_buffer_memory_{ VK_NULL_HANDLE };
        VkBuffer index_buffer_{ VK_NULL_HANDLE };
        VkDeviceMemory index_buffer_memory_{ VK_NULL_HANDLE };
        std::vector<VkBuffer> uniform_buffers_{};
        std::vector<VkDeviceMemory> uniform_buffers_memory_{};
        std::vector<void*> uniform_buffers_mapped_{};

        VkBuffer staging_buffer_{ VK_NULL_HANDLE };
        VkDeviceMemory staging_buffer_memory_{ VK_NULL_HANDLE };

        VkImage texture_image_{ VK_NULL_HANDLE };
        VkDeviceMemory texture_image_memory_{ VK_NULL_HANDLE };
        VkImageView texture_image_view_{ VK_NULL_HANDLE };
        VkSampler texture_sampler_{ VK_NULL_HANDLE };

        VkImage depth_image_{ VK_NULL_HANDLE };
        VkDeviceMemory depth_image_memory_{ VK_NULL_HANDLE };
        VkImageView depth_image_view_{ VK_NULL_HANDLE };

        VkDescriptorPool descriptor_pool_{ VK_NULL_HANDLE };
        std::vector<VkDescriptorSet> descriptor_sets_{};

        std::vector<Vertex> vertices_{
            { { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
            { { 0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
            { { 0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
            { { -0.5f, 0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },

            { { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
            { { 0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
            { { 0.5f, 0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
            { { -0.5f, 0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } }
        };

        std::vector<uint32_t> indices_{
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4
        };

        VkCommandPool command_pool_{ VK_NULL_HANDLE };
        std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT_> command_buffers_{ VK_NULL_HANDLE };
        mutable uint32_t current_frame_{ 0 };

        std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT_> image_available_semaphores_{ VK_NULL_HANDLE };
        std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT_> render_finished_semaphores_{ VK_NULL_HANDLE };
        std::array<VkFence, MAX_FRAMES_IN_FLIGHT_> in_flight_fences_{ VK_NULL_HANDLE };

        bool frame_buffer_resized_{ false };

        void vk_create_instance( );
        void vk_setup_debug_messenger( );
        static void vk_populate_debug_messenger_create_info( VkDebugUtilsMessengerCreateInfoEXT& info );

        static bool vk_check_extension_support( const std::vector<const char*>& extensions );
        static bool vk_check_validation_layer_support( const std::vector<const char*>& layers );

        void vk_pick_physical_device( );
        bool vk_is_device_suitable( VkPhysicalDevice device ) const;
        static bool vk_check_device_extension_support( VkPhysicalDevice device );

        void vk_create_logical_device( );

        void vk_create_surface( );

        void vk_create_swap_chain( );
        [[nodiscard]] VkSurfaceFormatKHR vk_choose_swap_surface_format(
            const std::vector<VkSurfaceFormatKHR>& availableFormats ) const;
        [[nodiscard]] VkPresentModeKHR vk_choose_swap_present_mode(
            const std::vector<VkPresentModeKHR>& availablePresentModes ) const;
        [[nodiscard]] VkExtent2D vk_choose_swap_extent( const VkSurfaceCapabilitiesKHR& capabilities ) const;
        void vk_recreate_swap_chain( );
        void vk_cleanup_swap_chain( ) const;

        void vk_create_image_views( );

        void vk_create_render_pass( );

        void vk_create_descriptor_set_layout( );

        void vk_create_graphics_pipeline( );

        void vk_create_buffer( VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                               VkBuffer& buffer, VkDeviceMemory& bufferMemory ) const;
        void vk_copy_buffer( VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size ) const;

        void vk_create_image( uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                              VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image,
                              VkDeviceMemory& imageMemory ) const;

        void vk_create_texture_image( );
        void vk_create_texture_image_view( );
        void vk_create_texture_sampler( );
        void vk_transition_image_layout( VkImage image, VkFormat format, VkImageLayout oldLayout,
                                         VkImageLayout newLayout ) const;
        void vk_copy_buffer_to_image( VkBuffer buffer, VkImage image, uint32_t width, uint32_t height ) const;
        [[nodiscard]] VkImageView vk_create_image_view( VkImage image, VkFormat format, VkImageAspectFlags aspectFlags ) const;

        void vk_create_vertex_buffer( );
        void vk_create_index_buffer( );

        void vk_create_frame_buffers( );

        void vk_create_uniform_buffers( );

        void vk_create_descriptor_pool( );
        void vk_create_descriptor_sets( );

        void vk_create_command_pool( );
        void vk_create_command_buffers( );

        void vk_create_sync_objects( );

        void vk_create_depth_resources();

        static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                              VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                              void* pUserData );

        static void frame_buffer_size_callback( GLFWwindow* pWindow, int width, int height );

        static VkResult vk_create_debug_utils_messenger_EXT( VkInstance instance,
                                                             const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             VkDebugUtilsMessengerEXT* pDebugMessenger );
        static void vk_destroy_debug_utils_messenger_EXT( VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                                          const VkAllocationCallbacks* pAllocator );

        void record_command_buffer( VkCommandBuffer commandBuffer, uint32_t imageIndex ) const;

        void update_uniform_buffer( uint32_t currentImage ) const;

        void draw_frame( );

        void init_window( );
        void init_vk( );
        void configure_relative_path( ) const;
        void main_loop( );
        void cleanup( ) const;

    };

}


#endif //TRIANGLEAPPLICATION_H
