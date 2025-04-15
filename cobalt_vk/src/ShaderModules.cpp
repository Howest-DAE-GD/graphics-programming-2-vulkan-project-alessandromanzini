#include "../include/ShaderModules.h"

#include <cassert>
#include <fstream>

#include <validation/result.h>

using namespace engine::shader;
using namespace cobalt_vk;


std::vector<char> engine::shader::read_file( const std::filesystem::path& filename )
{
    assert( std::filesystem::exists( filename ) && "File does not exist!" );

    if ( std::ifstream file( std::filesystem::absolute( filename ), std::ios::binary ); file.is_open( ) )
    {
        const auto fileSize = file_size( filename );

        // Read entire file on vector buffer
        std::vector<char> buffer( fileSize );
        file.read( buffer.data( ), static_cast<long>( fileSize ) );

        file.close( );
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
    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size( );

    // The data is stored in a std::vector with default allocator already ensures that the data satisfies the alignment
    // requirements of uint32_t
    createInfo.pCode = reinterpret_cast<const uint32_t*>( code.data( ) );

    VkShaderModule shaderModule;
    validation::throw_on_bad_result( vkCreateShaderModule( device, &createInfo, nullptr, &shaderModule ),
                                     "Failed to create shader module!" );

    return shaderModule;
}
