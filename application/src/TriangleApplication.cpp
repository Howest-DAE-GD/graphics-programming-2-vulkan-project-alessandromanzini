#include <TriangleApplication.h>

// +---------------------------+
// | STANDARD HEADERS          |
// +---------------------------+
#include <chrono>
#include <functional>
#include <iostream>
#include <set>
#include <stdexcept>

// +---------------------------+
// | PROJECT HEADERS           |
// +---------------------------+
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <xos/filesystem.h>
#include <xos/info.h>

#include <filesystem>

#include <CobaltVK.h>
#include <__model/AssimpModelLoader.h>

#include <SingleTimeCommand.h>
#include <UniformBufferObject.h>
#include <__enum/ValidationFlags.h>
#include <__image/ImageView.h>
#include <__shader/ShaderModule.h>
#include <__swapchain/Swapchain.h>
#include <__validation/result.h>

#include "../../cobalt/include/private/__query/queue_family.h"


using namespace cobalt;


// +---------------------------+
// | PUBLIC                    |
// +---------------------------+
TriangleApplication::TriangleApplication( )
{
    // 1. Create Window
    window_ = CVK.create_resource<Window>( WIDTH_, HEIGHT_, "Vulkan App" );

    // 2. Register VK Instance
    constexpr VkApplicationInfo app_info{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Hello Viking",
        .applicationVersion = VK_MAKE_VERSION( 1, 0, 0 ),
        .pEngineName = "Cobalt",
        .engineVersion = VK_MAKE_VERSION( 1, 0, 0 ),
        .apiVersion = VK_API_VERSION_1_3
    };
    context_ = CVK.create_resource<VkContext>(
        ContextWizard{ { window_.get( ), app_info } }
        .with<DeviceFeatureFlags>(
            DeviceFeatureFlags::SWAPCHAIN_EXT | DeviceFeatureFlags::ANISOTROPIC_SAMPLING |
            DeviceFeatureFlags::DYNAMIC_RENDERING_EXT | DeviceFeatureFlags::SYNCHRONIZATION_2_EXT )
        .with<ValidationLayers>( ValidationFlags::KHRONOS_VALIDATION, debug_callback )
    );

    // 3. Set the proper root directory to find shader modules and textures.
    configure_relative_path( );

    // 4. Swapchain
    swapchain_ptr_ = std::make_unique<Swapchain>(
        SwapchainWizard{
            *context_,
            *window_,
            SwapchainCreateInfo{
                .image_count = 3,
                .present_mode = VK_PRESENT_MODE_MAILBOX_KHR,
                .surface_format = {
                    VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
                }
            }
        } );
}


void TriangleApplication::run( )
{
    init_vk( );

    bool keep_running{ true };
    while ( keep_running )
    {
        keep_running = not window_->should_close( );
        main_loop( );
    }
    // All the operations in drawFrame are asynchronous. When we exit the loop, drawing and presentation operations may
    // still be going on. We solve it by waiting for the logical device to finish operations before exiting.
    context_->device( ).wait_idle( );

    cleanup( );
}


// +---------------------------+
// | PRIVATE                   |
// +---------------------------+
void TriangleApplication::vk_create_descriptor_set_layout( )
{
    descriptor_set_layout_ptr_ = std::make_unique<DescriptorSetLayout>(
        context_->device( ),
        std::vector<DescriptorSetLayout::layout_binding_pair_t>{
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT }
        }
    );
}


