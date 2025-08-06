#ifndef EXPECT_SIZE_H
#define EXPECT_SIZE_H


namespace cobalt::meta
{
    template <typename target_t, size_t expected>
    consteval void expect_size( )
    {
        static_assert( sizeof( target_t ) == expected, "expected size does not match!" );
    }

}


#endif //!EXPECT_SIZE_H
