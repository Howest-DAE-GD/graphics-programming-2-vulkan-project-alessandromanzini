#ifndef MEMORY_ALIASES_H
#define MEMORY_ALIASES_H

#include <cstdint>


namespace cobalt
{
    namespace memory
    {
        using handle_index_t = uint32_t;
    }

    static constexpr auto NULL_HANDLE_INDEX = std::numeric_limits<memory::handle_index_t>::max( );

}


#endif //!MEMORY_ALIASES_H
