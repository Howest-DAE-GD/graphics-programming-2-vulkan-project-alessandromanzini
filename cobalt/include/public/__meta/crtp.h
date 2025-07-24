#ifndef CRTP_H
#define CRTP_H


namespace meta
{
    template <typename derived_t>
    class crtp
    {
    protected:
        crtp( ) = default;

        [[nodiscard]] derived_t* derived_cast( );
        [[nodiscard]] derived_t const* derived_cast( ) const;

    };


    template <typename derived_t>
    derived_t* crtp<derived_t>::derived_cast( )
    {
        return static_cast<derived_t*>( this );
    }


    template <typename derived_t>
    derived_t const* crtp<derived_t>::derived_cast( ) const
    {
        return static_cast<derived_t const*>( this );
    }

}


#endif //!CRTP_H
