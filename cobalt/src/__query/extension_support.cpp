#include <__query/extension_support.h>

#include <__validation/compare.h>
#include <__validation/dispatch.h>

#include <vulkan/vulkan_core.h>


namespace cobalt::query
{
    bool check_instance_extensions_support( std::vector<char const*>& extensions )
    {
        // Retrieve a list of supported extensions
        uint32_t extension_count{};
        vkEnumerateInstanceExtensionProperties( nullptr, &extension_count, nullptr );
        std::vector<VkExtensionProperties> available_extensions( extension_count );
        vkEnumerateInstanceExtensionProperties( nullptr, &extension_count, available_extensions.data( ) );

        // Check if all the required extensions are supported
        return validation::contains_required( extensions, available_extensions,
                                                       std::mem_fn( &VkExtensionProperties::extensionName ) );
    }


    bool check_instance_extensions_support( std::vector<char const*> const& extensions )
    {
        std::vector extensions_cp{ extensions };
        return check_instance_extensions_support( extensions_cp );
    }


    bool check_validation_layers_support( std::vector<char const*>& layers )
    {
        // Retrieve a list of supported layers
        uint32_t layer_count{};
        vkEnumerateInstanceLayerProperties( &layer_count, nullptr );
        std::vector<VkLayerProperties> available_layers( layer_count );
        vkEnumerateInstanceLayerProperties( &layer_count, available_layers.data( ) );

        return validation::contains_required( layers, available_layers, std::mem_fn( &VkLayerProperties::layerName ) );
    }


    bool check_validation_layers_support( std::vector<char const*> const& layers )
    {
        std::vector layers_cp{ layers };
        return check_validation_layers_support( layers_cp );
    }

}
