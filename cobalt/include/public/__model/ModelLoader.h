#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <filesystem>


namespace cobalt::loader
{
    template <typename v_t, typename i_t>
    class ModelLoader
    {
    public:
        explicit ModelLoader( std::filesystem::path&& path ) : model_path_{ std::move( path ) } { }
        virtual ~ModelLoader( ) noexcept = default;

        ModelLoader( ModelLoader const& )                = delete;
        ModelLoader( ModelLoader&& ) noexcept            = delete;
        ModelLoader& operator=( ModelLoader const& )     = delete;
        ModelLoader& operator=( ModelLoader&& ) noexcept = delete;

        virtual void load( std::vector<v_t>& vertices, std::vector<i_t>& indices ) const = 0;

    protected:
        std::filesystem::path const model_path_{};

    };

}


#endif //!MODELLOADER_H
