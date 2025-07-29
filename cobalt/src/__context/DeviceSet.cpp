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


    VkQueue DeviceSet::graphics_queue( ) const
    {
        return graphics_queue_;
    }


    VkQueue DeviceSet::present_queue( ) const
    {
        return present_queue_;
    }


    void DeviceSet::wait_idle( ) const
    {
        vkDeviceWaitIdle( device_ );
    }


    void DeviceSet::pick_physical_device( )
    {
        // The physical device gets implicitly destroyed when we destroy the instance.
        uint32_t device_count{ 0 };
        vkEnumeratePhysicalDevices( instance_ref_.instance( ), &device_count, nullptr );

        if ( device_count == 0 )
        {
            throw std::runtime_error( "Failed to find GPUs with Vulkan support!" );
        }

        // Fetch the physical devices
        std::vector<VkPhysicalDevice> devices( device_count );
        vkEnumeratePhysicalDevices( instance_ref_.instance( ), &device_count, devices.data( ) );

        // Check if any of the physical devices meet the requirements
        for ( validation::PhysicalDeviceSelector const selector{ instance_ref_, feature_flags_ };
              auto const& device : devices )
        {
            if ( auto [adequate, extensions] = selector.select( device ); adequate )
            {
                physical_device_ = device;
                extensions_      = std::move( extensions );
                break;
            }
        }

        if ( physical_device_ == VK_NULL_HANDLE )
        {
            throw std::runtime_error( "Failed to find a suitable GPU!" );
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
        std::set unique_queue_families{ graphics_family.value( ), present_family.value( ) };

        // Vulkan lets you assign priorities to queues to influence the scheduling of command buffer execution using
        // floating point numbers between 0.0 and 1.0.
        constexpr float queue_priority{ 1.0f };
        std::vector<VkDeviceQueueCreateInfo> queue_create_infos{};
        for ( uint32_t queue_family : unique_queue_families )
        {
            VkDeviceQueueCreateInfo queue_create_info{};
            queue_create_info.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = queue_family;
            queue_create_info.queueCount       = 1;
            queue_create_info.pQueuePriorities = &queue_priority;
            queue_create_infos.push_back( queue_create_info );
        }

        // The next information to specify is the set of device features that we'll be using.
        // These are the features that we queried support for with vkGetPhysicalDeviceFeatures.
        VkPhysicalDeviceFeatures device_features{};
        device_features.samplerAnisotropy = VK_TRUE;

        // Set additional Vulkan 1+ features that we want to use.
        VkPhysicalDeviceVulkan13Features device_features13{};
        device_features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        device_features13.pNext = nullptr;
        if ( any( feature_flags_ & DeviceFeatureFlags::DYNAMIC_RENDERING_EXT ) )
        {
            device_features13.dynamicRendering = VK_TRUE;
        }

        // Create the logical device
        VkDeviceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        create_info.queueCreateInfoCount = static_cast<uint32_t>( queue_create_infos.size( ) );
        create_info.pQueueCreateInfos    = queue_create_infos.data( );

        create_info.pEnabledFeatures = &device_features;
        create_info.pNext            = &device_features13;

        // The remainder of the information bears a resemblance to the VkInstanceCreateInfo struct and requires you
        // to specify extensions and validation layers. The difference is that these are device specific this time.
#ifdef __APPLE__
        // Device specific extensions are necessary on macOS to allow MoltenVK to function properly.
        extensions_.push_back( "VK_KHR_portability_subset" );
#endif

        create_info.enabledExtensionCount   = static_cast<uint32_t>( extensions_.size( ) );
        create_info.ppEnabledExtensionNames = extensions_.data( );

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
            create_info.enabledLayerCount = 0;
        }

        // Create instance and validate the result
        validation::throw_on_bad_result(
            vkCreateDevice( physical_device_, &create_info, nullptr, &device_ ), "Failed to create logical device!" );

        // Retrieve the handles for the queues
        vkGetDeviceQueue( device_, graphics_family.value( ), 0, &graphics_queue_ );
        vkGetDeviceQueue( device_, present_family.value( ), 0, &present_queue_ );
    }

}
