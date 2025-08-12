#ifndef TEXTUREGROUP_H
#define TEXTUREGROUP_H

#include <filesystem>


namespace cobalt
{
    enum class TextureType
    {
        DIFFUSE,
    };


    struct TextureGroup
    {
        TextureType type{};
        std::filesystem::path path{};
    };

}


#endif //!TEXTUREGROUP_H
