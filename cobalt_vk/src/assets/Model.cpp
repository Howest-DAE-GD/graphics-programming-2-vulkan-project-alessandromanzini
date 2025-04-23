#include <assets/Model.h>

#include <builders/BufferBuilder.h>
#include <builders/ModelLoader.h>


namespace cobalt_vk
{
    Model::Model( std::filesystem::path path, const VkDevice device,
                  const VkPhysicalDevice physicalDevice,
                  const VkCommandPool commandPool,
                  const VkQueue graphicsQueue )
    {
        builders::ModelLoader{ std::move( path ) }.load( *this );

        builders::DataBufferBuilder{ vertices_.data( ), sizeof( vertices_[0] ) * vertices_.size( ) }
                .build( device, physicalDevice,
                        builders::transfer_ops::BufferToBuffer{ commandPool, graphicsQueue },
                        *vertex_buffer_ptr_ );
    }


    VkDeviceSize Model::get_indexes_size( ) const
    {
        return sizeof( indices_[0] ) * indices_.size( );
    }


    void* Model::get_indexes_data( )
    {
        return indices_.data( );
    }


    void Model::release( const VkDevice device )
    {
        vertex_buffer_ptr_->release( device );
    }

}
