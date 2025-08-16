#ifndef CSTR_COMPARATOR_H
#define CSTR_COMPARATOR_H

#include <cstring>


namespace cobalt::meta
{
    struct c_str_less
    {
        bool operator()( char const* lhs, char const* rhs ) const noexcept
        {
            return std::strcmp( lhs, rhs ) < 0;
        }
    };

}


#endif //!CSTR_COMPARATOR_H
