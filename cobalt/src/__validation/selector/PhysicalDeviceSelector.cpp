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
        { DeviceFeatureFlags::SWAPCHAIN_EXTENSION, &is_swapchain_adequate },
        { DeviceFeatureFlags::DYNAMIC_RENDERING_EXTENSION, &is_dynamic_rendering_supported },
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
            .device = device,
            .instance = instance_ref_,
            .extensions = {},
            .extensions_selection = {}
        };
        get_extensions( device, data.extensions );
        vkGetPhysicalDeviceFeatures( device, &data.features );

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
