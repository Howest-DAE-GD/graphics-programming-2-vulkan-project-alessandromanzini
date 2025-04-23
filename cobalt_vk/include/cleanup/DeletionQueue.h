#ifndef DELETIONQUEUE_H
#define DELETIONQUEUE_H

#include <deque>
#include <functional>


namespace cobalt_vk::cleanup
{
    class DeletionQueue final
    {
    public:
        DeletionQueue( )           = default;
        ~DeletionQueue( );

        DeletionQueue( const DeletionQueue& )                = delete;
        DeletionQueue( DeletionQueue&& ) noexcept            = delete;
        DeletionQueue& operator=( const DeletionQueue& )     = delete;
        DeletionQueue& operator=( DeletionQueue&& ) noexcept = delete;

        void push( std::function<void( )>&& deletor );
        void flush( uint32_t size = std::numeric_limits<uint32_t>::max( ) );

    private:
        std::deque<std::function<void( )>> queue_{};

    };

}


#endif //!DELETIONQUEUE_H
