#ifndef COBALTVK_H
#define COBALTVK_H

#include <log.h>
#include <__cleanup/DeletionQueue.h>
#include <__context/VkContext.h>
#include <__memory/handle/HandleTable.h>
#include <cobalt_vk/handle.h>
#include <__memory/handle/ResourceHandle.h>
#include <__render/Swapchain.h>

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

        template <typename resource_t, typename... args_t>
            requires std::derived_from<resource_t, memory::Resource>
        [[nodiscard]] ResourceHandle<resource_t, memory::Resource> create_resource( args_t&&... args );

    private:
        cleanup::DeletionQueue deletion_queue_{};
        std::vector<std::unique_ptr<memory::Resource>> resources_{};
        HandleTable<memory::Resource> resources_table_{};

        CobaltVK( ) = default;

        template <typename resource_t>
        [[nodiscard]] memory::Resource* store_resource( std::unique_ptr<resource_t>&& resource );

    };


    template <typename resource_t, typename... args_t>
        requires std::derived_from<resource_t, memory::Resource>
    ResourceHandle<resource_t, memory::Resource> CobaltVK::create_resource( args_t&&... args )
    {
        auto* const resource = this->store_resource( std::make_unique<resource_t>( std::forward<args_t>( args )... ) );

        auto const table_info = resources_table_.insert( *resource );
        ResourceHandle<resource_t, memory::Resource> handle{ resources_table_, table_info };
        deletion_queue_.push( [this, resource, table_info]
            {
                resources_table_.erase( table_info );
                std::erase_if( resources_, [resource]( auto const& uptr ) { return uptr.get( ) == resource; } );
            } );
        return handle;
    }


    template <typename resource_t>
    memory::Resource* CobaltVK::store_resource( std::unique_ptr<resource_t>&& resource )
    {
        return static_cast<resource_t*>( resources_.emplace_back( std::move( resource ) ).get( ) );
    }


    extern CobaltVK& CVK;

}


#endif //!COBALTVK_H
