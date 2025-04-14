#include "../include/ShaderModules.h"

#include <cassert>
#include <fstream>
#include <iostream>

using namespace engine::shader;

std::vector<char> engine::shader::read_file( const std::filesystem::path& filename )
{
    std::cout << "Reading file " << std::filesystem::absolute(filename) << std::endl;
    std::cout << "This " << std::filesystem::current_path() << std::endl;
    assert( std::filesystem::exists( filename ) && "File does not exist!" );

    if ( std::ifstream file( std::filesystem::absolute(filename), std::ios::binary ); file.is_open( ) )
    {
        const auto fileSize = file_size( filename );

        // Read entire file on vector buffer
        std::vector<char> buffer( fileSize );
        file.read(buffer.data(), static_cast<long>( fileSize ));

        file.close();
        return buffer;
    }
    else
    {
        throw std::runtime_error( "Failed to open file!" );
    }
}

VkShaderModule engine::shader::create_shader_module( const VkDevice device, const std::vector<char>& code )
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();

    // The data is stored in a std::vector with default allocator already ensures that the data satisfies the alignment
    // requirements of uint32_t
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (const auto result = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule); result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module! Error: " + get_result_string(result));
    }

    return shaderModule;
}
