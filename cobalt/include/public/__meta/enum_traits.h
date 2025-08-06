#ifndef ENUM_TRAITS_H
#define ENUM_TRAITS_H

#include <type_traits>


namespace cobalt::meta
{
    // +---------------------------+
    // | ENABLE ENUM FLAGS TRAIT   |
    // +---------------------------+
    template <typename enum_t>
    struct enable_enum_flags : std::false_type { };
}

// +---------------------------+
// | GLOBAL BITWISE OPS        |
// +---------------------------+
template <typename enum_t> requires std::is_enum_v<enum_t> && cobalt::meta::enable_enum_flags<enum_t>::value
[[nodiscard]] constexpr enum_t operator|( enum_t lhs, enum_t rhs )
{
    using underlying_t = std::underlying_type_t<enum_t>;
    return static_cast<enum_t>( static_cast<underlying_t>( lhs ) | static_cast<underlying_t>( rhs ) );
}


template <typename enum_t> requires std::is_enum_v<enum_t> && cobalt::meta::enable_enum_flags<enum_t>::value
[[nodiscard]] constexpr enum_t operator&( enum_t lhs, enum_t rhs )
{
    using underlying_t = std::underlying_type_t<enum_t>;
    return static_cast<enum_t>( static_cast<underlying_t>( lhs ) & static_cast<underlying_t>( rhs ) );
}


template <typename enum_t> requires std::is_enum_v<enum_t> && cobalt::meta::enable_enum_flags<enum_t>::value
[[nodiscard]] constexpr enum_t operator~( enum_t value )
{
    using underlying_t = std::underlying_type_t<enum_t>;
    return static_cast<enum_t>( ~static_cast<underlying_t>( value ) );
}


template <typename enum_t> requires std::is_enum_v<enum_t> && cobalt::meta::enable_enum_flags<enum_t>::value
[[nodiscard]] constexpr bool any( enum_t value )
{
    using underlying_t = std::underlying_type_t<enum_t>;
    return static_cast<underlying_t>( value ) != static_cast<underlying_t>( 0 );
}


#endif //!ENUM_TRAITS_H
