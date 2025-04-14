#ifndef SHADERMODULES_H
#define SHADERMODULES_H

#include <filesystem>
#include <vector>
#include <vulkan/vulkan_core.h>

inline std::string get_result_string( const VkResult& result )
{
    switch ( result )
    {
        case VK_SUCCESS:
            return "SUCCESS";
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            return "OUT OF HOST MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            return "OUT OF DEVICE MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED:
            return "INITIALIZATION FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT:
            return "LAYER NOT PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            return "EXTENSION NOT PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            return "INCOMPATIBLE DRIVER";
        default:
            return "UNKNOWN RESULT";
    }
}

namespace engine::shader
{

    std::vector<char> read_file( const std::filesystem::path& filename );
    VkShaderModule create_shader_module( VkDevice device, const std::vector<char>& code);

}

#endif //SHADERMODULES_H
