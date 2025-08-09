#ifndef DEVICESET_H
#define DEVICESET_H

#include <__memory/Resource.h>

#include <__context/Queue.h>
#include <__enum/DeviceFeatureFlags.h>

#include <vulkan/vulkan_core.h>

#include <vector>


namespace cobalt
{
    class ValidationLayers;
    class InstanceBundle;

    class DeviceSet final : public memory::Resource
    {
    public:
        explicit DeviceSet( InstanceBundle const& instance, DeviceFeatureFlags features,
                            ValidationLayers const* validation_layers = nullptr );
        ~DeviceSet( ) override;

        DeviceSet( const DeviceSet& )                = delete;
        DeviceSet( DeviceSet&& ) noexcept            = delete;
        DeviceSet& operator=( const DeviceSet& )     = delete;
        DeviceSet& operator=( DeviceSet&& ) noexcept = delete;

        [[nodiscard]] VkDevice logical( ) const;
        [[nodiscard]] VkPhysicalDevice physical( ) const;
        [[nodiscard]] Queue& graphics_queue( ) const;
        [[nodiscard]] Queue& present_queue( ) const;

        [[nodiscard]] bool has_feature( DeviceFeatureFlags feature ) const;
        [[nodiscard]] uint32_t device_index( ) const;

        void wait_idle( ) const;
        void wait_for_fence( VkFence fence, uint64_t timeout = UINT64_MAX ) const;

        void reset_fence( VkFence fence ) const;

    private:
        InstanceBundle const& instance_ref_;
        DeviceFeatureFlags feature_flags_{};
        std::vector<char const*> extensions_{};

        uint32_t device_index_{ UINT32_MAX };

        VkDevice device_{ VK_NULL_HANDLE };
        VkPhysicalDevice physical_device_{ VK_NULL_HANDLE };

        std::unique_ptr<Queue> graphics_queue_ptr_{ nullptr };
        std::unique_ptr<Queue> present_queue_ptr_{ nullptr };

        void pick_physical_device( );
        void create_logical_device( ValidationLayers const* validation_layers );

    };

}


#endif //!DEVICESET_H
