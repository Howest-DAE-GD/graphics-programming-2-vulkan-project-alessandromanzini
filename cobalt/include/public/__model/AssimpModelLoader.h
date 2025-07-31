#ifndef ASSIMPMODELLOADER_H
#define ASSIMPMODELLOADER_H

#include <__model/ModelLoader.h>


namespace cobalt::loader
{
    class AssimpModelLoader final : public ModelLoader
    {
    public:
        explicit AssimpModelLoader( std::filesystem::path );

        void load( Model& model ) const override;

    private:

    };

}


#endif //!ASSIMPMODELLOADER_H
