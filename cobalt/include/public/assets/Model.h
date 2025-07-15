#ifndef MODEL_H
#define MODEL_H

#include <Vertex.h>
#include <memory/Resource.h>

#include <vulkan/vulkan_core.h>

#include <vector>


namespace cobalt
{
    namespace builders
    {
        class ModelLoader;
    }

    class Model final : public memory::Resource
    {
        friend class builders::ModelLoader;

    public:
        using index_t = uint32_t;

        Model( ) = default;
        ~Model( ) override;

        Model( Model const& )                = delete;
        Model( Model&& ) noexcept            = delete;
        Model& operator=( Model const& )     = delete;
        Model& operator=( Model&& ) noexcept = delete;

        void create_vertex_buffer( VkDevice device, VkPhysicalDevice physical_device, VkCommandPool command_pool,
                                   VkQueue graphics_queue );

        [[nodiscard]] VkBuffer get_vertex_buffer( ) const { return vertex_buffer_; }
        [[nodiscard]] VkDeviceMemory get_vertex_buffer_memory( ) const { return vertex_buffer_memory_; }

        [[nodiscard]] std::vector<Vertex> const& get_vertices( ) const { return vertices_; }
        [[nodiscard]] std::vector<index_t> const& get_indices( ) const { return indices_; }

    private:
        std::vector<Vertex> vertices_{};
        std::vector<index_t> indices_{};

        VkBuffer vertex_buffer_{ VK_NULL_HANDLE };
        VkDeviceMemory vertex_buffer_memory_{ VK_NULL_HANDLE };

    };

}


#endif //!MODEL_H
