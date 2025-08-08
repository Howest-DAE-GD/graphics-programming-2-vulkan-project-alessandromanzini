#ifndef MODEL_H
#define MODEL_H

#include <__memory/Resource.h>

#include <__buffer/Buffer.h>
#include <__buffer/CommandPool.h>
#include <__model/Vertex.h>

#include <vulkan/vulkan_core.h>

#include <vector>


namespace cobalt
{
    namespace loader
    {
        class ModelLoader;
    }
    class DeviceSet;
}

namespace cobalt
{
    class Model final : public memory::Resource
    {
        friend class loader::ModelLoader;

    public:
        using index_t = uint32_t;

        explicit Model( DeviceSet const& device );
        ~Model( ) override = default;

        Model( Model const& )                = delete;
        Model( Model&& ) noexcept            = delete;
        Model& operator=( Model const& )     = delete;
        Model& operator=( Model&& ) noexcept = delete;

        [[nodiscard]] std::vector<Vertex> const& vertices( ) const;
        [[nodiscard]] std::vector<index_t> const& indices( ) const;

    private:
        DeviceSet const& device_ref_;

        std::vector<Vertex> vertices_{};
        std::vector<index_t> indices_{};

    };

}


#endif //!MODEL_H
