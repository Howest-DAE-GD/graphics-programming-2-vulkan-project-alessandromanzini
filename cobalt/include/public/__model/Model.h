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

        [[nodiscard]] Buffer const& surface_buffer( ) const;

        [[nodiscard]] std::span<TextureImage const> textures( ) const;

        [[nodiscard]] std::pair<glm::vec3, glm::vec3> aabb( ) const;

    private:
        std::vector<Mesh> meshes_{};

        std::unique_ptr<Buffer> index_buffer_ptr_{ nullptr };
        std::unique_ptr<Buffer> vertex_buffer_ptr_{ nullptr };

        std::unique_ptr<Buffer> surface_buffer_ptr_{ nullptr };
        std::vector<TextureImage> textures_{};

        glm::vec3 aabb_min_{ 0.0f };
        glm::vec3 aabb_max_{ 0.0f };

        void create_texture_images( DeviceSet const&, CommandPool&, std::span<TextureGroup const> textures );
        void create_materials_buffer( DeviceSet const&, CommandPool&, std::span<SurfaceMap const> materials );
        void calculate_aabb( std::span<Vertex const> vertices );

    };

}


#endif //!MODEL_H
