#include <ShaderModules.h>

#include <__validation/dispatch.h>
#include <__validation/result.h>

#include <cassert>
#include <fstream>


namespace cobalt::shader
{
    std::vector<char> read_file( std::filesystem::path const& filename )
    {
        assert( std::filesystem::exists( filename ) && "File does not exist!" );

        if ( std::ifstream file{ absolute( filename ), std::ios::binary }; file.is_open( ) )
        {
            auto const file_size = std::filesystem::file_size( filename );

            // Read entire file on vector buffer
            std::vector<char> buffer( file_size );
            file.read( buffer.data( ), static_cast<long>( file_size ) );

            file.close( );
            return buffer;
        }
        validation::throw_runtime_error( "File does not exist or could not be opened: " + filename.string( ) );
        return {};
    }


    VkShaderModule create_shader_module( VkDevice const device, std::vector<char> const& code )
    {
        VkShaderModuleCreateInfo create_info{};
        create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize = code.size( );

        // The data is stored in a std::vector with default allocator already ensures that the data satisfies the alignment
        // requirements of uint32_t
        create_info.pCode = reinterpret_cast<uint32_t const*>( code.data( ) );

        VkShaderModule shader_module;
        validation::throw_on_bad_result( vkCreateShaderModule( device, &create_info, nullptr, &shader_module ),
                                         "Failed to create shader module!" );

        return shader_module;
    }

}
