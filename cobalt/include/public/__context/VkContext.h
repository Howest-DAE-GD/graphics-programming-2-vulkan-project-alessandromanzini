#ifndef DEVICE_H
#define DEVICE_H

#include <__memory/Resource.h>

#include <__context/InstanceBundle.h>
#include <__context/ValidationLayers.h>

#include <vulkan/vulkan_core.h>

#include <vector>


namespace cobalt
{
    struct ContextCreateInfo final
    {
        VkApplicationInfo const* app_info{};
        Window const* window{ nullptr };
        std::optional<ValidationLayers> validation_layers{ std::nullopt };
    };

    class Window;
    class VkContext final : public memory::Resource
    {
    public:
        explicit VkContext( ContextCreateInfo const& create_info );
        ~VkContext( ) override;

        VkContext( VkContext const& )                = delete;
        VkContext( VkContext&& ) noexcept            = delete;
        VkContext& operator=( VkContext const& )     = delete;
        VkContext& operator=( VkContext&& ) noexcept = delete;

        void pick_physical_device( std::vector<char const*> extensions );
        void create_logical_device( );
        void wait_idle( ) const;

        [[nodiscard]] InstanceBundle& get_instance( ) const;

        [[nodiscard]] VkDevice get_device( ) const;
        [[nodiscard]] VkPhysicalDevice get_physical_device( ) const;
        [[nodiscard]] VkQueue get_graphics_queue( ) const;
        [[nodiscard]] VkQueue get_present_queue( ) const;

    private:
        std::unique_ptr<ValidationLayers> validation_layers_ptr_{};
        std::unique_ptr<InstanceBundle> instance_bundle_ptr_{};

        VkDevice device_{ VK_NULL_HANDLE };
        VkPhysicalDevice physical_device_{ VK_NULL_HANDLE };
        std::vector<char const*> device_extensions_{};

        VkQueue graphics_queue_{ VK_NULL_HANDLE };
        VkQueue present_queue_{ VK_NULL_HANDLE };

    };

}


#endif //!DEVICE_H
