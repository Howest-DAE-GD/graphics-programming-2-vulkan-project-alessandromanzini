#ifndef SHADERMODULES_H
#define SHADERMODULES_H

#include <filesystem>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace cobalt_vk::shader
{
    std::vector<char> read_file( const std::filesystem::path& filename );
    VkShaderModule create_shader_module( VkDevice device, const std::vector<char>& code);

}

#endif //SHADERMODULES_H
