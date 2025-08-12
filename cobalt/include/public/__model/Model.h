#ifndef MODEL_H
#define MODEL_H

#include <__memory/Resource.h>

#include <__buffer/Buffer.h>
#include <__image/TextureImage.h>
#include <__model/Mesh.h>
#include <__model/ModelLoader.h>
#include <__model/Vertex.h>

#include <memory>
#include <vector>


namespace cobalt
{
    class DeviceSet;
    class CommandPool;
}

namespace cobalt
{
    class Model final : public memory::Resource
    {
    public:
        using index_t = uint32_t;

        explicit Model( DeviceSet const&, CommandPool&, loader::ModelLoader<Vertex, index_t> const& loader );
        ~Model( ) noexcept override = default;

        Model( const Model& )                = delete;
        Model( Model&& ) noexcept            = delete;
        Model& operator=( const Model& )     = delete;
        Model& operator=( Model&& ) noexcept = delete;

        [[nodiscard]] std::span<Mesh const> meshes( ) const;

        [[nodiscard]] Buffer const& vertex_buffer( ) const;
        [[nodiscard]] Buffer const& index_buffer( ) const;

        [[nodiscard]] Buffer const& materials_buffer( ) const;

        [[nodiscard]] std::span<TextureImage const> textures( ) const;

    private:
        std::vector<Mesh> meshes_{};

        std::unique_ptr<Buffer> index_buffer_ptr_{ nullptr };
        std::unique_ptr<Buffer> vertex_buffer_ptr_{ nullptr };

        std::unique_ptr<Buffer> materials_buffer_ptr_{ nullptr };
        std::vector<TextureImage> textures_{};

        void create_materials_buffer( DeviceSet const& device, CommandPool& cmd_pool, std::span<Material const> materials );

    };

}


#endif //!MODEL_H
