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
        void load( std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::vector<Mesh>& meshes,
                   std::vector<Material>& materials, std::vector<TextureGroup>& textures ) const override;

    };

}


#endif //!ASSIMPMODELLOADER_H
