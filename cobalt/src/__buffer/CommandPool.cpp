#include <__buffer/CommandPool.h>

#include <__context/VkContext.h>
#include <__query/queue_family.h>
#include <__validation/result.h>

#include <log.h>
#ifdef COBALT_ENABLE_COMMAND_POOL_LOGGING
constexpr bool ENABLE_LOGGING{ true };
#else
constexpr bool ENABLE_LOGGING{ false };
#endif


namespace cobalt
{
    CommandPool::CommandPool( VkContext const& context, VkCommandPoolCreateFlags const pool_type )
        : context_ref_{ context }
        , pool_type_{ pool_type }
    {
        // There are two possible flags for command pools:
        // 1. VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: Hint that command buffers are rerecorded with new commands very often.
        // 2. VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: Allow command buffers to be rerecorded individually, without
        // this flag they all have to be reset together.
        // We will be recording a command buffer every frame, so we want to be able to reset and rerecord over it.
        VkCommandPoolCreateInfo const pool_create_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = pool_type_,
            .queueFamilyIndex = context_ref_.device( ).graphics_queue( ).queue_family_index( )
        };

        validation::throw_on_bad_result(
            vkCreateCommandPool( context_ref_.device( ).logical( ), &pool_create_info, nullptr, &command_pool_ ),
            "Failed to create command pool!" );
    }


    CommandPool::~CommandPool( ) noexcept
    {
        // 1. Release all buffers
        buffer_pool_.clear( );

        // 2. Destroy the command pool
        vkDestroyCommandPool( context_ref_.device( ).logical( ), command_pool_, nullptr );
    }


    VkCommandPool CommandPool::handle( ) const
    {
        return command_pool_;
    }


    CommandBuffer& CommandPool::acquire( VkCommandBufferLevel const level )
    {
        if ( not free_pool_.empty( ) )
        {
            auto const index = free_pool_.back( );
            free_pool_.pop_back( );
            return *buffer_pool_[index];
        }

        if constexpr ( ENABLE_LOGGING )
        {
            log::loginfo<CommandPool>( "acquire", std::format( "current buffers: {}", buffer_pool_.size( ) + 1 ) );
        }

        return *buffer_pool_.emplace_back( new CommandBuffer{
            *this, context_ref_.device( ), level, buffer_pool_.size( )
        } );
    }


    void CommandPool::release( size_t const index )
    {
        assert( index < buffer_pool_.size( ) && "index is out of range!" );
        free_pool_.push_back( static_cast<uint32_t>( index ) );
    }

}
