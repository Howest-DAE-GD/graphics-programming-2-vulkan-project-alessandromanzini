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
    glm::vec2 uv;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bi_tangent;


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
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = offsetof( Vertex, uv )
            },
            VkVertexInputAttributeDescription{
                .binding = 0,
                .location = 2,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof( Vertex, normal )
            },
            VkVertexInputAttributeDescription{
                .binding = 0,
                .location = 3,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof( Vertex, tangent )
            },
            VkVertexInputAttributeDescription{
                .binding = 0,
                .location = 4,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof( Vertex, bi_tangent )
            },
        };
    }

};


#endif //!VERTEX_H
