#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <__model/Model.h>

#include <filesystem>


namespace cobalt::loader
{
    class ModelLoader
    {
    public:
        explicit ModelLoader( std::filesystem::path&& );
        virtual ~ModelLoader( ) noexcept = default;

        ModelLoader( ModelLoader const& )                = delete;
        ModelLoader( ModelLoader&& ) noexcept            = delete;
        ModelLoader& operator=( ModelLoader const& )     = delete;
        ModelLoader& operator=( ModelLoader&& ) noexcept = delete;

        virtual void load( Model& model ) const = 0;

    protected:
        std::filesystem::path const model_path_{};

        [[nodiscard]] static decltype(Model::vertices_)& get_vertices( Model& model );
        [[nodiscard]] static decltype(Model::indices_)& get_indices( Model& model );

    };

}


#endif //!MODELLOADER_H
