#ifndef HANDLETABLE_H
#define HANDLETABLE_H

#include <memory/memory_aliases.h>

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

    // todo: create specialization for unique_ptr and shared_ptr. make the template parameter more generic by not getting the pointer type, the resource type itself.
    template <typename resource_t>
    class HandleTable final
    {
    public:
        HandleTable( )  = default;
        ~HandleTable( ) = default;

        HandleTable( HandleTable const& )                = delete;
        HandleTable( HandleTable&& ) noexcept            = delete;
        HandleTable& operator=( HandleTable const& )     = delete;
        HandleTable& operator=( HandleTable&& ) noexcept = delete;

        /**
         * O(n) insertion of a resource into the handle table.
         * @param resource
         * @return
         */
        memory::RedirectionInfo insert( resource_t* resource );

        /**
         * O(1) retrieval of a resource from the handle table.
         * @param table_index
         * @param generation
         * @return
         * @throws std::range_error if the table index is out of bounds.
         * @throws std::runtime_error if the resource index is NULL_HANDLE_INDEX.
         * @throws std::runtime_error in case of generation mismatch.
         */
        resource_t* at( memory::handle_index_t table_index, memory::handle_index_t generation ) const;

        /**
         * O(n) erase of a resource from the handle table.
         * @param table_index
         * @param generation
         * @throws std::range_error if the table index is out of bounds.
         * @throws std::runtime_error if the resource index is NULL_HANDLE_INDEX.
         * @throws std::runtime_error in case of generation mismatch.
         */
        void erase( memory::handle_index_t table_index, memory::handle_index_t generation );

    private:
        std::vector<resource_t*> resources_{};
        std::vector<memory::RedirectionInfo> redirection_map_{};

        // todo: create a separate library for exceptions and error handling communication.
        void throw_on_invalid_table_index( std::string_view src, memory::handle_index_t index ) const;
        void throw_on_invalid_resource_index( std::string_view src, memory::handle_index_t index ) const;
        void throw_on_generation_mismatch( std::string_view src, memory::handle_index_t gen1, memory::handle_index_t gen2 ) const;


        static std::string_view get_source_name( )
        {
            static std::string const SOURCE_NAME = std::format( "HandleTable<{}>", std::string( typeid( resource_t ).name( ) ) );
            return SOURCE_NAME;
        }

    };


    template <typename resource_t>
    memory::RedirectionInfo HandleTable<resource_t>::insert( resource_t* resource )
    {
        resources_.emplace_back( resource );
        auto const new_index = static_cast<memory::handle_index_t>( resources_.size( ) - 1 );

        // If a resource index is null, we can create a new generation and reuse the memory slot.
        for ( auto it = redirection_map_.begin( ); it < redirection_map_.end( ); ++it )
        {
            if ( it->index == NULL_HANDLE_INDEX )
            {
                it->index = new_index;
                ++it->generation;
                return {
                    .index = static_cast<memory::handle_index_t>( std::distance( redirection_map_.begin( ), it ) ),
                    .generation = it->generation
                };
            }
        }

        // ... else create a new redirection entry.
        auto const& redirection = redirection_map_.emplace_back( new_index, 0 );
        return {
            .index = static_cast<memory::handle_index_t>( redirection_map_.size( ) - 1 ),
            .generation = redirection.generation
        };
    }


    template <typename resource_t>
    resource_t* HandleTable<resource_t>::at( memory::handle_index_t const table_index,
                                             memory::handle_index_t const generation ) const
    {
        throw_on_invalid_table_index( "at", table_index );

        auto const& [resource_index, resource_generation] = redirection_map_[table_index];
        throw_on_invalid_resource_index( "at", resource_index );
        throw_on_generation_mismatch( "at", resource_generation, generation );

        return resources_[resource_index];
    }


    template <typename resource_t>
    void HandleTable<resource_t>::erase( memory::handle_index_t const table_index,
                                         memory::handle_index_t const generation )
    {
        throw_on_invalid_table_index( "erase", table_index );

        auto& [resource_index, resource_generation] = redirection_map_[table_index];
        throw_on_invalid_resource_index( "erase", resource_index );
        throw_on_generation_mismatch( "erase", resource_generation, generation );

        // 1. Erase from resources_ vector
        resources_.erase( std::next( resources_.begin( ), resource_index ) );

        // 2. Update all redirection indices greater than the removed one
        for ( auto& redirection : redirection_map_ )
        {
            if ( redirection.index != NULL_HANDLE_INDEX && redirection.index > resource_index )
            {
                --redirection.index;
            }
        }

        // 3. Invalidate redirection entry
        resource_index = NULL_HANDLE_INDEX;
    }


    template <typename resource_t>
    void HandleTable<resource_t>::throw_on_invalid_table_index( std::string_view src, memory::handle_index_t const index ) const
    {
        if ( index >= redirection_map_.size( ) )
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

}


#endif //!HANDLETABLE_H
