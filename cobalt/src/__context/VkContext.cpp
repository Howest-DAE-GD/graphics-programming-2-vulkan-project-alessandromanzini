#include <__context/VkContext.h>

#include <log.h>
#include <__query/queue_family.h>

#include <set>


namespace cobalt
{
    VkContext::VkContext( ContextCreateInfo const& create_info )
    {
        if ( not create_info.window )
        {
            log::logerr<VkContext>( "VkContext", "window cannot be nullptr!" );
            return;
        }

        if ( create_info.validation_layers.has_value( ) )
        {
            validation_layers_ptr_ = std::make_unique<ValidationLayers>( *create_info.validation_layers );
            instance_bundle_ptr_   =
                    std::make_unique<InstanceBundle>( create_info.app_info, *create_info.window, validation_layers_ptr_.get( ) );
        }
        else
        {
            instance_bundle_ptr_ = std::make_unique<InstanceBundle>( create_info.app_info, *create_info.window );
        }
    }


    VkContext::~VkContext( )
    {
        if ( device_set_ptr_ )
        {
            device_set_ptr_.reset( );
        }

        if ( validation_layers_ptr_ )
        {
            validation_layers_ptr_->destroy_debug_messenger( *instance_bundle_ptr_ );
        }

        instance_bundle_ptr_.reset( );
    }


    InstanceBundle& VkContext::instance( ) const
    {
        return *instance_bundle_ptr_;
    }


    DeviceSet& VkContext::device( ) const
    {
        log::logerr<VkContext>( "device", "device set not initialized!", not device_set_ptr_ );
        return *device_set_ptr_;
    }


    void VkContext::create_device( DeviceFeatureFlags features )
    {
        device_set_ptr_ = std::make_unique<DeviceSet>( instance( ), features );
    }

}
