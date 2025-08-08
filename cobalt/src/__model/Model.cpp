#include <__model/Model.h>

#include <__builder/ModelLoader.h>


namespace cobalt
{
    Model::Model( loader::ModelLoader<Vertex, index_t> const& loader )
    {
        loader.load( vertices_, indices_ );
    }


    std::vector<Vertex> const& Model::vertices( ) const
    {
        return vertices_;
    }


    std::vector<Model::index_t> const& Model::indices( ) const
    {
        return indices_;
    }

}
