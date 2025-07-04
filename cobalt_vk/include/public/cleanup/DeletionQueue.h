#ifndef DELETIONQUEUE_H
#define DELETIONQUEUE_H

#include <stack>


namespace cobalt_vk::cleanup
{
    class DeletionQueue final
    {
    public:
        DeletionQueue( )           = default;
        ~DeletionQueue( ) noexcept = default;

        DeletionQueue( const DeletionQueue& )                = delete;
        DeletionQueue( DeletionQueue&& ) noexcept            = delete;
        DeletionQueue& operator=( const DeletionQueue& )     = delete;
        DeletionQueue& operator=( DeletionQueue&& ) noexcept = delete;

        void push( std::function<void( )>&& deleter );
        void flush( );

    private:
        std::stack<std::function<void( )>> deleters_{};

    };

}


#endif //!DELETIONQUEUE_H