void TriangleApplication::vk_create_graphics_pipeline( )
{
    // The compilation and linking of the SPIR-V bytecode to machine code for execution by the GPU doesn't happen until the
    // graphics pipeline is created. That means that we're allowed to destroy the shader modules again as soon as pipeline
    // creation is finished. Therefore, we create them as local temporary variables.
    shader::ShaderModule const vert_shader{ context_->device( ), "shaders/shader.vert.spv" };
    shader::ShaderModule const frag_shader{ context_->device( ), "shaders/shader.frag.spv" };

    // We assign the shaders to specific pipelines stages through the shaderStages member of the VkPipelineShaderStageCreateInfo
    std::vector shader_stages{
        VkPipelineShaderStageCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vert_shader.handle( ),
            .pName = "main"
        },
        VkPipelineShaderStageCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = frag_shader.handle( ),
            .pName = "main"
        }
    };

    // The VkPipelineInputAssemblyStateCreateInfo struct describes two things: what kind of geometry will be drawn from
    // the vertices and if primitive restart should be enabled.
    constexpr VkPipelineInputAssemblyStateCreateInfo input_assembly{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    // The rasterizer takes the geometry that is shaped by the vertices from the vertex shader and turns it into
    // fragments to be colored by the fragment shader. It also performs depth testing, face culling and the scissor
    // test.
    constexpr VkPipelineRasterizationStateCreateInfo rasterization{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE
    };

    // The VkPipelineMultisampleStateCreateInfo struct configures multisampling, which is one of the ways to perform
    // antialiasing.
    constexpr VkPipelineMultisampleStateCreateInfo multisampling{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .minSampleShading = 1.0f,          // Optional
        .pSampleMask = nullptr,            // Optional
        .alphaToCoverageEnable = VK_FALSE, // Optional
        .alphaToOneEnable = VK_FALSE,      // Optional
    };

    // After a fragment shader has returned a color, it needs to be combined with the color that is already in the
    // framebuffer. This transformation is known as color blending and there are two ways to do it:
    // 1. Mix the old and new value to produce a final color.
    // 2. Combine the old and new value using a bitwise operation.
    constexpr VkPipelineColorBlendAttachmentState color_blend_attachment{
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                          VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,  // Optional
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional
        .colorBlendOp = VK_BLEND_OP_ADD,             // Optional
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,  // Optional
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional
        .alphaBlendOp = VK_BLEND_OP_ADD              // Optional
    };

    // Depth stencil creation
    constexpr VkPipelineDepthStencilStateCreateInfo depth_stencil{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .minDepthBounds = 0.0f, // Optional
        .maxDepthBounds = 1.0f, // Optional
        .stencilTestEnable = VK_FALSE,
        .front = {}, // Optional
        .back = {}   // Optional
    };

    graphics_pipeline_ptr_ = std::make_unique<GraphicsPipeline>(
        context_->device( ),
        GraphicsPipelineCreateInfo{
            .shader_stages = std::move( shader_stages ),
            .descriptor_set_layouts = { descriptor_set_layout_ptr_->handle( ) },
            .binding_description = { Vertex::get_binding_description( ), Vertex::get_attribute_descriptions( ) },
            .input_assembly = input_assembly,
            .rasterization = rasterization,
            .multisampling = multisampling,
            .color_blend_attachment = color_blend_attachment,
            .depth_stencil = depth_stencil,
            .swapchain_image_format = swapchain_ptr_->image_format( ),
            .depth_image_format = swapchain_ptr_->depth_image( ).format( ),
        } );
}


void TriangleApplication::vk_copy_buffer( Buffer const& src, Buffer const& dst )
{
    auto const& cmd_buffer = command_pool_ptr_->acquire( VK_COMMAND_BUFFER_LEVEL_PRIMARY );
    cmd_buffer.reset( 0 );

    auto buffer_op = cmd_buffer.command_operator( 0 );
    buffer_op.copy_buffer( src, dst );
    buffer_op.end_recording( );

    auto const command_buffer_info = cmd_buffer.make_submit_info( );

    VkSubmitInfo2 submit_info{};
    submit_info.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos    = &command_buffer_info;

    context_->device( ).graphics_queue( ).submit_and_wait( submit_info );

    cmd_buffer.unlock( );
}


