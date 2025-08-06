#include <__model/Model.h>

#include <__builder/ModelLoader.h>


namespace cobalt
{
    Model::Model( DeviceSet const& device )
        : device_ref_{ device } { }


    std::vector<Vertex> const& Model::vertices( ) const
    {
        return vertices_;
    }


    std::vector<Model::index_t> const& Model::indices( ) const
    {
        return indices_;
    }

}
