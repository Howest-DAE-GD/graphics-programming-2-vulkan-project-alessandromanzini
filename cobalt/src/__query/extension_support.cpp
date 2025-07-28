#include <__query/extension_support.h>

#include <__validation/compare.h>
#include <__validation/dispatch.h>

#include <vulkan/vulkan_core.h>

using cobalt::validation::contains_required;


namespace cobalt::query
{
    bool check_instance_extensions_support( std::vector<char const*> const& extensions )
    {
        // Retrieve a list of supported extensions
        uint32_t extension_count{};
        vkEnumerateInstanceExtensionProperties( nullptr, &extension_count, nullptr );
        std::vector<VkExtensionProperties> available_extensions( extension_count );
        vkEnumerateInstanceExtensionProperties( nullptr, &extension_count, available_extensions.data( ) );

        // Check that all the required extensions are supported
        return contains_required( extensions, available_extensions, std::mem_fn( &VkExtensionProperties::extensionName ) );
    }


    bool check_validation_layers_support( std::vector<char const*> const& layers )
    {
        // Retrieve all supported layers
        uint32_t layer_count{};
        vkEnumerateInstanceLayerProperties( &layer_count, nullptr );
        std::vector<VkLayerProperties> available_layers( layer_count );
        vkEnumerateInstanceLayerProperties( &layer_count, available_layers.data( ) );

        // Check that all the required layers are supported
        return contains_required( layers, available_layers, std::mem_fn( &VkLayerProperties::layerName ) );
    }

}
