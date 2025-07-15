#ifndef MULTICASTDELEGATE_H
#define MULTICASTDELEGATE_H

#include <functional>


namespace cobalt
{
    // +---------------------------+
    // | GENERAL TYPE              |
    // +---------------------------+
    template <typename... params_t>
    class MulticastDelegate final
    {
    public:
        using delegate_t = std::function<void( params_t... )>;

        MulticastDelegate( )  = default;
        ~MulticastDelegate( ) = default;

        MulticastDelegate( MulticastDelegate const& )                = delete;
        MulticastDelegate( MulticastDelegate&& ) noexcept            = delete;
        MulticastDelegate& operator=( MulticastDelegate const& )     = delete;
        MulticastDelegate& operator=( MulticastDelegate&& ) noexcept = delete;


        void bind( delegate_t&& delegate )
        {
            delegates_.emplace_back( delegate );
        }


        void unbind( delegate_t&& delegate )
        {
            std::erase( delegates_, delegate );
        }


        void broadcast( params_t&&... args ) const
        {
            for ( auto&& delegate : delegates_ )
            {
                delegate( std::forward<params_t>( args )... );
            }
        }

    private:
        std::vector<delegate_t> delegates_{};

    };


    // +---------------------------+
    // | VOID SPECIALIZATION       |
    // +---------------------------+
    template <>
    class MulticastDelegate<void> final
    {
    public:
        using delegate_t = std::function<void( )>;

        MulticastDelegate( )  = default;
        ~MulticastDelegate( ) = default;

        MulticastDelegate( MulticastDelegate const& )                = delete;
        MulticastDelegate( MulticastDelegate&& ) noexcept            = delete;
        MulticastDelegate& operator=( MulticastDelegate const& )     = delete;
        MulticastDelegate& operator=( MulticastDelegate&& ) noexcept = delete;


        void bind( delegate_t&& delegate )
        {
            delegates_.emplace_back( delegate );
        }


        void unbind( delegate_t&& delegate )
        {
            std::erase_if( delegates_,
                           [&delegate]( delegate_t const& d )
                               {
                                   // Check if the delegate's target type matches
                                   return d.target_type( ) == delegate.target_type( );
                               } );
        }


        void broadcast( ) const
        {
            for ( auto&& delegate : delegates_ )
            {
                delegate( );
            }
        }

    private:
        std::vector<delegate_t> delegates_{};

    };

}


#endif //!MULTICASTDELEGATE_H
