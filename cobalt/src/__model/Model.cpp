#include <__model/Model.h>

#include <__builder/ModelLoader.h>

#include <set>


namespace cobalt
{
    Model::Model( DeviceSet const& device, CommandPool& cmd_pool, loader::ModelLoader<Vertex, index_t> const& loader )
    {
        std::vector<Vertex> vertices{};
        std::vector<index_t> indices{};
        std::vector<SurfaceMap> surface_maps{};
        std::vector<TextureGroup> textures{};

        loader.load( vertices, indices, meshes_, surface_maps, textures );

        // Create buffers
        index_buffer_ptr_ = std::make_unique<Buffer>(
            buffer::make_index_buffer<index_t>( device, cmd_pool, indices )
        );
        vertex_buffer_ptr_ = std::make_unique<Buffer>(
            buffer::make_vertex_buffer<Vertex>( device, cmd_pool, vertices )
        );

        create_texture_images( device, cmd_pool, textures );
        create_materials_buffer( device, cmd_pool, surface_maps );
    }


    std::span<Mesh const> Model::meshes( ) const
    {
        return meshes_;
    }


    Buffer const& Model::vertex_buffer( ) const
    {
        return *vertex_buffer_ptr_;
    }


    Buffer const& Model::index_buffer( ) const
    {
        return *index_buffer_ptr_;
    }


    Buffer const& Model::surface_buffer( ) const
    {
        return *surface_buffer_ptr_;
    }


    std::span<TextureImage const> Model::textures( ) const
    {
        return textures_;
    }


    void Model::create_texture_images( DeviceSet const& device, CommandPool& cmd_pool, std::span<TextureGroup const> textures )
    {
        std::set<std::string> texture_paths{};
        for ( auto const& [type, path] : textures )
        {
            if ( texture_paths.contains( path.string( ) ) )
            {
                printf("DUBLO!\n");
            }
            texture_paths.insert( path.string( ) );
            TextureImageCreateInfo create_info{ .path_to_img = path };
            switch ( type )
            {
                case TextureType::DIFFUSE:
                    create_info.image_format = VK_FORMAT_R8G8B8A8_SRGB;
                break;
                case TextureType::NORMAL:
                    create_info.image_format = VK_FORMAT_R8G8B8A8_UNORM;
                break;
                default:
                    throw std::runtime_error( "unsupported texture type" );
            }
            textures_.emplace_back( device, cmd_pool, create_info );
        }
    }


    void Model::create_materials_buffer( DeviceSet const& device, CommandPool& cmd_pool, std::span<SurfaceMap const> materials )
    {
        auto const buffer_size = materials.size_bytes( );

        Buffer staging_buffer = buffer::make_staging_buffer( device, buffer_size );

        staging_buffer.map_memory( );
        staging_buffer.write( materials.data(), buffer_size );
        staging_buffer.unmap_memory( );

        surface_buffer_ptr_ = std::make_unique<Buffer>( device, buffer_size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

        staging_buffer.copy_to( *surface_buffer_ptr_, cmd_pool );
    }

}
