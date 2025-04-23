#include <ResourcePool.h>


namespace cobalt_vk
{
    ResourcePool::~ResourcePool( ) noexcept
    {
        clean( );
    }


    void ResourcePool::push_pool( )
    {
        scoped_queues_stack_.push( 0 );
    }

    void ResourcePool::pop_pool( )
    {
        deletion_queue_.flush( scoped_queues_stack_.top( ) );
        scoped_queues_stack_.pop( );
    }


    void ResourcePool::register_resource( cleanup::Releasable& resource, const VkDevice device )
    {
        deletion_queue_.push( resource.get_deletor( device ) );
        scoped_queues_stack_.top( )++;
    }


    void ResourcePool::clean( )
    {
        deletion_queue_.flush( );
    }


}
