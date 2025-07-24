#ifndef RESULT_H
#define RESULT_H

#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>


namespace cobalt::validation
{
    void throw_on_bad_result( VkResult result, std::string_view const& error_message );
    void throw_on_bad_result( VkResult result, std::string_view const& error_message, std::vector<VkResult> const& ignore );

}


#endif //!RESULT_H
