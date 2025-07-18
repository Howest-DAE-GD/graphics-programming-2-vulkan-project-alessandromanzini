#ifndef RESOURCEHANDLE_H
#define RESOURCEHANDLE_H

#include "HandleTable.h"

#include <cassert>


namespace cobalt
{
    // todo: handle resource destruction in ResourceHandle for table.
    template <typename resource_t, typename table_resource_t = resource_t>
    class ResourceHandle final
    {
    public:
        explicit ResourceHandle( HandleTable<table_resource_t>&, memory::RedirectionInfo );
        ResourceHandle( ) = default;

        resource_t* get( );
        resource_t const* get( ) const;

        resource_t* operator->( );
        resource_t const* operator->( ) const;

        resource_t& operator*( );
        resource_t const& operator*( ) const;

    private:
        HandleTable<table_resource_t>* table_ptr_{ nullptr };
        memory::RedirectionInfo handle_info_{ NULL_HANDLE_INDEX };

    };


    template <typename resource_t, typename table_resource_t>
    ResourceHandle<resource_t, table_resource_t>::ResourceHandle( HandleTable<table_resource_t>& table,
                                                                  memory::RedirectionInfo const handle_info )
        : table_ptr_{ &table }
        , handle_info_{ handle_info } { }


    template <typename resource_t, typename table_resource_t>
    resource_t* ResourceHandle<resource_t, table_resource_t>::get( )
    {
        assert( table_ptr_ != nullptr && "ResourceHandle::get: table_ptr_ is null!" );
        return static_cast<resource_t*>( &table_ptr_->at( handle_info_ ) );
    }


    template <typename resource_t, typename table_resource_t>
    resource_t const* ResourceHandle<resource_t, table_resource_t>::get( ) const
    {
        assert( table_ptr_ != nullptr && "ResourceHandle::get: table_ptr_ is null!" );
        return static_cast<resource_t*>( &table_ptr_->at( handle_info_ ) );
    }


    template <typename resource_t, typename table_resource_t>
    resource_t* ResourceHandle<resource_t, table_resource_t>::operator->( )
    {
        return get( );
    }


    template <typename resource_t, typename table_resource_t>
    resource_t const* ResourceHandle<resource_t, table_resource_t>::operator->( ) const
    {
        return get( );
    }


    template <typename resource_t, typename table_resource_t>
    resource_t& ResourceHandle<resource_t, table_resource_t>::operator*( )
    {
        return *get( );
    }


    template <typename resource_t, typename table_resource_t>
    resource_t const& ResourceHandle<resource_t, table_resource_t>::operator*( ) const
    {
        return *get( );
    }

}


#endif //!RESOURCEHANDLE_H
