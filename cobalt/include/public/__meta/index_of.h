#ifndef INDEX_OF_H
#define INDEX_OF_H


namespace cobalt::meta
{
    template <typename target_t, typename... pack_t>
    consteval std::size_t index_of( )
    {
        static_assert( sizeof...( pack_t ) != 0, "pack must have a size of at least 1!" );
        constexpr bool matches[] = { std::is_same_v<std::decay_t<target_t>, std::decay_t<pack_t>>... };
        for ( std::size_t i{}; i < sizeof...( pack_t ); ++i )
        {
            if ( matches[i] ) { return i; }
        }
        return sizeof...( pack_t );
    }


    template <typename target_t, typename... pack_t>
    constexpr target_t& element_of_type( std::tuple<pack_t...>& tuple )
    {
        constexpr auto index = meta::index_of<target_t, pack_t...>( );
        static_assert( index < sizeof...( pack_t ) && "type not found!" );
        return std::get<index>( tuple );
    }


    template <typename target_t, typename... pack_t>
    constexpr target_t const& element_of_type( std::tuple<pack_t...> const& tuple )
    {
        constexpr auto index = meta::index_of<target_t, pack_t...>( );
        static_assert( index < sizeof...( pack_t ) && "type not found!" );
        return std::get<index>( tuple );
    }

}


#endif //!INDEX_OF_H
