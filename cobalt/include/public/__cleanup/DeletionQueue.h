#ifndef DELETIONQUEUE_H
#define DELETIONQUEUE_H

#include <functional>
#include <stack>


namespace cobalt::cleanup
{
    class DeletionQueue final
    {
    public:
        DeletionQueue( )           = default;
        ~DeletionQueue( ) noexcept = default;

        DeletionQueue( DeletionQueue const& )                = delete;
        DeletionQueue( DeletionQueue&& ) noexcept            = delete;
        DeletionQueue& operator=( DeletionQueue const& )     = delete;
        DeletionQueue& operator=( DeletionQueue&& ) noexcept = delete;

        void push( std::function<void( )>&& deleter );
        void flush( );
        [[nodiscard]] bool is_flushed() const;

    private:
        std::stack<std::function<void( )>> deleters_{};

    };

}


#endif //!DELETIONQUEUE_H
