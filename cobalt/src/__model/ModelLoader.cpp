#include <__model/ModelLoader.h>


namespace cobalt::loader
{
    ModelLoader::ModelLoader( std::filesystem::path&& path ) : model_path_{ std::move( path ) } { }


    decltype(Model::vertices_)& ModelLoader::get_vertices( Model& model )
    {
        return model.vertices_;
    }


    decltype(Model::indices_)& ModelLoader::get_indices( Model& model )
    {
        return model.indices_;
    }

}
