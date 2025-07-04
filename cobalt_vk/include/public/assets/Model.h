#ifndef MODEL_H
#define MODEL_H

#include <Vertex.h>
#include <cleanup/Releasable.h>
#include <vulkan/vulkan_core.h>

#include <vector>


namespace cobalt_vk
{
    namespace builders
    {
        class ModelLoader;
    }

    class Model final : public cleanup::Releasable
    {
        friend class builders::ModelLoader;

    public:
        using index_t = uint32_t;

        Model( )           = default;
        ~Model( ) override = default;

        Model( const Model& )                = delete;
        Model( Model&& ) noexcept            = delete;
        Model& operator=( const Model& )     = delete;
        Model& operator=( Model&& ) noexcept = delete;

        void create_vertex_buffer( VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
                                   VkQueue graphicsQueue );

        [[nodiscard]] VkBuffer get_vertex_buffer( ) const { return vertex_buffer_; }
        [[nodiscard]] VkDeviceMemory get_vertex_buffer_memory( ) const { return vertex_buffer_memory_; }

        [[nodiscard]] const std::vector<Vertex>& get_vertices( ) const { return vertices_; }
        [[nodiscard]] const std::vector<index_t>& get_indices( ) const { return indices_; }

        void release( VulkanInstance& instance ) override;

    private:
        std::vector<Vertex> vertices_{};
        std::vector<index_t> indices_{};

        VkBuffer vertex_buffer_{ VK_NULL_HANDLE };
        VkDeviceMemory vertex_buffer_memory_{ VK_NULL_HANDLE };

    };

}


#endif //!MODEL_H
