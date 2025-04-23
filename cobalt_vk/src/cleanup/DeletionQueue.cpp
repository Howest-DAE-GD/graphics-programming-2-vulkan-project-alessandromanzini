#include <DeletionQueue.h>

#include <validation/dispatch.h>


namespace cobalt_vk::cleanup
{
    DeletionQueue::~DeletionQueue( )
    {
        if ( not queue_.empty( ) )
        {
            validation::throw_runtime_error( "DeletionQueue has not been flushed!" );
        }
    }


    void DeletionQueue::push( std::function<void( )>&& deletor )
    {
        queue_.emplace_front( std::move( deletor ) );
    }


    void DeletionQueue::flush( uint32_t size )
    {
        while ( not queue_.empty( ) && size > 0)
        {
            auto& deletor = queue_.front( );
            deletor( );
            queue_.pop_front( );
            --size;
        }
    }

}
