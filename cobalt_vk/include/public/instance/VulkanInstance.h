#ifndef DEVICE_H
#define DEVICE_H

#include <assets/Window.h>
#include <instance/ValidationLayers.h>
#include <validation/compare.h>

#include <vulkan/vulkan_core.h>

#include <vector>


namespace cobalt_vk
{
    class VulkanInstance final
    {
    public:
        VulkanInstance( const VkApplicationInfo& appInfo, const Window& window );
        VulkanInstance( const VkApplicationInfo& appInfo, const Window& window, ValidationLayers::layers_vector_t validationLayers,
            ValidationLayers::debug_callback_t&& debugCallback );
        ~VulkanInstance( );

        VulkanInstance( const VulkanInstance& )                = delete;
        VulkanInstance( VulkanInstance&& ) noexcept            = delete;
        VulkanInstance& operator=( const VulkanInstance& )     = delete;
        VulkanInstance& operator=( VulkanInstance&& ) noexcept = delete;

        [[nodiscard]] VkInstance get_instance( ) const;
        [[nodiscard]] VkDevice get_device( ) const;
        [[nodiscard]] VkSurfaceKHR get_surface( ) const;

        void set_device( VkDevice );

        void wait_idle() const;

    private:
        //VkPhysicalDevice physical_device_{ VK_NULL_HANDLE };
        VkDevice device_{ VK_NULL_HANDLE };
        VkInstance instance_{ VK_NULL_HANDLE };

        VkSurfaceKHR surface_{ VK_NULL_HANDLE };

        std::unique_ptr<ValidationLayers> validation_layers_ptr_{};

        void create_instance( const VkApplicationInfo& appInfo, const Window& window );
        void create_surface( const Window& window );

        [[nodiscard]] static bool check_extension_support( const std::vector<const char*>& extensions );

    };

}


#endif //!DEVICE_H
