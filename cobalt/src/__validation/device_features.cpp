#include <__validation/device_features.h>

#include <__query/queue_family.h>
#include <__query/swapchain_support.h>
#include <__validation/compare.h>


namespace cobalt::validation
{
    // +---------------------------+
    // | EXTENSION CHECKING        |
    // +---------------------------+
    [[nodiscard]] bool has_extension( ValidationCallbackData& data, char const* extension_name )
    {
        if ( contains_required( extension_name, data.extensions, std::mem_fn( &VkExtensionProperties::extensionName ) ) )
        {
            data.extensions_selection.push_back( extension_name );
            return true;
        }
        return false;
    }


    bool is_family_indices_suitable( ValidationCallbackData& data )
    {
        return query::find_queue_families( data.device, data.instance ).is_suitable( );
    }


    bool is_swapchain_adequate( ValidationCallbackData& data )
    {
        if ( not has_extension( data, VK_KHR_SWAPCHAIN_EXTENSION_NAME ) )
        {
            return false;
        }
        return query::check_swapchain_support( data.device, data.instance ).is_adequate( );
    }


    bool is_dynamic_rendering_supported( ValidationCallbackData& data )
    {
        if ( not has_extension( data, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME ) )
        {
            return false;
        }
        return true;
    }


    bool is_anisotropy_sampler_supported( ValidationCallbackData& data )
    {
        return data.features.samplerAnisotropy;
    }


}
