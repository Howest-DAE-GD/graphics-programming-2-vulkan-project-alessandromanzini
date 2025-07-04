#include <instance/ValidationLayers.h>

#include <validation/compare.h>
#include <validation/dispatch.h>
#include <validation/result.h>


namespace cobalt_vk
{
    ValidationLayers::ValidationLayers( layers_vector_t&& layers, debug_callback_t callback )
        : validation_layers_{ std::move( layers ) }
        , debug_callback_{ callback } { }


    void ValidationLayers::populate_debug_info( VkDebugUtilsMessengerCreateInfoEXT& debugInfo ) const
    {
        // There are a lot more settings for the behavior of validation layers than just the flags specified in the
        // VkDebugUtilsMessengerCreateInfoEXT struct.
        // Browse to the Vulkan SDK and go to the Config directory. There you will find a vk_layer_settings.txt
        // file that explains how to configure the layers.
        // Fill in a structure with details about the messenger and its callback.
        debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

        // The messageSeverity field allows you to specify all the types of severities you would like
        // your callback to be called for.
        debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        // The messageType field lets you filter which types of messages your callback is notified about.
        // Currently, all types are enabled.
        debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugInfo.pUserData       = nullptr;
        debugInfo.pfnUserCallback = debug_callback_;
    }


    void ValidationLayers::setup_debug_messenger( const VkInstance instance )
    {
        VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
        populate_debug_info( debugInfo );

        const auto func =
                reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
                    vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" ) );

        VkResult result;
        if ( func != nullptr )
        {
            result = func( instance, &debugInfo, nullptr, &debug_messenger_ );
        }
        else
        {
            result = VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        validation::throw_on_bad_result(result, "Failed to set up debug messenger!" );
    }


    void ValidationLayers::destroy_debug_messenger( const VkInstance instance ) const
    {
        const auto func =
            reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>( vkGetInstanceProcAddr(
                instance, "vkDestroyDebugUtilsMessengerEXT" ) );
        if ( func != nullptr )
        {
            func( instance, debug_messenger_, nullptr );
        }
    }


    const ValidationLayers::layers_vector_t& ValidationLayers::get_validation_layers( ) const
    {
        return validation_layers_;
    }


    void ValidationLayers::check_support( ) const
    {
        // Retrieve a list of supported layers
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties( &layerCount, nullptr );
        std::vector<VkLayerProperties> availableLayers( layerCount );
        vkEnumerateInstanceLayerProperties( &layerCount, availableLayers.data( ) );

        const bool result = validation::compare_support_containers<VkLayerProperties>( validation_layers_, availableLayers,
            []( const VkLayerProperties& layer )
                {
                    return layer.layerName;
                } );
        if ( not result )
        {
            validation::throw_runtime_error( "Validation layers requested, but not available!" );
        }
    }

}