void TriangleApplication::load_model( )
{
    model_ = CVK.create_resource<Model>( context_->device( ) );
    loader::AssimpModelLoader const loader{ MODEL_PATH_ };
    loader.load( *model_ );
}


void TriangleApplication::vk_create_texture_image( )
{
    int tex_width, tex_height, tex_channels;
    stbi_uc* pixels = stbi_load( "resources/viking_room.png", &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha );
    VkDeviceSize const image_size = tex_width * tex_height * 4;

    if ( not pixels )
    {
        throw std::runtime_error( "Failed to load texture image!" );
    }

    Buffer staging_buffer{
        context_->device( ), image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };

    staging_buffer.map_memory( );
    memcpy( staging_buffer.data( ), pixels, staging_buffer.memory_size( ) );
    staging_buffer.unmap_memory( );

    // Cleanup original pixel data
    stbi_image_free( pixels );

    texture_image_ptr_ = std::make_unique<Image>(
        context_->device( ),
        ImageCreateInfo{
            .extent = { static_cast<uint32_t>( tex_width ), static_cast<uint32_t>( tex_height ) },
            .format = VK_FORMAT_R8G8B8A8_SRGB,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT
        } );
    vk_transition_image_layout( *texture_image_ptr_, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );
    vk_copy_buffer_to_image( staging_buffer, *texture_image_ptr_, static_cast<uint32_t>( tex_width ),
                             static_cast<uint32_t>( tex_height ) );
    vk_transition_image_layout( *texture_image_ptr_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
}


void TriangleApplication::vk_create_texture_sampler( )
{
    VkSamplerCreateInfo sampler_info{};

    // The magFilter and minFilter fields specify how to interpolate texels that are magnified or minified.
    sampler_info.sType     = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;

    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties( context_->device( ).physical( ), &properties );

    sampler_info.anisotropyEnable = VK_TRUE;
    sampler_info.maxAnisotropy    = properties.limits.maxSamplerAnisotropy;

    // samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable           = VK_FALSE;
    sampler_info.compareOp               = VK_COMPARE_OP_ALWAYS;

    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod     = 0.0f;
    sampler_info.maxLod     = 0.0f;

    validation::throw_on_bad_result(
        vkCreateSampler( context_->device( ).logical( ), &sampler_info, nullptr, &texture_sampler_ ),
        "Failed to create texture sampler!" );
}


void TriangleApplication::vk_transition_image_layout( Image const& image, VkImageLayout const old_layout,
                                                      VkImageLayout const new_layout ) const
{
    auto const& buffer = command_pool_ptr_->acquire( VK_COMMAND_BUFFER_LEVEL_PRIMARY );

    buffer.reset( 0 );
    auto buffer_op = buffer.command_operator( 0 );

    VkImageMemoryBarrier2 barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
        .dstStageMask = VK_PIPELINE_STAGE_2_NONE,
        .srcAccessMask = VK_ACCESS_2_NONE,
        .dstAccessMask = VK_ACCESS_2_NONE,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image.handle( ),
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    if ( old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL )
    {
        barrier.srcAccessMask = VK_ACCESS_2_NONE;
        barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;

        barrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    }
    else if ( old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL )
    {
        barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;

        barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    }
    else if ( old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL )
    {
        barrier.srcAccessMask = VK_ACCESS_2_NONE;
        barrier.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        barrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
    }
    else
    {
        throw std::invalid_argument( "Unsupported layout transition!" );
    }

    VkDependencyInfo const dep_info{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier,
    };

    buffer_op.insert_barrier( dep_info );
    buffer_op.end_recording( );

    auto const command_buffer_info = buffer.make_submit_info( );

    VkSubmitInfo2 submit_info{};
    submit_info.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos    = &command_buffer_info;

    context_->device( ).graphics_queue( ).submit_and_wait( submit_info );

    buffer.unlock( );
}


void TriangleApplication::vk_copy_buffer_to_image( Buffer const& src, Image const& dst, uint32_t const width,
                                                   uint32_t const height ) const
{
    auto const& cmd_buffer = command_pool_ptr_->acquire( VK_COMMAND_BUFFER_LEVEL_PRIMARY );

    cmd_buffer.reset( 0 );
    auto buffer_op = cmd_buffer.command_operator( 0 );

    VkBufferImageCopy const region{
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,

        .imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .imageSubresource.mipLevel = 0,
        .imageSubresource.baseArrayLayer = 0,
        .imageSubresource.layerCount = 1,

        .imageOffset = { 0, 0, 0 },
        .imageExtent = { width, height, 1 }
    };
    buffer_op.copy_buffer_to_image( src, dst, region );
    buffer_op.end_recording( );

    auto const command_buffer_info = cmd_buffer.make_submit_info( );

    VkSubmitInfo2 const submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &command_buffer_info
    };

    context_->device( ).graphics_queue( ).submit_and_wait( submit_info );

    cmd_buffer.unlock( );
}


