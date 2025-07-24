#ifndef VK_FUNCTION_TRAITS_H
#define VK_FUNCTION_TRAITS_H

#include <vulkan/vulkan_core.h>


namespace cobalt::meta
{
    template <typename create_fn_t>
    struct vk_create_function_traits;


    template <typename _create_info_t, typename _resource_t>
    struct vk_create_function_traits<VkResult( * )( VkDevice, _create_info_t const*, VkAllocationCallbacks const*, _resource_t* )>
    {
        using create_info_t = _create_info_t;
        using resource_t    = _resource_t;
    };


    template <typename _create_info_t, typename _resource_t>
    struct vk_create_function_traits<VkResult( VkDevice, _create_info_t const*, VkAllocationCallbacks const*, _resource_t* )>
    {
        using create_info_t = _create_info_t;
        using resource_t    = _resource_t;
    };

}


#endif //!VK_FUNCTION_TRAITS_H
