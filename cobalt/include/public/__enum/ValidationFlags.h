#ifndef VALIDATIONFLAGS_H
#define VALIDATIONFLAGS_H

#include <__meta/enum_traits.h>

#include <cstdint>


namespace cobalt
{
    enum class ValidationFlags : uint32_t
    {
        NONE               = 0,
        KHRONOS_VALIDATION = 1 << 0
    };

    template <>
    struct meta::enable_enum_flags<ValidationFlags> : std::true_type {};

}


#endif //!VALIDATIONFLAGS_H
