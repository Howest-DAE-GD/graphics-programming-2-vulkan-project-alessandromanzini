#ifndef CUBEMAPIMAGE_H
#define CUBEMAPIMAGE_H

#include <__memory/Resource.h>

#include <__image/Image.h>
#include <__shader/ShaderModule.h>

#include <filesystem>


namespace cobalt
{
    class DeviceSet;
    class CommandPool;
}

namespace cobalt
{
    struct CubeMapImageCreateInfo
    {
        std::filesystem::path const& path_to_img{};
    };


    class CubeMapImage final : public memory::Resource
    {
    public:
        CubeMapImage( DeviceSet const&, CommandPool&, CubeMapImageCreateInfo const& );
        ~CubeMapImage( ) noexcept override = default;

        CubeMapImage( const CubeMapImage& )                = delete;
        CubeMapImage( CubeMapImage&& ) noexcept            = delete;
        CubeMapImage& operator=( const CubeMapImage& )     = delete;
        CubeMapImage& operator=( CubeMapImage&& ) noexcept = delete;

        [[nodiscard]] Image& hdr_image( ) const;
        [[nodiscard]] Image& cube_image( ) const;

        [[nodiscard]] std::array<ImageView, 6u> generate_cubic_views( DeviceSet const& ) const;

        [[nodiscard]] Image&& steal_cubemap_image( ) const;

    private:
        std::unique_ptr<Image> hdr_image_ptr_{ nullptr };
        std::unique_ptr<Image> cubemap_image_ptr_{ nullptr };

        void load_hdr_image( DeviceSet const&, CommandPool&, CubeMapImageCreateInfo const& create_info );

    };

}


#endif //!CUBEMAPIMAGE_H
