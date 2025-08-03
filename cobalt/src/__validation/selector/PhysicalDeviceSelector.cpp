#include <__validation/selector/PhysicalDeviceSelector.h>

#include <__command/ValidationCommandBind.h>
#include <__context/InstanceBundle.h>
#include <__validation/device_features.h>

#include <map>


namespace cobalt::validation
{
    using validation_bind_t = exe::ValidationCommandBind<validation_fn_t, validation_arg_t>;

    std::map<DeviceFeatureFlags, validation_fn_t> validation_fn_map{
        { DeviceFeatureFlags::FAMILIES_INDICES_SUITABLE, &is_family_indices_suitable },
        { DeviceFeatureFlags::ANISOTROPIC_SAMPLING, &is_anisotropy_sampler_supported },
        { DeviceFeatureFlags::SWAPCHAIN_EXT, &is_swapchain_adequate },
        { DeviceFeatureFlags::SWAPCHAIN_MAINTENANCE_1_EXT, &is_swapchain_maintenance_1_supported },
        { DeviceFeatureFlags::DYNAMIC_RENDERING_EXT, &is_dynamic_rendering_supported },
        { DeviceFeatureFlags::SYNCHRONIZATION_2_EXT, &is_synchronization_2_supported },
        { DeviceFeatureFlags::VERTEX_INPUT_DYNAMIC_STATE_EXT, &is_vertex_input_dynamic_state_supported },
    };


    PhysicalDeviceSelector::PhysicalDeviceSelector( InstanceBundle const& instance, DeviceFeatureFlags const features )
        : instance_ref_{ instance }
    {
        for ( auto const& [flag, fn] : validation_fn_map )
        {
            if ( any( features & flag ) )
            {
                validators_.emplace_back( std::make_unique<validation_bind_t>( fn ) );
            }
        }
    }


    std::pair<bool, std::vector<char const*>> PhysicalDeviceSelector::select( VkPhysicalDevice const device ) const
    {
        ValidationCallbackData data{
            .instance = instance_ref_,
            .device = device,
            .extensions = {},
            .extensions_selection = {}
        };
        get_extensions( device, data.extensions );

        data.features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        vkGetPhysicalDeviceFeatures2( device, &data.features );

        for ( auto& validator : validators_ )
        {
            static_cast<validation_bind_t*>( validator.get( ) )->set_parameter( data );
            validator->execute( );
            if ( not validator->is_valid( ) )
            {
                return { false, {} };
            }
        }
        return { true, data.extensions_selection };
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
