#ifndef STBIMAGELOADER_H
#define STBIMAGELOADER_H

#include <filesystem>


namespace cobalt
{
class StbImageLoader final
{
public:
    explicit StbImageLoader( std::filesystem::path const& path, uint32_t channels );
    ~StbImageLoader( ) noexcept;

    StbImageLoader( const StbImageLoader& )                = delete;
    StbImageLoader( StbImageLoader&& ) noexcept            = delete;
    StbImageLoader& operator=( const StbImageLoader& )     = delete;
    StbImageLoader& operator=( StbImageLoader&& ) noexcept = delete;

    [[nodiscard]] void* pixels( ) const noexcept;

    [[nodiscard]] uint32_t img_width( ) const noexcept;
    [[nodiscard]] uint32_t img_height( ) const noexcept;
    [[nodiscard]] uint32_t img_size( ) const noexcept;

private:
    void* pixels_ptr_{ nullptr };

    uint32_t img_channels_{ 0 };
    uint32_t img_width_{ 0 };
    uint32_t img_height_{ 0 };

    void load_image( std::filesystem::path const& path, uint32_t desired_channels );

};

}


#endif //!STBIMAGELOADER_H
