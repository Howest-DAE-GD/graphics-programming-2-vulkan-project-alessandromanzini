#ifndef COMMANDPOOL_H
#define COMMANDPOOL_H

#include <__memory/Resource.h>

#include <__buffer/CommandBuffer.h>

#include <vulkan/vulkan_core.h>

#include <memory>
#include <vector>


namespace cobalt
{
    class VkContext;
}

namespace cobalt
{
    class CommandPool final : public memory::Resource
    {
    public:
        explicit CommandPool( VkContext const&, VkCommandPoolCreateFlags pool_type );
        ~CommandPool( ) noexcept override;

        CommandPool( const CommandPool& )                = delete;
        CommandPool( CommandPool&& ) noexcept            = delete;
        CommandPool& operator=( const CommandPool& )     = delete;
        CommandPool& operator=( CommandPool&& ) noexcept = delete;

        [[nodiscard]] VkCommandPool handle( ) const;

        [[nodiscard]] CommandBuffer& acquire( VkCommandBufferLevel level );
        void release( size_t index );

    private:
        VkContext const& context_ref_;

        VkCommandPoolCreateFlags const pool_type_{ VK_COMMAND_POOL_CREATE_FLAG_BITS_MAX_ENUM };
        VkCommandPool command_pool_{ VK_NULL_HANDLE };

        std::vector<std::unique_ptr<CommandBuffer>> buffer_pool_{};
        std::vector<uint32_t> free_pool_{};

    };

}


#endif //!COMMANDPOOL_H
