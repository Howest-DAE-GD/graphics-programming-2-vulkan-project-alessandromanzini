#include <__cleanup/DeletionQueue.h>

namespace cobalt::cleanup
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


    bool DeletionQueue::is_flushed( ) const
    {
        return deleters_.empty( );
    }

}
