#ifndef MODEL_H
#define MODEL_H

#include <__memory/Resource.h>

#include <__model/ModelLoader.h>
#include <__model/Vertex.h>

#include <vector>


namespace cobalt
{
    class Model final : public memory::Resource
    {
    public:
        using index_t = uint32_t;

        explicit Model( loader::ModelLoader<Vertex, index_t> const& loader );

        [[nodiscard]] std::vector<Vertex> const& vertices( ) const;
        [[nodiscard]] std::vector<index_t> const& indices( ) const;

    private:
        std::vector<Vertex> vertices_{};
        std::vector<index_t> indices_{};

    };

}


#endif //!MODEL_H
