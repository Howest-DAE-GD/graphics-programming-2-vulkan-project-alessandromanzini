#ifndef HANDLETABLE_H
#define HANDLETABLE_H

#include <__memory/memory_aliases.h>

#include <format>
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

        // todo: create a separate library for exceptions and error handling communication.
        void throw_on_invalid_table_index( std::string_view src, memory::handle_index_t index ) const;
        void throw_on_invalid_resource_index( std::string_view src, memory::handle_index_t index ) const;
        void throw_on_generation_mismatch( std::string_view src, memory::handle_index_t gen1, memory::handle_index_t gen2 ) const;

        static std::string_view get_source_name( );

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
        throw_on_invalid_table_index( "at", info.index );

        auto const& slot = table_[info.index];
        throw_on_generation_mismatch( "at", slot.info.generation, info.generation );

        return *slot.resource;
    }


    template <typename resource_t>
    void HandleTable<resource_t>::erase( memory::RedirectionInfo const& info )
    {
        throw_on_invalid_table_index( "erase", info.index );

        auto& slot = table_[info.index];
        throw_on_generation_mismatch( "erase", slot.info.generation, info.generation );

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


    template <typename resource_t>
    void HandleTable<resource_t>::throw_on_invalid_table_index( std::string_view src, memory::handle_index_t const index ) const
    {
        if ( index >= table_.size( ) )
        {
            throw std::range_error( std::format( "{}::{}: table_index out of bounds.", get_source_name( ), src ) );
        }
    }


    template <typename resource_t>
    void HandleTable<resource_t>::throw_on_invalid_resource_index( std::string_view src,
                                                                   memory::handle_index_t const index ) const
    {
        if ( index == NULL_HANDLE_INDEX )
        {
            throw std::runtime_error( std::format( "{}::{}: resource index is NULL_HANDLE_INDEX.", get_source_name( ), src ) );
        }
    }


    template <typename resource_t>
    void HandleTable<resource_t>::throw_on_generation_mismatch( std::string_view src, memory::handle_index_t const gen1,
                                                                memory::handle_index_t const gen2 ) const
    {
        if ( gen1 != gen2 )
        {
            throw std::runtime_error( std::format( "{}::{}: generation mismatch.", get_source_name( ), src ) );
        }
    }


    template <typename resource_t>
    std::string_view HandleTable<resource_t>::get_source_name( )
    {
        static std::string const SOURCE_NAME = std::format( "HandleTable<{}>", std::string( typeid( resource_t ).name( ) ) );
        return SOURCE_NAME;
    }

}


#endif //!HANDLETABLE_H
