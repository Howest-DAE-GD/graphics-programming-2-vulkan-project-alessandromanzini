#include <cleanup/DeletionQueue.h>

namespace cobalt_vk::cleanup
{
    void DeletionQueue::push( std::function<void( )>&& deleter )
    {
        deleters_.push( std::move( deleter ) );
    }


    void DeletionQueue::flush( )
    {
        while ( !deleters_.empty( ) )
        {
            deleters_.top( )( );
            deleters_.pop( );
        }
    }

}