void TriangleApplication::vk_create_model_buffers( )
{
    // index buffer
    {
        VkDeviceSize const buffer_size = sizeof( model_->indices( )[0] ) * model_->indices( ).size( );

        Buffer staging_buffer{
            context_->device( ), buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };

        staging_buffer.map_memory( );
        memcpy( staging_buffer.data( ), model_->indices( ).data( ), staging_buffer.memory_size( ) );
        staging_buffer.unmap_memory( );

        index_buffer_ptr_ = std::make_unique<Buffer>( context_->device( ), buffer_size,
                                                      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

        vk_copy_buffer( staging_buffer, *index_buffer_ptr_ );
    }

    // vertex buffer
    {
        VkDeviceSize const buffer_size = sizeof( model_->vertices( )[0] ) * model_->vertices( ).size( );

        Buffer staging_buffer{
            context_->device( ), buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };

        staging_buffer.map_memory( );
        memcpy( staging_buffer.data( ), model_->vertices( ).data( ), staging_buffer.memory_size( ) );
        staging_buffer.unmap_memory( );

        vertex_buffer_ptr_ = std::make_unique<Buffer>( context_->device( ), buffer_size,
                                                       VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

        vk_copy_buffer( staging_buffer, *vertex_buffer_ptr_ );
    }
}


void TriangleApplication::vk_create_uniform_buffers( )
{
    for ( size_t i{}; i < MAX_FRAMES_IN_FLIGHT_; i++ )
    {
        auto& buffer = uniform_buffers_.emplace_back(
            context_->device( ), sizeof( UniformBufferObject ), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

        // The buffer stays mapped to this pointer for the application's whole lifetime. This technique is called
        // "persistent mapping" and works on all Vulkan implementations.
        buffer.map_memory( );
    }
}


void TriangleApplication::vk_create_descriptor_pool( )
{
    std::array<VkDescriptorPoolSize, 2> pool_sizes{};
    pool_sizes[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = static_cast<uint32_t>( MAX_FRAMES_IN_FLIGHT_ );
    pool_sizes[1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[1].descriptorCount = static_cast<uint32_t>( MAX_FRAMES_IN_FLIGHT_ );

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = static_cast<uint32_t>( pool_sizes.size( ) );
    pool_info.pPoolSizes    = pool_sizes.data( );

    pool_info.maxSets = static_cast<uint32_t>( MAX_FRAMES_IN_FLIGHT_ );

    validation::throw_on_bad_result(
        vkCreateDescriptorPool( context_->device( ).logical( ), &pool_info, nullptr, &descriptor_pool_ ),
        "Failed to create descriptor pool!" );
}


void TriangleApplication::vk_create_descriptor_sets( )
{
    std::vector const layouts( MAX_FRAMES_IN_FLIGHT_, descriptor_set_layout_ptr_->handle( ) );
    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool     = descriptor_pool_;
    alloc_info.descriptorSetCount = static_cast<uint32_t>( MAX_FRAMES_IN_FLIGHT_ );
    alloc_info.pSetLayouts        = layouts.data( );

    descriptor_sets_.resize( MAX_FRAMES_IN_FLIGHT_ );
    validation::throw_on_bad_result(
        vkAllocateDescriptorSets( context_->device( ).logical( ), &alloc_info, descriptor_sets_.data( ) ),
        "Failed to allocate descriptor sets!" );

    for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT_; i++ )
    {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = uniform_buffers_[i].handle( );
        buffer_info.offset = 0;
        buffer_info.range  = sizeof( UniformBufferObject );

        VkDescriptorImageInfo image_info{};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView   = texture_image_ptr_->view( ).handle( );
        image_info.sampler     = texture_sampler_;

        std::array<VkWriteDescriptorSet, 2> descriptor_writes{};
        descriptor_writes[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet          = descriptor_sets_[i];
        descriptor_writes[0].dstBinding      = 0;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].pBufferInfo     = &buffer_info;

        descriptor_writes[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[1].dstSet          = descriptor_sets_[i];
        descriptor_writes[1].dstBinding      = 1;
        descriptor_writes[1].dstArrayElement = 0;
        descriptor_writes[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[1].descriptorCount = 1;
        descriptor_writes[1].pImageInfo      = &image_info;

        vkUpdateDescriptorSets( context_->device( ).logical( ), static_cast<uint32_t>( descriptor_writes.size( ) ),
                                descriptor_writes.data( ), 0, nullptr );
    }
}


void TriangleApplication::vk_create_command_pool( )
{
    command_pool_ptr_ = std::make_unique<CommandPool>( *context_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT );
}


void TriangleApplication::vk_create_command_buffers( )
{
    for ( size_t i{}; i < MAX_FRAMES_IN_FLIGHT_; i++ )
    {
        command_buffers_[i] = &command_pool_ptr_->acquire( VK_COMMAND_BUFFER_LEVEL_PRIMARY );
    }
}


void TriangleApplication::vk_create_sync_objects( )
{
    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    // The VK_FENCE_CREATE_SIGNALED_BIT flag specifies that the fence object is created in the signaled state.
    // This allows us to skip the first render frame, or we would otherwise be stuck waiting at the start.
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for ( size_t i{}; i < MAX_FRAMES_IN_FLIGHT_; i++ )
    {
        validation::throw_on_bad_result(
            vkCreateSemaphore( context_->device( ).logical( ), &semaphore_info, nullptr, &image_available_semaphores_[i] ),
            "Failed to create image semaphore!" );
        validation::throw_on_bad_result(
            vkCreateFence( context_->device( ).logical( ), &fence_info, nullptr, &in_flight_fences_[i] ),
            "Failed to create fence!" );
    }
    for ( size_t i{}; i < 3; i++ )
    {
        validation::throw_on_bad_result(
            vkCreateSemaphore( context_->device( ).logical( ), &semaphore_info, nullptr, &render_finished_semaphores_[i] ),
            "Failed to create render semaphore!" );
    }
}


VkBool32 TriangleApplication::debug_callback( VkDebugUtilsMessageSeverityFlagBitsEXT const message_severity,
                                              VkDebugUtilsMessageTypeFlagsEXT /* messageType */,
                                              VkDebugUtilsMessengerCallbackDataEXT const* p_callback_data,
                                              void* /* pUserData */ )
{
    std::string severity_color{};

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    constexpr std::string_view reset_color{ "\e[0m" };
#pragma GCC diagnostic pop
#else
#pragma warning(push, 0)
    constexpr std::string_view reset_color{ "\033[0m" };
#pragma warning(pop)
#endif

    switch ( message_severity )
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            severity_color = "\033[34m";
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            severity_color = "\033[37m";
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            severity_color = "\033[33m";
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            severity_color = "\033[31m";
            break;
        default:
            severity_color = "\033[36m";
            break;
    }

    std::cerr << severity_color << "validation layer: " << p_callback_data->pMessage << reset_color << std::endl;
    return VK_FALSE;
}


void TriangleApplication::record_command_buffer( CommandBuffer& buffer, uint32_t const image_index ) const
{
    buffer.reset( );
    auto const command_op = buffer.command_operator( 0 );

    // Pre-rendering barrier
    {
        auto const swapchain_image = swapchain_ptr_->image_at( image_index ).handle( );
        VkImageMemoryBarrier2 pre_barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .srcAccessMask = VK_ACCESS_2_NONE,
            .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = swapchain_image,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };
        command_op.insert_barrier( VkDependencyInfo{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &pre_barrier
        } );

        // Post rendering barrier
        VkImageMemoryBarrier2 post_barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
            .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_2_NONE,
            .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = swapchain_image,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };
        command_op.insert_barrier( VkDependencyInfo{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &post_barrier
        } );
    }

    // Now we specify the dynamic rendering information, which allows us to render directly to the swapchain images without
    // setting up a pipeline with a render pass.
    {
        VkRenderingAttachmentInfo color_attachment_info{};
        color_attachment_info.sType            = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        color_attachment_info.imageView        = swapchain_ptr_->image_at( image_index ).view( ).handle( );
        color_attachment_info.imageLayout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color_attachment_info.loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment_info.storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment_info.clearValue.color = VkClearColorValue{ { 0.0f, 0.0f, 0.0f, 1.0f } };

        VkRenderingAttachmentInfo depth_attachment_info{};
        depth_attachment_info.sType                         = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depth_attachment_info.imageView                     = swapchain_ptr_->depth_image( ).view( ).handle( );
        depth_attachment_info.imageLayout                   = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depth_attachment_info.loadOp                        = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment_info.storeOp                       = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment_info.clearValue.depthStencil.depth = 1.0f;

        VkRenderingInfo render_info{};
        render_info.sType                = VK_STRUCTURE_TYPE_RENDERING_INFO;
        render_info.renderArea.extent    = swapchain_ptr_->extent( );
        render_info.renderArea.offset    = { 0, 0 };
        render_info.layerCount           = 1;
        render_info.colorAttachmentCount = 1;
        render_info.pColorAttachments    = &color_attachment_info;
        render_info.pDepthAttachment     = &depth_attachment_info;

        // We can now begin the rendering process by calling vkCmdBeginRendering, which starts recording commands
        command_op.begin_rendering( render_info );
    }

    // We can now bind the graphics pipeline. The second parameter specifies if the pipeline object is a graphics or
    // compute pipeline.
    command_op.bind_pipeline( VK_PIPELINE_BIND_POINT_GRAPHICS, *graphics_pipeline_ptr_ );

    // We did specify viewport and scissor state for this pipeline to be dynamic. So we need to set them in the command
    // buffer before issuing our draw command.
    command_op.set_viewport( VkViewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>( swapchain_ptr_->extent( ).width ),
        .height = static_cast<float>( swapchain_ptr_->extent( ).height ),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    } );
    command_op.set_scissor( VkRect2D{
        .offset = { 0, 0 },
        .extent = swapchain_ptr_->extent( )
    } );

    command_op.bind_vertex_buffers( *vertex_buffer_ptr_, 0 );
    command_op.bind_index_buffer( *index_buffer_ptr_, 0, VK_INDEX_TYPE_UINT32 );

    // Bind the right descriptor set for each frame to the descriptors in the shader.
    command_op.bind_descriptor_set( VK_PIPELINE_BIND_POINT_GRAPHICS, *graphics_pipeline_ptr_, descriptor_sets_[current_frame_] );

    // We now issue the draw command. The number of indices represents the number of vertices that will be passed to
    // the vertex shader.
    command_op.draw_indexed( static_cast<uint32_t>( model_->indices( ).size( ) ), 1 );

    // After we've finished recording the command buffer, we end the rendering process.
    command_op.end_rendering( );
}


void TriangleApplication::update_uniform_buffer( uint32_t const current_image ) const
{
    static auto start_time = std::chrono::high_resolution_clock::now( );

    auto const current_time = std::chrono::high_resolution_clock::now( );
    float const time        = std::chrono::duration<float>( current_time - start_time ).count( );

    UniformBufferObject ubo{};
    ubo.model = glm::rotate( glm::mat4( 1.0f ), time * glm::radians( 90.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    ubo.view  = glm::lookAt( glm::vec3( 2.0f, 2.0f, 2.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ),
                             glm::vec3( 0.0f, 0.0f, 1.0f ) );
    ubo.proj = glm::perspective( glm::radians( 45.0f ),
                                 static_cast<float>( swapchain_ptr_->extent( ).width ) / swapchain_ptr_->extent( ).height,
                                 0.1f,
                                 10.0f );
    ubo.proj[1][1] *= -1;

    memcpy( uniform_buffers_[current_image].data( ), &ubo, sizeof( ubo ) );
}


void TriangleApplication::draw_frame( )
{
    // We don't render frames when the window is minimized.
    if ( window_->is_minimized( ) )
    {
        return;
    }

    // 1. Wait for the previous frame to finish. We wait for the fence.
    auto const in_flight_fence = in_flight_fences_[current_frame_];
    context_->device( ).wait_for_fence( in_flight_fence );

    // 2. Acquire an image from the swapchain.
    auto const acquire_semaphore = image_available_semaphores_[current_frame_];
    uint32_t const image_index   = swapchain_ptr_->acquire_next_image( acquire_semaphore );

    // If the image index is UINT32_MAX, it means that the swapchain could not acquire an image.
    if ( image_index == UINT32_MAX )
    {
        return;
    }

    // After waiting, we need to manually reset the fence to the unsignaled state with the vkResetFences call.
    // We reset the fence only if there's work to do, which is why we're doing it after the acquire image check. DEADLOCK warning!
    context_->device( ).reset_fence( in_flight_fence );

    // 3. Record a command buffer which draws the scene onto that image.
    // With the imageIndex specifying the swap chain image to use in hand, we can now record the command buffer.
    record_command_buffer( *command_buffers_[current_frame_], image_index );

    // We need to submit the recorded command buffer to the graphics queue before submitting the image to the swap chain.
    update_uniform_buffer( current_frame_ );

    VkSemaphoreSubmitInfo const wait_semaphore_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = acquire_semaphore,
        .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
        .deviceIndex = 0,
        .value = 0 // 0 for binary semaphore, non-zero for timeline
    };

    auto const command_buffer_info = command_buffers_[current_frame_]->make_submit_info( );

    auto const submit_semaphore = render_finished_semaphores_[image_index];
    VkSemaphoreSubmitInfo const signal_semaphore_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = submit_semaphore,
        .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR,
        .deviceIndex = 0,
        .value = 0,
    };

    VkSubmitInfo2 const submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .waitSemaphoreInfoCount = 1,
        .pWaitSemaphoreInfos = &wait_semaphore_info,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &command_buffer_info,
        .signalSemaphoreInfoCount = 1,
        .pSignalSemaphoreInfos = &signal_semaphore_info,
    };

    // 4. Submit the recorded command buffer.
    // The last parameter references an optional fence that will be signaled when the command buffers finish execution.
    // This allows us to know when it is safe for the command buffer to be reused, thus we want to give it inFlightFence.
    context_->device( ).graphics_queue( ).submit( submit_info, in_flight_fence );

    // The last step of drawing a frame is submitting the result back to the swap chain to have it eventually show up
    // on the screen. Presentation is configured through a VkPresentInfoKHR structure.
    VkPresentInfoKHR const present_info{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,

        // These two parameters specify which semaphores to wait on before presentation can happen.
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &submit_semaphore,

        // These two parameters specify the swap chains to present images to and the index of the image for each swap chain.
        .swapchainCount = 1,
        .pSwapchains = swapchain_ptr_->handle_ptr( ),
        .pImageIndices = &image_index,
    };

    // 5. Present the swapchain image. The vkQueuePresentKHR function submits the request to present an image to the queue.
    // We check for the callback boolean after the queue presentation to avoid the semaphore to be signaled.
    if ( VkResult const queue_present_result = context_->device( ).graphics_queue( ).present( present_info );
        queue_present_result == VK_ERROR_OUT_OF_DATE_KHR || queue_present_result == VK_SUBOPTIMAL_KHR )
    {
        window_->force_framebuffer_resize( );
    }

    // Next frame
    current_frame_ = ( current_frame_ + 1 ) % MAX_FRAMES_IN_FLIGHT_;

    // todo: add this info inside fence and semaphore class
    // A core design philosophy in Vulkan is that synchronization of execution on the GPU is explicit. The order of
    // operations is up to us to define using various synchronization primitives.

    // A semaphore is used to add order between queue operations. Queue operations refer to the work we submit to a
    // queue, either in a command buffer or from within a function. The queueing happens on the CPU, but the execution
    // and the waiting happen on the GPU.

    // A fence has a similar purpose, in that it is used to synchronize execution, but it is for ordering the execution
    // on the CPU. Simply put, if the host needs to know when the GPU has finished something, we use a fence.

    // In summary, semaphores are used to specify the execution order of operations on the GPU while fences are used to
    // keep the CPU and GPU in sync with each-other.

    // We'll need one semaphore to signal that an image has been acquired from the swapchain and is ready for rendering,
    // another one to signal that rendering has finished and presentation can happen, and a fence to make sure only one
    // frame is rendering at a time.
}


void TriangleApplication::init_vk( )
{
    vk_create_descriptor_set_layout( );

    vk_create_graphics_pipeline( );

    vk_create_command_pool( );
    vk_create_command_buffers( );

    vk_create_texture_image( );
    vk_create_texture_sampler( );

    load_model( );
    vk_create_model_buffers( );
    vk_create_uniform_buffers( );

    vk_create_descriptor_pool( );
    vk_create_descriptor_sets( );

    vk_create_sync_objects( );
}


void TriangleApplication::configure_relative_path( ) const
{
    xos::info::log_info( std::clog );
    xos::filesystem::configure_relative_path( );
}


void TriangleApplication::main_loop( )
{
    glfwPollEvents( );

    draw_frame( );
}


void TriangleApplication::cleanup( )
{
    // Allocation and de-allocation functions in Vulkan have an optional allocator callback
    // that we'll ignore by passing nullptr.
    for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT_; i++ )
    {
        vkDestroySemaphore( context_->device( ).logical( ), image_available_semaphores_[i], nullptr );
        vkDestroyFence( context_->device( ).logical( ), in_flight_fences_[i], nullptr );
    }
    for ( size_t i = 0; i < 3; i++ )
    {
        vkDestroySemaphore( context_->device( ).logical( ), render_finished_semaphores_[i], nullptr );
    }

    command_pool_ptr_.reset( );

    graphics_pipeline_ptr_.reset( );
    swapchain_ptr_.reset( );

    vkDestroySampler( context_->device( ).logical( ), texture_sampler_, nullptr );
    texture_image_ptr_.reset( );

    uniform_buffers_.clear( );

    vkDestroyDescriptorPool( context_->device( ).logical( ), descriptor_pool_, nullptr );
    descriptor_set_layout_ptr_.reset( );

    index_buffer_ptr_.reset( );
    vertex_buffer_ptr_.reset( );

    CVK.reset_instance( );
}
