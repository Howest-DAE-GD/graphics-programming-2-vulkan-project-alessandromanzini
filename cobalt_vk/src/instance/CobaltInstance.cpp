#include <instance/CobaltInstance.h>


namespace cobalt_vk
{
    CobaltInstance& CVK_INSTANCE{ CobaltInstance::get_instance( ) };


    CobaltInstance::~CobaltInstance( ) noexcept
    {
        assert( not window_ptr_ && not vk_instance_ptr_ && "CobaltInstance::~CobaltInstance: instance has not been cleared!" );
    }


    CobaltInstance& CobaltInstance::get_instance( )
    {
        static CobaltInstance instance;
        return instance;
    }

    Window& CobaltInstance::get_window( ) const
    {
        return *window_ptr_;
    }


    VulkanInstance& CobaltInstance::get_vk_instance( ) const
    {
        return *vk_instance_ptr_;
    }


    void CobaltInstance::register_window( std::unique_ptr<Window>&& window )
    {
        window_ptr_ = std::move( window );
    }


    void CobaltInstance::register_vk_instance( std::unique_ptr<VulkanInstance>&& instance )
    {
        vk_instance_ptr_ = std::move( instance );
    }


    void CobaltInstance::clear_cobalt_instance( )
    {
        deletion_queue_.flush( );
        vk_instance_ptr_ = nullptr;
        window_ptr_      = nullptr;
    }

}
