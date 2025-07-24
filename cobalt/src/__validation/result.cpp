#include <__validation/result.h>

#include <__validation/dispatch.h>

#include <sstream>
#include <string>


namespace cobalt::validation
{
    // +---------------------------+
    // | HELPERS                   |
    // +---------------------------+
    [[nodiscard]] std::string_view get_result_string( VkResult const& result )
    {
        switch ( result )
        {
            case VK_SUCCESS: return "SUCCESS";
            case VK_ERROR_OUT_OF_HOST_MEMORY: return "OUT OF HOST MEMORY";
            case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "OUT OF DEVICE MEMORY";
            case VK_ERROR_INITIALIZATION_FAILED: return "INITIALIZATION FAILED";
            case VK_ERROR_LAYER_NOT_PRESENT: return "LAYER NOT PRESENT";
            case VK_ERROR_EXTENSION_NOT_PRESENT: return "EXTENSION NOT PRESENT";
            case VK_ERROR_INCOMPATIBLE_DRIVER: return "INCOMPATIBLE DRIVER";
            default: return "UNKNOWN RESULT";
        }
    }


    // +---------------------------+
    // | VALIDATION                |
    // +---------------------------+
    void throw_on_bad_result( VkResult const result, std::string_view const& error_message )
    {
        if ( result != VK_SUCCESS )
        {
            std::stringstream ss;
            ss << "Error: " << get_result_string( result ) << std::endl << error_message;

            throw_runtime_error( ss.str( ) );
        }
    }


    void throw_on_bad_result( VkResult const result, std::string_view const& error_message, std::vector<VkResult> const& ignore )
    {
        if ( std::ranges::find( ignore, result ) != ignore.end( ) )
        {
            throw_on_bad_result( result, error_message );
        }
    }

}
