#include <CobaltVK.h>


namespace cobalt
{
    CobaltVK& CVK{ CobaltVK::get_instance( ) };


    CobaltVK::~CobaltVK( )
    {
        assert( deletion_queue_.is_flushed( ) && "CobaltVK::~CobaltVK: instance has not been cleared!" );
    }


    CobaltVK& CobaltVK::get_instance( )
    {
        static CobaltVK instance;
        return instance;
    }


    void CobaltVK::reset_instance( )
    {
        deletion_queue_.flush( );
    }


    VkContext& CobaltVK::get_vk_instance( ) const
    {
        return *vk_instance_ptr_;
    }

}
