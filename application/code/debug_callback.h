#ifndef DEBUG_CALLBACK_H
#define DEBUG_CALLBACK_H

#include <vulkan/vulkan_core.h>

#include <string>
#include <string_view>


namespace debug
{
    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback( VkDebugUtilsMessageSeverityFlagBitsEXT const message_severity,
                                                          VkDebugUtilsMessageTypeFlagsEXT /* message_type */,
                                                          VkDebugUtilsMessengerCallbackDataEXT const* callback_data,
                                                          void* /* user_data */ )
    {
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
        static constexpr auto RESET_COLOR{ "\e[0m" };
#pragma GCC diagnostic pop
#else
#pragma warning(push, 0)
        static constexpr auto RESET_COLOR{ "\033[0m" };
#pragma warning(pop)
#endif

        std::string severity_color{};
        switch ( message_severity )
        {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                severity_color = "\033[34m";
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                severity_color = "\033[37m";
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                severity_color = "\033[33m";
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                severity_color = "\033[31m";
                break;
            default:
                severity_color = "\033[36m";
                break;
        }

        printf( "%svalidation layer: %s%s\n", severity_color.c_str(), callback_data->pMessage, RESET_COLOR );
        return VK_FALSE;
    }

}


#endif //!DEBUG_CALLBACK_H
