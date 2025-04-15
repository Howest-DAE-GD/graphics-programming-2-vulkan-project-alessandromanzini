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
    glm::vec2 texCoord;


    bool operator==( const Vertex& other ) const
    {
        return position == other.position && color == other.color && texCoord == other.texCoord;
    }


    static VkVertexInputBindingDescription get_binding_description( )
    {
        VkVertexInputBindingDescription bindingDescription{};

        bindingDescription.binding = 0;
        bindingDescription.stride  = sizeof( Vertex );

        // inputRate parameter can have one of the following values:
        // - VK_VERTEX_INPUT_RATE_VERTEX: Move to the next data entry after each vertex
        // - VK_VERTEX_INPUT_RATE_INSTANCE: Move to the next data entry after each instance
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }


    static std::array<VkVertexInputAttributeDescription, 3> get_attribute_descriptions( )
    {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        // The binding parameter tells Vulkan from which binding the per-vertex data comes.
        // The location parameter references the location directive of the input in the vertex shader.
        // The input in the vertex shader with location 0 is the position, which has two 32-bit float components.
        attributeDescriptions[0].binding  = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset   = offsetof( Vertex, position );

        attributeDescriptions[1].binding  = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset   = offsetof( Vertex, color );

        attributeDescriptions[2].binding  = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format   = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset   = offsetof( Vertex, texCoord );

        return attributeDescriptions;
    }

};


// +---------------------------+
// | VERTEX HASHING            |
// +---------------------------+
template <>
struct std::hash<Vertex>
{
    size_t operator()( const Vertex& vertex ) const noexcept
    {
        return ( hash<glm::vec3>( )( vertex.color ) << 1 ^ hash<glm::vec3>( )( vertex.position ) ) >> 1 ^
               hash<glm::vec2>( )( vertex.texCoord ) << 1;
    }
};


#endif //!VERTEX_H
