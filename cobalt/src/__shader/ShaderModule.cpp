#include <__shader/ShaderModule.h>

#include <__context/DeviceSet.h>
#include <__validation/dispatch.h>
#include <__validation/result.h>

#include <cassert>
#include <fstream>
#include <__meta/expect_size.h>


namespace cobalt::shader
{
    // +---------------------------+
    // | FILE UTILITY              |
    // +---------------------------+
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


    // +---------------------------+
    // | SHADER MODULE             |
    // +---------------------------+
    ShaderModule::ShaderModule( DeviceSet const& device, std::filesystem::path const& path, VkShaderStageFlagBits const stage )
        : device_ref_{ device }
        , stage_{ stage }
    {
        auto const code = read_file( path );

        VkShaderModuleCreateInfo create_info{};
        create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize = code.size( );

        // The data is stored in a std::vector with default allocator already ensures that the data satisfies the alignment
        // requirements of uint32_t
        create_info.pCode = reinterpret_cast<uint32_t const*>( code.data( ) );

        validation::throw_on_bad_result( vkCreateShaderModule( device_ref_.logical( ), &create_info, nullptr, &shader_module_ ),
                                         "Failed to create shader module!" );
    }


    ShaderModule::~ShaderModule( ) noexcept
    {
        vkDestroyShaderModule( device_ref_.logical( ), shader_module_, nullptr );
    }


    ShaderModule::ShaderModule( ShaderModule&& other ) noexcept
        : device_ref_{ other.device_ref_ }
        , stage_{ other.stage_ }
        , shader_module_{ std::exchange( other.shader_module_, VK_NULL_HANDLE ) }
    {
        meta::expect_size<ShaderModule, 24u>( );
    }


    VkShaderModule ShaderModule::handle( ) const noexcept
    {
        return shader_module_;
    }


    VkShaderStageFlagBits ShaderModule::stage( ) const noexcept
    {
        return stage_;
    }

}
