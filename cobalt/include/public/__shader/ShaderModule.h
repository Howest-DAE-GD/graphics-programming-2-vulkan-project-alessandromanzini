#ifndef SHADERMODULES_H
#define SHADERMODULES_H

#include <filesystem>
#include <vector>

#include <vulkan/vulkan_core.h>


namespace cobalt
{
    class DeviceSet;
}

namespace cobalt::shader
{
    class ShaderModule final
    {
    public:
        ShaderModule( DeviceSet const& device, std::filesystem::path const& path, VkShaderStageFlagBits stage );
        ~ShaderModule( ) noexcept;

        ShaderModule( ShaderModule&& ) noexcept;
        ShaderModule( const ShaderModule& )                = delete;
        ShaderModule& operator=( const ShaderModule& )     = delete;
        ShaderModule& operator=( ShaderModule&& ) noexcept = delete;

        [[nodiscard]] VkShaderModule handle( ) const noexcept;
        [[nodiscard]] VkShaderStageFlagBits stage( ) const noexcept;

    private:
        DeviceSet const& device_ref_;
        VkShaderStageFlagBits const stage_;
        VkShaderModule shader_module_{ VK_NULL_HANDLE };

    };

}


#endif //!SHADERMODULES_H
