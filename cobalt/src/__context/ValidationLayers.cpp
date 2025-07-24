#include <__context/ValidationLayers.h>

#include <__validation/compare.h>
#include <__validation/result.h>
#include <__context/InstanceBundle.h>
#include <__query/extension_support.h>


namespace cobalt
{
    ValidationLayers::ValidationLayers( std::vector<char const*> layers, debug_callback_t const callback )
        : validation_layers_{ std::move( layers ) }
        , debug_callback_{ callback }
    {
        if ( not query::check_validation_layers_support( validation_layers_ ) )
        {
            throw std::runtime_error( "Validation layers requested, but not available!" );
        }
    }


    void ValidationLayers::populate_messanger_debug_info( VkDebugUtilsMessengerCreateInfoEXT& debug_info ) const
    {
        // There are a lot more settings for the behavior of validation layers than just the flags specified in the
        // VkDebugUtilsMessengerCreateInfoEXT struct.
        // Browse to the Vulkan SDK and go to the Config directory. There you will find a vk_layer_settings.txt
        // file that explains how to configure the layers.
        // Fill in a structure with details about the messenger and its callback.
        debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

        // The messageSeverity field allows you to specify all the types of severities you would like
        // your callback to be called for.
        debug_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        // The messageType field lets you filter which types of messages your callback is notified about.
        // Currently, all types are enabled.
        debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debug_info.pUserData       = nullptr;
        debug_info.pfnUserCallback = debug_callback_;
    }


    void ValidationLayers::setup_debug_messenger( InstanceBundle const& instance )
    {
        VkDebugUtilsMessengerCreateInfoEXT debug_info{};
        populate_messanger_debug_info( debug_info );

        auto const func =
                reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
                    vkGetInstanceProcAddr( instance.instance( ), "vkCreateDebugUtilsMessengerEXT" ) );

        VkResult result;
        if ( func != nullptr )
        {
            result = func( instance.instance( ), &debug_info, nullptr, &debug_messenger_ );
        }
        else
        {
            result = VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        validation::throw_on_bad_result( result, "Failed to set up debug messenger!" );
    }


    void ValidationLayers::destroy_debug_messenger( InstanceBundle const& instance ) const
    {
        auto const func =
                reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>( vkGetInstanceProcAddr(
                    instance.instance( ), "vkDestroyDebugUtilsMessengerEXT" ) );
        if ( func != nullptr )
        {
            func( instance.instance( ), debug_messenger_, nullptr );
        }
    }


    std::vector<char const*> const& ValidationLayers::get_validation_layers( ) const
    {
        return validation_layers_;
    }

}
