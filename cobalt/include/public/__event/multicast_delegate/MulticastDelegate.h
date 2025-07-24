#ifndef MULTICASTDELEGATE_H
#define MULTICASTDELEGATE_H

#include <__event/multicast_delegate/Dispatcher.h>
#include <__meta/function_traits.h>

#include <cassert>
#include <functional>
#include <vector>


namespace cobalt
{
    // +---------------------------+
    // | GENERAL TYPE              |
    // +---------------------------+
    // TODO: support all std::function signatures
    template <typename... delegate_params_t>
    class MulticastDelegate final
    {
    public:
        using dispatcher_t    = Dispatcher<delegate_params_t...>;
        using delegate_info_t = typename dispatcher_t::delegate_info_t;

        using raw_delegate_t = typename meta::function_traits<void( delegate_params_t... )>::raw_fn_t;
        using delegate_t     = typename meta::function_traits<void( delegate_params_t... )>::std_fn_t;


        explicit MulticastDelegate( dispatcher_t& dispatcher );
        ~MulticastDelegate( ) = default;

        MulticastDelegate( const MulticastDelegate& )                = delete;
        MulticastDelegate( MulticastDelegate&& ) noexcept            = delete;
        MulticastDelegate& operator=( const MulticastDelegate& )     = delete;
        MulticastDelegate& operator=( MulticastDelegate&& ) noexcept = delete;

        void bind( raw_delegate_t delegate );

        template <typename class_t, typename method_t>
            requires std::is_same_v<delegate_t, typename meta::function_traits<method_t>::std_fn_t>
        void bind( class_t* binder, method_t delegate );


        void unbind( raw_delegate_t delegate );

        template <typename class_t>
        void unbind( class_t* binder );

        bool empty( ) const;

    private:
        dispatcher_t& dispatcher_ref_;
        std::vector<delegate_info_t> delegates_{};

    };


    template <typename... delegate_params_t>
    MulticastDelegate<delegate_params_t...>::MulticastDelegate( dispatcher_t& dispatcher )
        : dispatcher_ref_{ dispatcher }
    {
        dispatcher_ref_.set_delegates_pool( delegates_ );
    }


    template <typename... delegate_params_t>
    void MulticastDelegate<delegate_params_t...>::bind( raw_delegate_t delegate )
    {
        delegates_.emplace_back( delegate_info_t{ .delegate = delegate_t{ delegate } } );
    }


    template <typename... delegate_params_t>
    template <typename class_t, typename method_t> requires std::is_same_v<typename meta::function_traits<void( delegate_params_t
        ... )>::std_fn_t, typename meta::function_traits<method_t>::std_fn_t>
    void MulticastDelegate<delegate_params_t...>::bind( class_t* binder, method_t delegate )
    {
        assert( binder && "MulticastDelegate::bind: Binder cannot be nullptr!" );
        delegates_.emplace_back(
            delegate_info_t{
                .binder = binder,
                .delegate = delegate_t{
                    [=]( delegate_params_t&&... args )
                        {
                            std::invoke( delegate, binder, std::forward<delegate_params_t>( args )... );
                        }
                }
            } );
    }


    template <typename... delegate_params_t>
    void MulticastDelegate<delegate_params_t...>::unbind( raw_delegate_t delegate )
    {
        std::erase_if( delegates_,
                       [delegate]( delegate_info_t& info )
                           {
                               typedef void ( raw_fn_t )( );
                               auto** address1 = info.delegate.template target<raw_fn_t*>( );
                               auto** address2 = delegate.template target<raw_fn_t*>( );
                               return reinterpret_cast<size_t>( *address1 ) == reinterpret_cast<size_t>( *address2 );
                           } );
    }


    template <typename... delegate_params_t>
    template <typename class_t>
    void MulticastDelegate<delegate_params_t...>::unbind( class_t* binder )
    {
        std::erase_if( delegates_,
                       [binder]( delegate_info_t& info )
                           {
                               return info.binder == binder;
                           } );
    }


    template <typename... delegate_params_t>
    bool MulticastDelegate<delegate_params_t...>::empty( ) const
    {
        return delegates_.empty( );
    }

}


#endif //!MULTICASTDELEGATE_H
