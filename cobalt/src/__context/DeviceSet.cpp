#include <__context/DeviceSet.h>

#include <log.h>
#include <__context/InstanceBundle.h>
#include <__context/ValidationLayers.h>
#include <__query/queue_family.h>
#include <__validation/result.h>
#include <__validation/selector/PhysicalDeviceSelector.h>

#include <set>


namespace cobalt
{
    // +---------------------------+
    // | DEVICE SET                |
    // +---------------------------+
    DeviceSet::DeviceSet( InstanceBundle const& instance, DeviceFeatureFlags const features,
                          ValidationLayers const* validation_layers )
        : instance_ref_{ instance }
        , feature_flags_{ features | DeviceFeatureFlags::FAMILIES_INDICES_SUITABLE }
    {
        pick_physical_device( );
        create_logical_device( validation_layers );
    }


    DeviceSet::~DeviceSet( )
    {
        vkDestroyDevice( device_, nullptr );
    }


    VkDevice DeviceSet::logical( ) const
    {
        return device_;
    }


    VkPhysicalDevice DeviceSet::physical( ) const
    {
        return physical_device_;
    }


    Queue& DeviceSet::graphics_queue( ) const
    {
        return *graphics_queue_ptr_;
    }


    Queue& DeviceSet::present_queue( ) const
    {
        return *present_queue_ptr_;
    }


    bool DeviceSet::has_feature( DeviceFeatureFlags const feature ) const
    {
        return any( feature_flags_ & feature );
    }


    uint32_t DeviceSet::device_index( ) const
    {
        return device_index_;
    }


    void DeviceSet::wait_idle( ) const
    {
        vkDeviceWaitIdle( device_ );
    }


    void DeviceSet::wait_for_fence( VkFence const fence, uint64_t const timeout ) const
    {
        // todo: perhaps make sure fence is valid and result is VK_SUCCESS?
        // The VK_TRUE we pass here indicates that we want to wait for all fences, but in the case of a single one it
        // doesn't matter.
        vkWaitForFences( device_, 1, &fence, VK_TRUE, timeout );
    }


    void DeviceSet::reset_fence( VkFence const fence ) const
    {
        // todo: perhaps make sure fence is valid and result is VK_SUCCESS?
        vkResetFences( device_, 1, &fence );
    }


    void DeviceSet::pick_physical_device( )
    {
        // The physical device gets implicitly destroyed when we destroy the instance.
        uint32_t device_count{ 0u };
        vkEnumeratePhysicalDevices( instance_ref_.instance( ), &device_count, nullptr );

        if ( device_count == 0u )
        {
            log::logerr<DeviceSet>( "pick_physical_device", "failed to find GPUs with Vulkan support!" );
        }

        // fetch all physical devices
        std::vector<VkPhysicalDevice> devices( device_count );
        vkEnumeratePhysicalDevices( instance_ref_.instance( ), &device_count, devices.data( ) );

        // check if any of the physical devices meet the requirements
        for ( validation::PhysicalDeviceSelector const selector{ instance_ref_, feature_flags_ };
              VkPhysicalDevice const device : devices )
        {
            if ( selector.select( device ) )
            {
                physical_device_ = device;
                device_index_    = 0u;
                break;
            }
        }

        // if we didn't find a suitable physical device, we log an error.
        if ( physical_device_ == VK_NULL_HANDLE )
        {
            log::logerr<DeviceSet>( "pick_physical_device", "failed to find a suitable GPU!" );
        }
    }


    // todo: ask for options to be passed in, like queue priorities, etc.
    void DeviceSet::create_logical_device( ValidationLayers const* validation_layers )
    {
        if ( device_ != VK_NULL_HANDLE )
        {
            log::logerr<DeviceSet>( "create_logical_device", "logical device already created." );
            return;
        }

        // This structure describes the number of queues we want for a single queue family. Right now we're only
        // interested in a queue with graphics capabilities.
        auto const [graphics_family, present_family] = query::find_queue_families( physical_device_, instance_ref_ );

        // Get the unique queue families to load once.
        std::set const unique_queue_families{ graphics_family.value( ), present_family.value( ) };

        // Vulkan lets you assign priorities to queues to influence the scheduling of command buffer execution using
        // floating point numbers between 0.f and 1.f
        constexpr float queue_priority{ 1.f };
        std::vector<VkDeviceQueueCreateInfo> queue_create_infos{};
        for ( uint32_t const queue_family : unique_queue_families )
        {
            queue_create_infos.emplace_back( VkDeviceQueueCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = queue_family,
                .queueCount = 1u,
                .pQueuePriorities = &queue_priority
            } );
        }

        // Extract the required features and extensions
        exe::EnableData device_features = validation::PhysicalDeviceSelector{ instance_ref_, feature_flags_ }.require( );
        device_features.map_data( );

        // Create the logical device
        VkDeviceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        create_info.queueCreateInfoCount = static_cast<uint32_t>( queue_create_infos.size( ) );
        create_info.pQueueCreateInfos    = queue_create_infos.data( );
        create_info.pNext                = &device_features.features;

        // The remainder of the information bears a resemblance to the VkInstanceCreateInfo struct and requires you
        // to specify extensions and validation layers. The difference is that these are device specific this time.
#ifdef __APPLE__
        // Device specific extensions are necessary on macOS to allow MoltenVK to function properly.
        device_features.extensions.push_back( "VK_KHR_portability_subset" );
#endif

        create_info.enabledExtensionCount   = static_cast<uint32_t>( device_features.extensions.size( ) );
        create_info.ppEnabledExtensionNames = device_features.extensions.data( );

        // Previous implementations of Vulkan made a distinction between instance and device specific validation layers,
        // but this is no longer the case. We're still doing it for backwards compatibility.
        if ( validation_layers )
        {
            validation_layers->populate_create_info( create_info );
            create_info.enabledLayerCount   = static_cast<uint32_t>( validation_layers->get_validation_layers( ).size( ) );
            create_info.ppEnabledLayerNames = validation_layers->get_validation_layers( ).data( );
        }
        else
        {
            create_info.enabledLayerCount = 0u;
        }

        // Create instance and validate the result
        validation::throw_on_bad_result(
            vkCreateDevice( physical_device_, &create_info, nullptr, &device_ ), "Failed to create logical device!" );

        // Now we can create the queues by extraction
        graphics_queue_ptr_ = std::make_unique<Queue>( *this, graphics_family.value( ), 0 );
        present_queue_ptr_  = std::make_unique<Queue>( *this, present_family.value( ), 0 );
    }

}
