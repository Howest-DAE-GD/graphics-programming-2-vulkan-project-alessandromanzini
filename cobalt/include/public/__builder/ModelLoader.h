#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <assets/Model.h>

#include <filesystem>


namespace cobalt::builder
{
    class ModelLoader final
    {
    public:
        explicit ModelLoader( std::filesystem::path path );
        ~ModelLoader( ) noexcept = default;

        ModelLoader( ModelLoader const& )                = delete;
        ModelLoader( ModelLoader&& ) noexcept            = delete;
        ModelLoader& operator=( ModelLoader const& )     = delete;
        ModelLoader& operator=( ModelLoader&& ) noexcept = delete;

        void load( Model& model ) const;

    private:
        std::filesystem::path const model_path_{};

    };

}


#endif //!MODELLOADER_H
