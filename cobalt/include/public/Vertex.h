#ifndef VERTEX_H
#define VERTEX_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <vulkan/vulkan_core.h>

#include <array>
#include <functional>


struct Vertex
{
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 tex_coord;


    bool operator==( Vertex const& other ) const
    {
        return position == other.position && color == other.color && tex_coord == other.tex_coord;
    }


    static consteval VkVertexInputBindingDescription get_binding_description( )
    {
        // inputRate parameter can have one of the following values:
        // - VK_VERTEX_INPUT_RATE_VERTEX: Move to the next data entry after each vertex
        // - VK_VERTEX_INPUT_RATE_INSTANCE: Move to the next data entry after each instance
        return {
            .binding = 0,
            .stride = sizeof( Vertex ),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        };
    }


    static constexpr std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions( )
    {
        return {
            VkVertexInputAttributeDescription{
                .binding = 0,
                .location = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof( Vertex, position )
            },
            VkVertexInputAttributeDescription{
                .binding = 0,
                .location = 1,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof( Vertex, color )
            },
            VkVertexInputAttributeDescription{
                .binding = 0,
                .location = 2,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = offsetof( Vertex, tex_coord )
            }
        };
    }

};


// +---------------------------+
// | VERTEX HASHING            |
// +---------------------------+
template <>
struct std::hash<Vertex>
{
    size_t operator()( Vertex const& vertex ) const noexcept
    {
        return ( hash<glm::vec3>( )( vertex.color ) << 1 ^ hash<glm::vec3>( )( vertex.position ) ) >> 1 ^
               hash<glm::vec2>( )( vertex.tex_coord ) << 1;
    }
};


#endif //!VERTEX_H
