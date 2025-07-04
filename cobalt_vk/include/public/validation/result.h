#ifndef RESULT_H
#define RESULT_H

#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>


namespace cobalt_vk::validation
{
    void throw_on_bad_result( VkResult result, const std::string_view& errorMessage );
    void throw_on_bad_result( VkResult result, const std::string_view& errorMessage, const std::vector<VkResult>& ignore );

}


#endif //!RESULT_H
