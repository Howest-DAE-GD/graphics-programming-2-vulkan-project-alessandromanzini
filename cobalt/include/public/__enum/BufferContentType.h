#ifndef BUFFERCONTENTTYPE_H
#define BUFFERCONTENTTYPE_H

#include <cstdint>
#include <type_traits>

#include <vulkan/vulkan_core.h>


namespace cobalt::buffer
{
    enum class BufferContentType : uint8_t
    {
        ANY,
        VERTEX,
        INDEX_UINT8,
        INDEX_UINT16,
        INDEX_UINT32,
        UNIFORM,

        MAX = UINT8_MAX
    };


    [[nodiscard]] inline VkIndexType to_index_type( BufferContentType const content_type )
    {
        switch ( content_type )
        {
            case BufferContentType::INDEX_UINT8:
                return static_cast<VkIndexType>( 0 );
            case BufferContentType::INDEX_UINT16:
                return VK_INDEX_TYPE_UINT16;
            case BufferContentType::INDEX_UINT32:
                return VK_INDEX_TYPE_UINT32;
            default:
                break;
        }
        return VK_INDEX_TYPE_MAX_ENUM;
    }


    template <typename integer_t> requires std::is_integral_v<integer_t>
    [[nodiscard]] BufferContentType to_buffer_content_type( )
    {
        if constexpr ( std::is_same_v<integer_t, uint8_t> )
        {
            return BufferContentType::INDEX_UINT8;
        }
        else if constexpr ( std::is_same_v<integer_t, uint16_t> )
        {
            return BufferContentType::INDEX_UINT16;
        }
        else if constexpr ( std::is_same_v<integer_t, uint32_t> )
        {
            return BufferContentType::INDEX_UINT32;
        }
        else
        {
            return BufferContentType::ANY;
        }
    }

}


#endif //!BUFFERCONTENTTYPE_H
