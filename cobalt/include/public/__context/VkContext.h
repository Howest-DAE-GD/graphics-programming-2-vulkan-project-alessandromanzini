#ifndef DEVICE_H
#define DEVICE_H

#include <__memory/Resource.h>

#include "DeviceSet.h"
#include "InstanceBundle.h"
#include "ValidationLayers.h"

#include <vulkan/vulkan_core.h>

#include <vector>
#include <memory>
#include <optional>
#include <__init/InitWizard.h>


namespace cobalt
{
    struct ContextCreateInfo
    {
        Window const* window;
        VkApplicationInfo app_info;
    };

    using ContextWizard = InitWizard<ContextCreateInfo>::WithFeatures<DeviceFeatureFlags, ValidationLayers>;


    class VkContext final : public memory::Resource
    {
    public:
        explicit VkContext( ContextWizard );
        ~VkContext( ) override;

        VkContext( VkContext const& )                = delete;
        VkContext( VkContext&& ) noexcept            = delete;
        VkContext& operator=( VkContext const& )     = delete;
        VkContext& operator=( VkContext&& ) noexcept = delete;

        [[nodiscard]] InstanceBundle& instance( ) const;
        [[nodiscard]] DeviceSet& device( ) const;

    private:
        std::unique_ptr<ValidationLayers> layers_ptr_{};
        std::unique_ptr<InstanceBundle> instance_bundle_ptr_{};
        std::unique_ptr<DeviceSet> device_set_ptr_{};

        void create_device( DeviceFeatureFlags features );

    };

}


#endif //!DEVICE_H
