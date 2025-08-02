#include <__context/VkContext.h>

#include <log.h>
#include <__query/queue_family.h>

#include <set>


namespace cobalt
{
    VkContext::VkContext( ContextWizard wizard )
    {
        // Retrieve the window and application info from the wizard
        auto const [window, app_info] = wizard.get<ContextCreateInfo>( );
        if ( not window )
        {
            log::logerr<VkContext>( "VkContext", "window cannot be nullptr!" );
            return;
        }

        // 1. Create validation layers if requested and ...
        // 2. ... create the instance bundle
        if ( wizard.has<ValidationLayers>( ) )
        {
            layers_ptr_          = std::make_unique<ValidationLayers>( wizard.steal<ValidationLayers>( ) );
            instance_bundle_ptr_ = std::make_unique<InstanceBundle>( app_info, *window, layers_ptr_.get( ) );
        }
        else
        {
            instance_bundle_ptr_ = std::make_unique<InstanceBundle>( app_info, *window );
        }

        // 3. Create the device set
        create_device( wizard.has<DeviceFeatureFlags>( ) ? wizard.feat<DeviceFeatureFlags>( ) : DeviceFeatureFlags::NONE );
    }


    VkContext::~VkContext( )
    {
        // 1. Destroy the device set
        device_set_ptr_.reset( );

        // 2. Destroy the validation layers if they were created
        if ( layers_ptr_ )
        {
            layers_ptr_->destroy_debug_messenger( *instance_bundle_ptr_ );
            layers_ptr_.reset( );
        }

        // 3. Destroy the instance bundle
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
