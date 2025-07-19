#ifndef HANDLETABLE_H
#define HANDLETABLE_H

#include <__memory/memory_aliases.h>

#include <string_view>
#include <vector>


namespace cobalt
{
    namespace memory
    {
        struct RedirectionInfo final
        {
            handle_index_t index{ NULL_HANDLE_INDEX };
            handle_index_t generation{ 0 };
        };
    }


    namespace handle_log
    {
        void logerr_on_invalid_table_index( std::string_view const& src, memory::handle_index_t index, size_t table_size );
        void logerr_on_invalid_resource_index( std::string_view const& src, memory::handle_index_t index );
        void logerr_on_generation_mismatch( std::string_view const& src, memory::handle_index_t gen1,
                                            memory::handle_index_t gen2 );
    }


    // todo: destroy resource and entry when owner count goes to 0
    // todo: manager log error as actual exceptions
    template <typename resource_t>
    class HandleTable final
    {
        struct HandleSlot
        {
            memory::RedirectionInfo info;
            resource_t* resource;
            uint32_t owner_count{ 1 };
        };

    public:
        HandleTable( )  = default;
        ~HandleTable( ) = default;

        HandleTable( const HandleTable& )                = delete;
        HandleTable( HandleTable&& ) noexcept            = delete;
        HandleTable& operator=( const HandleTable& )     = delete;
        HandleTable& operator=( HandleTable&& ) noexcept = delete;

        /**
         * O(1)/O(n) Insertion of a resource into the handle table.
         * @return
         */
        memory::RedirectionInfo insert( resource_t& );

        /**
         * O(1) retrieval of a resource from the handle table.
         * @return
         * @throws std::range_error if the table index is out of bounds.
         * @throws std::runtime_error if the resource index is NULL_HANDLE_INDEX.
         * @throws std::runtime_error in case of generation mismatch.
         */
        resource_t& at( memory::RedirectionInfo const& ) const;

        /**
         * O(1) erase of a resource from the handle table.
         * @throws std::range_error if the table index is out of bounds.
         * @throws std::runtime_error if the resource index is NULL_HANDLE_INDEX.
         * @throws std::runtime_error in case of generation mismatch.
         */
        void erase( memory::RedirectionInfo const& );

    private:
        std::vector<HandleSlot> table_{};
        std::vector<memory::handle_index_t> empty_slots_{};

        [[nodiscard]] memory::handle_index_t find_empty_slot( );

    };


    template <typename resource_t>
    memory::RedirectionInfo HandleTable<resource_t>::insert( resource_t& resource )
    {
        // Find an empty slot in the table.
        auto index = find_empty_slot( );

        // Use the empty slot if found, otherwise create a new entry.
        if ( index != NULL_HANDLE_INDEX )
        {
            ++table_[index].info.generation;
            table_[index].resource    = &resource;
            table_[index].owner_count = 1;
        }
        else
        {
            index = table_.size( );
            table_.emplace_back( memory::RedirectionInfo{ .index = index, .generation = 0 }, &resource, 1 );
        }

        return table_[index].info;
    }


    template <typename resource_t>
    resource_t& HandleTable<resource_t>::at( memory::RedirectionInfo const& info ) const
    {
        handle_log::logerr_on_invalid_table_index( "at", info.index, table_.size( ) );

        auto const& slot = table_[info.index];
        handle_log::logerr_on_generation_mismatch( "at", slot.info.generation, info.generation );

        return *slot.resource;
    }


    template <typename resource_t>
    void HandleTable<resource_t>::erase( memory::RedirectionInfo const& info )
    {
        handle_log::logerr_on_invalid_table_index( "erase", info.index, table_.size( ) );

        auto& slot = table_[info.index];
        handle_log::logerr_on_generation_mismatch( "erase", slot.info.generation, info.generation );

        // 1. Invalidate the resource entry
        slot.info.index = NULL_HANDLE_INDEX;

        // 2. Mark the slot as empty
        empty_slots_.push_back( info.index );
    }


    template <typename resource_t>
    memory::handle_index_t HandleTable<resource_t>::find_empty_slot( )
    {
        if ( empty_slots_.empty( ) )
        {
            return NULL_HANDLE_INDEX;
        }
        auto const index = empty_slots_.back( );
        empty_slots_.pop_back( );
        return index;
    }

}


#endif //!HANDLETABLE_H
