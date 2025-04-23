#ifndef MODEL_H
#define MODEL_H

#include <vulkan/vulkan_core.h>

#include <Vertex.h>
#include <assets/Buffer.h>
#include <cleanup/Releasable.h>

#include <filesystem>
#include <vector>


namespace cobalt_vk
{
    namespace builders
    {
        class ModelLoader;
    }

    class Model final : public cleanup::Releasable
    {
        friend class builders::ModelLoader;

    public:
        using index_t = uint32_t;

        Model( std::filesystem::path path, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
               VkQueue graphicsQueue );
        ~Model( ) noexcept override = default;

        Model( const Model& )                = delete;
        Model( Model&& ) noexcept            = delete;
        Model& operator=( const Model& )     = delete;
        Model& operator=( Model&& ) noexcept = delete;

        [[nodiscard]] Buffer& get_vertex_buffer( ) { return *vertex_buffer_ptr_; }
        [[nodiscard]] const Buffer& get_vertex_buffer( ) const { return *vertex_buffer_ptr_; }

        [[nodiscard]] VkDeviceSize get_indexes_size( ) const;
        [[nodiscard]] void* get_indexes_data( );

        void release( VkDevice device ) override;

    private:
        std::vector<Vertex> vertices_{};
        std::vector<index_t> indices_{};

        Buffer* vertex_buffer_ptr_{ nullptr };

    };

}


#endif //!MODEL_H
