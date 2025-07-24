#ifndef FUNCTIONTRAITS_H
#define FUNCTIONTRAITS_H

#include <functional>


namespace cobalt::meta
{
    // +---------------------------+
    // | FUNCTION TRAITS           |
    // +---------------------------+
    template <typename sig_t>
    struct function_traits;

    template <typename _raw_fn_t, typename _return_t, typename... _params_t>
    struct function_traits_info
    {
        using return_t = _return_t;
        using params_t = std::tuple<_params_t...>;

        using raw_fn_t = _raw_fn_t;

        using sig_fn_t = _return_t( _params_t... );
        using std_fn_t = std::function<sig_fn_t>;

        static constexpr std::size_t ARITY = sizeof...( _params_t );
    };


    // +---------------------------+
    // | RAW FUNCTION              |
    // +---------------------------+
    template <typename _return_t, typename... _params_t>
    struct function_traits<_return_t ( * )( _params_t... )>
        : function_traits_info<_return_t ( * )( _params_t... ), _return_t, _params_t...> { };


    // +---------------------------+
    // | NON CONST MEMBER          |
    // +---------------------------+
    template <typename _class_t, typename _return_t, typename... _params_t>
    struct function_traits<_return_t ( _class_t::* )( _params_t... )>
        : function_traits_info<_return_t ( _class_t::* )( _params_t... ), _return_t, _params_t...>
    {
        using class_t = _class_t;
    };


    // +---------------------------+
    // | CONST MEMBER              |
    // +---------------------------+
    template <typename _class_t, typename _return_t, typename... _params_t>
    struct function_traits<_return_t ( _class_t::* )( _params_t... ) const>
        : function_traits_info<_return_t ( _class_t::* )( _params_t... ) const, _return_t, _params_t...>
    {
        using class_t = _class_t;
    };


    // +---------------------------+
    // | STD::FUNCTION             |
    // +---------------------------+
    template <typename _return_t, typename... _params_t>
    struct function_traits<std::function<_return_t( _params_t... )>>
        : function_traits_info<_return_t ( * )( _params_t... ), _return_t, _params_t...> { };


    // +---------------------------+
    // | FUNCTION SIGNATURE        |
    // +---------------------------+
    template <typename _return_t, typename... _params_t>
    struct function_traits<_return_t( _params_t... )>
        : function_traits_info<_return_t ( * )( _params_t... ), _return_t, _params_t...> { };

}


#endif //!FUNCTIONTRAITS_H
