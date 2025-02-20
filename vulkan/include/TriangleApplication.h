#ifndef TRIANGLEAPPLICATION_H
#define TRIANGLEAPPLICATION_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>


namespace engine
{
    class TriangleApplication
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
        VkFormat swapchain_image_format{};
        VkExtent2D swapchain_extent{};

        std::vector<VkImageView> swapchain_image_views_{};

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

        void vk_create_image_views( );

        static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                              VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                              void* pUserData );

        static VkResult vk_create_debug_utils_messenger_EXT( VkInstance instance,
                                                             const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             VkDebugUtilsMessengerEXT* pDebugMessenger );
        static void vk_destroy_debug_utils_messenger_EXT( VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                                          const VkAllocationCallbacks* pAllocator );

        void init_window( );
        void init_vk( );
        void main_loop( );
        void cleanup( ) const;

    };
}

#endif //TRIANGLEAPPLICATION_H
