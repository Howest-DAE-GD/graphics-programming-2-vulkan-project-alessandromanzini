#ifndef SHADERMODULES_H
#define SHADERMODULES_H

#include <filesystem>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace cobalt::shader
{
    std::vector<char> read_file( std::filesystem::path const& filename );
    VkShaderModule create_shader_module( VkDevice device, std::vector<char> const& code );

}

#endif //SHADERMODULES_H
