#ifndef PHYSICALDEVICESELECTOR_H
#define PHYSICALDEVICESELECTOR_H

#include <__command/ValidationCommand.h>
#include <__enum/DeviceFeatureFlags.h>

#include <vulkan/vulkan_core.h>

#include <memory>
#include <vector>


namespace cobalt
{
    class InstanceBundle;
}

namespace cobalt::validation
{
    class PhysicalDeviceSelector final
    {
    public:
        PhysicalDeviceSelector( InstanceBundle const& instance, DeviceFeatureFlags features );
        [[nodiscard]] std::pair<bool, std::vector<char const*>> select( VkPhysicalDevice device ) const;

    private:
        InstanceBundle const& instance_ref_;
        std::vector<std::unique_ptr<exe::ValidationCommand>> validators_{};

        static void get_extensions( VkPhysicalDevice device, std::vector<VkExtensionProperties>& dest );

    };
}


#endif //!PHYSICALDEVICESELECTOR_H
