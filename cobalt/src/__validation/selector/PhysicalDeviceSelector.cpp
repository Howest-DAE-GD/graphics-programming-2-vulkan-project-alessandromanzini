#include <__validation/selector/PhysicalDeviceSelector.h>

#include <__command/FeatureCommand.h>
#include <__context/InstanceBundle.h>


namespace cobalt::validation
{
    PhysicalDeviceSelector::PhysicalDeviceSelector( InstanceBundle const& instance, DeviceFeatureFlags const features )
        : instance_ref_{ instance }
        , features_{ features } { }


    bool PhysicalDeviceSelector::select( VkPhysicalDevice const device ) const
    {
        // 1. fetch the physical device features and extensions
        exe::ValidationData data{
            .instance = &instance_ref_,
            .device = device,
            .features = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 },
            .extensions = {}
        };
        vkGetPhysicalDeviceFeatures2( device, &data.features );
        get_extensions( device, data.extensions );

        // 2. cross-check with the validation map
        for ( auto const& [flag, command] : FEATURE_COMMAND_MAP )
        {
            // If any validation fails, the device is not suitable
            if ( any( features_ & flag ) && not command->validate( data ) )
            {
                return false;
            }
        }
        return true;
    }


    exe::EnableData PhysicalDeviceSelector::require( ) const
    {
        exe::EnableData data{};
        data.features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        data.features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
        data.features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        data.features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;

        for ( auto const& [flag, command] : FEATURE_COMMAND_MAP )
        {
            if ( any( features_ & flag ) )
            {
                command->enable( data );
            }
        }
        return data;
    }


    void PhysicalDeviceSelector::get_extensions( VkPhysicalDevice const device, std::vector<VkExtensionProperties>& dest )
    {
        // Check if the device supports the required extensions
        uint32_t extension_count{};
        vkEnumerateDeviceExtensionProperties( device, nullptr, &extension_count, nullptr );

        dest = std::vector<VkExtensionProperties>( extension_count );
        vkEnumerateDeviceExtensionProperties( device, nullptr, &extension_count, dest.data( ) );
    }

}
