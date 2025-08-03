//
// Created by Alessandro Manzini on 25/03/25.
//

#ifndef SINGLETIMECOMMAND_H
#define SINGLETIMECOMMAND_H

#include <vulkan/vulkan.h>

namespace cobalt
{
    [[nodiscard]] inline VkCommandBuffer begin_single_time_commands( VkDevice const device,
                                                                     VkCommandPool const commandPool )
    {
        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool        = commandPool;
        alloc_info.commandBufferCount = 1;

        VkCommandBuffer command_buffer;
        vkAllocateCommandBuffers( device, &alloc_info, &command_buffer );

        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer( command_buffer, &begin_info );

        return command_buffer;
    }


    inline void end_single_time_commands( VkDevice const device, VkCommandPool const command_pool,
                                          VkCommandBuffer const command_buffer, VkQueue const graphics_queue )
    {
        vkEndCommandBuffer( command_buffer );

        VkSubmitInfo submit_info{};
        submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers    = &command_buffer;

        vkQueueSubmit( graphics_queue, 1, &submit_info, VK_NULL_HANDLE );
        vkQueueWaitIdle( graphics_queue );

        vkFreeCommandBuffers( device, command_pool, 1, &command_buffer );
    }


    inline void end_single_time_commands2( VkDevice const device, VkCommandPool const command_pool,
                                          VkCommandBuffer const command_buffer, VkQueue const graphics_queue )
    {
        vkEndCommandBuffer( command_buffer );

        VkCommandBufferSubmitInfo command_buffer_info{};
        command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        command_buffer_info.commandBuffer = command_buffer;

        VkSubmitInfo2 submit_info{};
        submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submit_info.commandBufferInfoCount = 1;
        submit_info.pCommandBufferInfos = &command_buffer_info;

        vkQueueSubmit2( graphics_queue, 1, &submit_info, VK_NULL_HANDLE );
        vkQueueWaitIdle( graphics_queue );

        vkFreeCommandBuffers( device, command_pool, 1, &command_buffer );
    }
}

#endif //SINGLETIMECOMMAND_H
