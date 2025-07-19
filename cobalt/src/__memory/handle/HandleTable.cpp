#include <__memory/handle/HandleTable.h>

#include <log.h>


namespace cobalt::handle_log
{
    void logerr_on_invalid_table_index( std::string_view const& src, memory::handle_index_t const index, size_t const table_size )
    {
        log::logerr<HandleTable<int>>( src, "table_index out of bounds.", index >= table_size );
    }


    void logerr_on_invalid_resource_index( std::string_view const& src, memory::handle_index_t const index )
    {
        log::logerr<HandleTable<int>>( src, "resource index is NULL_HANDLE_INDEX.", index == NULL_HANDLE_INDEX );
    }


    void logerr_on_generation_mismatch( std::string_view const& src, memory::handle_index_t const gen1,
                                        memory::handle_index_t const gen2 )
    {
        log::logerr<HandleTable<int>>( src, "generation mismatch.", gen1 != gen2 );
    }

}
