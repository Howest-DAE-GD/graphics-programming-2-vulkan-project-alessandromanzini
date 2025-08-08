#ifndef ASSIMPMODELLOADER_H
#define ASSIMPMODELLOADER_H

#include <__model/ModelLoader.h>

#include <__model/Vertex.h>

#include <cstdint>


namespace cobalt::loader
{
    class AssimpModelLoader final : public ModelLoader<Vertex, uint32_t>
    {
    public:
        explicit AssimpModelLoader( std::filesystem::path );
        void load( std::vector<Vertex>& vertices, std::vector<unsigned>& indices ) const override;

    };

}


#endif //!ASSIMPMODELLOADER_H
