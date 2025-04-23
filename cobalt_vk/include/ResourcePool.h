#ifndef RESOURCECLEANER_H
#define RESOURCECLEANER_H

#include <Singleton.h>

#include <cleanup/DeletionQueue.h>
#include <cleanup/Releasable.h>

#include <memory>
#include <stack>
#include <vector>


namespace cobalt_vk
{
    class ResourcePool final : public Singleton<ResourcePool>
    {
        friend class Singleton;
    public:
        ~ResourcePool( ) noexcept override;

        ResourcePool( const ResourcePool& )                = delete;
        ResourcePool( ResourcePool&& ) noexcept            = delete;
        ResourcePool& operator=( const ResourcePool& )     = delete;
        ResourcePool& operator=( ResourcePool&& ) noexcept = delete;

        template <typename resource_t, typename... args_t> requires std::derived_from<resource_t, cleanup::Releasable>
        resource_t& create_resource( VkDevice device, args_t&&... args );

        void push_pool( );
        void pop_pool( );

        void register_resource( cleanup::Releasable& resource, VkDevice device );
        void clean( );

    private:
        std::vector<std::unique_ptr<cleanup::Releasable>> resources_;
        cleanup::DeletionQueue deletion_queue_{};

        std::stack<uint32_t> scoped_queues_stack_{};

        ResourcePool( ) = default;

    };


    template <typename resource_t, typename ... args_t> requires std::derived_from<resource_t, cleanup::Releasable>
    resource_t& ResourcePool::create_resource( VkDevice device, args_t&&... args )
    {
        auto& resource = resources_.emplace_back( std::make_unique<resource_t>( device, std::forward<args_t>( args )... ) );
        register_resource( resource, device );
        return resource;
    }

}


#endif //!RESOURCECLEANER_H
