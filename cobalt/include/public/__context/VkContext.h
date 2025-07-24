#ifndef DEVICE_H
#define DEVICE_H

#include <__memory/Resource.h>

#include "DeviceSet.h"
#include "InstanceBundle.h"
#include "ValidationLayers.h"

#include <vulkan/vulkan_core.h>

#include <vector>


namespace cobalt
{
    struct ContextCreateInfo
    {
        VkApplicationInfo const* app_info;
        Window const* window;
        std::optional<ValidationLayers> validation_layers{ std::nullopt };
    };


    class VkContext final : public memory::Resource
    {
    public:
        explicit VkContext( ContextCreateInfo const& create_info );
        ~VkContext( ) override;

        VkContext( VkContext const& )                = delete;
        VkContext( VkContext&& ) noexcept            = delete;
        VkContext& operator=( VkContext const& )     = delete;
        VkContext& operator=( VkContext&& ) noexcept = delete;

        [[nodiscard]] InstanceBundle& instance( ) const;
        [[nodiscard]] DeviceSet& device( ) const;

        void create_device( std::vector<char const*> extensions );

    private:
        std::unique_ptr<ValidationLayers> validation_layers_ptr_{};
        std::unique_ptr<InstanceBundle> instance_bundle_ptr_{};
        std::unique_ptr<DeviceSet> device_set_ptr_{};

    };

}


#endif //!DEVICE_H
