#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <__model/SurfaceMap.h>
#include <__model/Mesh.h>
#include <__model/TextureGroup.h>

#include <filesystem>


namespace cobalt::loader
{
    template <typename v_t, typename i_t>
    class ModelLoader
    {
    public:
        explicit ModelLoader( std::filesystem::path&& path )
        {
            base_path_ = path.parent_path(  );
            model_path_ = std::move( path );
        }

        virtual ~ModelLoader( ) noexcept = default;

        ModelLoader( ModelLoader const& )                = delete;
        ModelLoader( ModelLoader&& ) noexcept            = delete;
        ModelLoader& operator=( ModelLoader const& )     = delete;
        ModelLoader& operator=( ModelLoader&& ) noexcept = delete;

        virtual void load( std::vector<v_t>& vertices, std::vector<i_t>& indices, std::vector<Mesh>& meshes,
                           std::vector<SurfaceMap>& surface_maps, std::vector<TextureGroup>& textures ) const = 0;

    protected:
        std::filesystem::path model_path_{};
        std::filesystem::path base_path_{};

    };

}


#endif //!MODELLOADER_H
