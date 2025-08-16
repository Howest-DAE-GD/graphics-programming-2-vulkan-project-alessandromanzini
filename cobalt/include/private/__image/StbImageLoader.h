#ifndef STBIMAGELOADER_H
#define STBIMAGELOADER_H

#include <filesystem>

#include <vulkan/vulkan_core.h>


namespace cobalt::image
{
    [[nodiscard]] uint32_t to_channel_count( VkFormat format );
}

namespace cobalt
{
    class StbImageLoader final
    {
        using load_fn_t = void* ( * )( char const*, int*, int*, int*, int );
    public:
        explicit StbImageLoader( std::filesystem::path const& path, uint32_t channels, bool f_load = false );
        ~StbImageLoader( ) noexcept;

        StbImageLoader( const StbImageLoader& )                = delete;
        StbImageLoader( StbImageLoader&& ) noexcept            = delete;
        StbImageLoader& operator=( const StbImageLoader& )     = delete;
        StbImageLoader& operator=( StbImageLoader&& ) noexcept = delete;

        [[nodiscard]] void* pixels( ) const noexcept;

        [[nodiscard]] uint32_t img_width( ) const noexcept;
        [[nodiscard]] uint32_t img_height( ) const noexcept;
        [[nodiscard]] uint64_t img_size( ) const noexcept;

    private:
        void* pixels_ptr_{ nullptr };

        uint32_t img_channels_{ 0 };
        uint32_t img_channel_size_{ 0 };
        uint32_t img_width_{ 0 };
        uint32_t img_height_{ 0 };

        void load_image( std::filesystem::path const& path, uint32_t desired_channels );
        void load_image_float( std::filesystem::path const& path, uint32_t desired_channels );

        void load_image_impl( load_fn_t fn, std::filesystem::path const& path, uint32_t desired_channels );

    };

}


#endif //!STBIMAGELOADER_H
