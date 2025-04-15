#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <assets/Model.h>

#include <filesystem>


namespace cobalt_vk::builders
{
    class ModelLoader final
    {
    public:
        explicit ModelLoader( std::filesystem::path path );
        ~ModelLoader( ) noexcept = default;

        ModelLoader( const ModelLoader& )                = delete;
        ModelLoader( ModelLoader&& ) noexcept            = delete;
        ModelLoader& operator=( const ModelLoader& )     = delete;
        ModelLoader& operator=( ModelLoader&& ) noexcept = delete;

        void load( Model& model ) const;

    private:
        const std::filesystem::path model_path_{};

    };

}


#endif //!MODELLOADER_H
