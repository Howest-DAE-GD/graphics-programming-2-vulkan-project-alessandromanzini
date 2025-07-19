#ifndef COBALTVK_H
#define COBALTVK_H

#include <log.h>
#include <cleanup/DeletionQueue.h>
#include <__context/VkContext.h>
#include <__memory/handle/HandleTable.h>
#include <__memory/handle/handle_aliases.h>
#include <__memory/handle/ResourceHandle.h>

#include <stack>


namespace cobalt
{
    // ReSharper disable once CppInconsistentNaming
    class CobaltVK final
    {
    public:
        ~CobaltVK( );

        CobaltVK( CobaltVK const& )                = delete;
        CobaltVK( CobaltVK&& ) noexcept            = delete;
        CobaltVK& operator=( CobaltVK const& )     = delete;
        CobaltVK& operator=( CobaltVK&& ) noexcept = delete;

        static CobaltVK& get_instance( );
        void reset_instance( );

        // todo: remove this
        [[nodiscard]] VkContext& get_vk_instance( ) const;

        template <typename... args_t>
        VkContextHandle create_instance( args_t&&... args );

        template <typename resource_t, typename... args_t>
            requires std::derived_from<resource_t, memory::Resource>
        [[nodiscard]] ResourceHandle<resource_t, memory::Resource> create_resource( args_t&&... args );

    private:
        cleanup::DeletionQueue deletion_queue_{};
        std::vector<std::unique_ptr<memory::Resource>> resources_{};
        HandleTable<memory::Resource> resources_table_{};

        VkContext* vk_instance_ptr_{ nullptr };

        CobaltVK( ) = default;

    };


    template <typename... args_t>
    VkContextHandle CobaltVK::create_instance( args_t&&... args )
    {
        if ( vk_instance_ptr_ )
        {
            log::logerr<CobaltVK>( "create_instance", "vulkan instance is already set up. call reset_instance() first!" );
            return VkContextHandle{};
        }
        auto handle      = create_resource<VkContext>( std::forward<args_t>( args )... );
        vk_instance_ptr_ = handle.get( );
        return handle;
    }


    template <typename resource_t, typename... args_t>
        requires std::derived_from<resource_t, memory::Resource>
    ResourceHandle<resource_t, memory::Resource> CobaltVK::create_resource( args_t&&... args )
    {
        resource_t* resource{ nullptr };

        if constexpr ( std::is_constructible_v<resource_t, VkContext&, args_t...> )
        {
            if ( not vk_instance_ptr_ )
            {
                log::logerr<CobaltVK>( "create_resource", "vulkan instance is not set up!" );
                return ResourceHandle<resource_t>{};
            }
            auto& resource_uptr = resources_.emplace_back( *vk_instance_ptr_,
                                                           std::make_unique<resource_t>( std::forward<args_t>( args )... ) );
            resource = static_cast<resource_t*>( resource_uptr.get( ) );
        }
        else
        {
            auto& resource_uptr = resources_.emplace_back( std::make_unique<resource_t>( std::forward<args_t>( args )... ) );
            resource            = static_cast<resource_t*>( resource_uptr.get( ) );
        }

        auto const table_info = resources_table_.insert( *resource );
        ResourceHandle<resource_t, memory::Resource> handle{ resources_table_, table_info };
        deletion_queue_.push( [this, resource, table_info]
            {
                resources_table_.erase( table_info );
                std::erase_if( resources_, [resource]( auto const& uptr ) { return uptr.get( ) == resource; } );
            } );
        return handle;
    }


    extern CobaltVK& CVK;

}


#endif //!COBALTVK_H
