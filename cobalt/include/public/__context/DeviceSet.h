#ifndef DEVICESET_H
#define DEVICESET_H

#include <__memory/Resource.h>

#include <vulkan/vulkan_core.h>

#include <vector>
#include <__enum/DeviceFeatureFlags.h>


namespace cobalt
{
    class ValidationLayers;
    class InstanceBundle;

    class DeviceSet final : public memory::Resource
    {
    public:
        explicit DeviceSet( InstanceBundle const& instance, DeviceFeatureFlags features, ValidationLayers const* validation_layers = nullptr );
        ~DeviceSet( ) override;

        DeviceSet( const DeviceSet& )                = delete;
        DeviceSet( DeviceSet&& ) noexcept            = delete;
        DeviceSet& operator=( const DeviceSet& )     = delete;
        DeviceSet& operator=( DeviceSet&& ) noexcept = delete;

        [[nodiscard]] VkDevice logical() const;
        [[nodiscard]] VkPhysicalDevice physical() const;
        [[nodiscard]] VkQueue graphics_queue() const;
        [[nodiscard]] VkQueue present_queue() const;

        void wait_idle() const;
        void wait_for_fence( VkFence fence, uint64_t timeout = UINT64_MAX ) const;

        void reset_fence( VkFence fence ) const;

    private:
        InstanceBundle const& instance_ref_;
        DeviceFeatureFlags feature_flags_{};
        std::vector<char const*> extensions_{};

        VkDevice device_{ VK_NULL_HANDLE };
        VkPhysicalDevice physical_device_{ VK_NULL_HANDLE };

        VkQueue graphics_queue_{ VK_NULL_HANDLE };
        VkQueue present_queue_{ VK_NULL_HANDLE };

        void pick_physical_device( );
        void create_logical_device( ValidationLayers const* validation_layers );

    };

}


#endif //!DEVICESET_H
