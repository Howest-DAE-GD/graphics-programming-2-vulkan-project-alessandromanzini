#ifndef PHYSICALDEVICESELECTOR_H
#define PHYSICALDEVICESELECTOR_H

#include <cobalt_vk_internal/feature_command.h>
#include <__command/FeatureCommand.h>
#include <__enum/DeviceFeatureFlags.h>

#include <vulkan/vulkan_core.h>

#include <map>
#include <memory>
#include <vector>


namespace cobalt
{
    namespace exe
    {
        class FeatureCommand;
    }
    class InstanceBundle;
}

namespace cobalt::validation
{
    // +---------------------------+
    // | FEATURE COMMANDS          |
    // +---------------------------+
    using feature_map_t = std::map<DeviceFeatureFlags, std::unique_ptr<exe::FeatureCommand>>;


    [[nodiscard]] static feature_map_t make_feature_map( )
    {
        feature_map_t feat_map{};
        feat_map.emplace( DeviceFeatureFlags::SWAPCHAIN_EXT, std::make_unique<exe::SwapchainAdequateFeature>( ) );
        feat_map.emplace( DeviceFeatureFlags::SYNCHRONIZATION_2_EXT, std::make_unique<exe::Synchronization2Feature>( ) );
        feat_map.emplace( DeviceFeatureFlags::ANISOTROPIC_SAMPLING, std::make_unique<exe::AnisotropySamplingFeature>( ) );
        feat_map.emplace( DeviceFeatureFlags::DYNAMIC_RENDERING_EXT, std::make_unique<exe::DynamicRenderingFeature>( ) );
        feat_map.emplace( DeviceFeatureFlags::FAMILIES_INDICES_SUITABLE, std::make_unique<exe::FamilyIndicesFeature>( ) );
        feat_map.emplace( DeviceFeatureFlags::SHADER_IMAGE_ARRAY_NON_UNIFORM_INDEXING,
                          std::make_unique<exe::ShaderImgArrNonUniIdxFeature>( ) );
        return feat_map;
    }


    static const feature_map_t FEATURE_COMMAND_MAP = make_feature_map( );


    // +---------------------------+
    // | DEVICE SELECTOR           |
    // +---------------------------+
    class PhysicalDeviceSelector final
    {
    public:
        PhysicalDeviceSelector( InstanceBundle const&, DeviceFeatureFlags features );
        [[nodiscard]] bool select( VkPhysicalDevice device ) const;
        [[nodiscard]] exe::EnableData require( ) const;

    private:
        InstanceBundle const& instance_ref_;
        DeviceFeatureFlags const features_;

        static void get_extensions( VkPhysicalDevice device, std::vector<VkExtensionProperties>& dest );


    };

}


#endif //!PHYSICALDEVICESELECTOR_H
