#include "MyApplication.h"

#include "debug_callback.h"
#include "UniformBufferObject.h"

#include <CobaltVK.h>
#include <__buffer/CommandPool.h>
#include <__context/VkContext.h>
#include <__enum/ValidationFlags.h>
#include <__model/AssimpModelLoader.h>
#include <__model/Model.h>
#include <__render/DescriptorAllocator.h>
#include <__render/Renderer.h>
#include <__render/Swapchain.h>
#include <__shader/ShaderModule.h>

#include <xos/filesystem.h>
#include <xos/info.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <filesystem>
#include <iostream>
#include <__buffer/Buffer.h>
#include <__image/TextureImage.h>
#include <__pipeline/GraphicsPipeline.h>


using namespace cobalt;

// +---------------------------+
// | PUBLIC                    |
// +---------------------------+
MyApplication::MyApplication( )
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
        .with<ValidationLayers>( ValidationFlags::KHRONOS_VALIDATION, ::debug::debug_callback )
    );

    // 3. Set the proper root directory to find shader modules and textures.
    configure_relative_path( );

    // 4. Swapchain
    swapchain_ = CVK.create_resource<Swapchain>(
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

    // 5. Render pipeline
    command_pool_         = CVK.create_resource<CommandPool>( *context_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT );
    descriptor_allocator_ = CVK.create_resource<DescriptorAllocator>(
        context_->device( ), MAX_FRAMES_IN_FLIGHT_,
        std::array{
            DescriptorSetLayout::layout_binding_pair_t{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT },
            DescriptorSetLayout::layout_binding_pair_t{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT }
        }
    );
    create_graphics_pipeline( );

    // 7. Renderer
    renderer_ = CVK.create_resource<Renderer>( RendererCreateInfo{
        .device = &context_->device( ),
        .swapchain = swapchain_.get( ),
        .cmd_pool = command_pool_.get( ),
        .desc_allocator = descriptor_allocator_.get( ),
        .max_frames_in_flight = MAX_FRAMES_IN_FLIGHT_
    } );
    renderer_->set_record_command_buffer_fn(
        std::bind( &MyApplication::record_command_buffer, this,
                   std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 ) );
    renderer_->set_update_uniform_buffer_fn(
        std::bind( &MyApplication::update_uniform_buffer, this, std::placeholders::_1 ) );

    // 8. Texture image
    texture_image_ = CVK.create_resource<TextureImage>(
        context_->device( ), *command_pool_,
        TextureImageCreateInfo{
            .path_to_img = TEXTURE_PATH_,
            .image_format = VK_FORMAT_R8G8B8A8_SRGB
        },
        TextureSamplerCreateInfo{
            .filter = VK_FILTER_LINEAR,
            .address_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .border_color = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK,
            .mipmap_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .unnormalized_coordinates = false,
            .compare_enable = false,
            .compare_op = VK_COMPARE_OP_ALWAYS
        } );

    // 9. Model
    model_        = CVK.create_resource<Model>( loader::AssimpModelLoader{ MODEL_PATH_ } );
    index_buffer_ = CVK.create_resource<Buffer>(
        buffer::make_index_buffer<Model::index_t>( context_->device( ), *command_pool_, model_->indices( ) )
    );
    vertex_buffer_ = CVK.create_resource<Buffer>(
        buffer::make_vertex_buffer<Vertex>( context_->device( ), *command_pool_, model_->vertices( ) )
    );

    // 10. Uniform buffers
    for ( uint32_t i{}; i < MAX_FRAMES_IN_FLIGHT_; i++ )
    {
        uniform_buffers_.emplace_back(
            CVK.create_resource<Buffer>( buffer::make_uniform_buffer( context_->device( ), sizeof( UniformBufferObject ) ) ) );
    }

    // 11. Update descriptor sets
    std::array write_ops{
        WriteDescription{
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            [this]( uint32_t const frame ) -> VkDescriptorBufferInfo
                {
                    return {
                        .buffer = uniform_buffers_[frame]->handle( ),
                        .offset = 0,
                        .range = sizeof( UniformBufferObject ),
                    };
                }
        },
        WriteDescription{
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            [this]( uint32_t const ) -> VkDescriptorImageInfo
                {
                    return {
                        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                        .imageView = texture_image_->image( ).view( ).handle( ),
                        .sampler = texture_image_->sampler( )
                    };
                },
        }
    };
    descriptor_allocator_->update_sets( write_ops );
}


MyApplication::~MyApplication( ) noexcept
{
    // We wait for the device to finish all operations before cleaning up, since the render is asynchronous in the GPU.
    context_->device( ).wait_idle( );
    CVK.reset_instance( );
}


void MyApplication::run( )
{
    running_ = true;
    while ( running_ )
    {
        glfwPollEvents( );

        if ( window_->is_minimized( ) )
        {
            continue;
        }

        if ( auto const render_result = renderer_->render( );
            render_result == VK_ERROR_OUT_OF_DATE_KHR || render_result == VK_SUBOPTIMAL_KHR )
        {
            window_->force_framebuffer_resize( );
        }

        running_ = not window_->should_close( );
    }
}


// +---------------------------+
// | PRIVATE                   |
// +---------------------------+
void MyApplication::create_graphics_pipeline( )
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

    graphics_pipeline_ = CVK.create_resource<GraphicsPipeline>(
        context_->device( ),
        GraphicsPipelineCreateInfo{
            .shader_stages = std::move( shader_stages ),
            .descriptor_set_layouts = { descriptor_allocator_->layout( ).handle( ) },
            .binding_description = { Vertex::get_binding_description( ), Vertex::get_attribute_descriptions( ) },
            .input_assembly = input_assembly,
            .rasterization = rasterization,
            .multisampling = multisampling,
            .color_blend_attachment = color_blend_attachment,
            .depth_stencil = depth_stencil,
            .swapchain_image_format = swapchain_->image_format( ),
            .depth_image_format = swapchain_->depth_image( ).format( ),
        } );
}


void MyApplication::record_command_buffer( CommandBuffer const& buffer, Image const& image, VkDescriptorSet const desc_set ) const
{
    buffer.reset( );
    auto const command_op = buffer.command_operator( 0 );

    // 1. We set the memory barriers for the swapchain image.
    {
        std::array const barriers{
            // Pre-rendering barrier
            VkImageMemoryBarrier2{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                .srcAccessMask = VK_ACCESS_2_NONE,
                .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = image.handle( ),
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                }
            },

            // Post-rendering barrier
            VkImageMemoryBarrier2{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
                .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                .dstAccessMask = VK_ACCESS_2_NONE,
                .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = image.handle( ),
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                }
            }
        };

        command_op.insert_barrier( VkDependencyInfo{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 2,
            .pImageMemoryBarriers = barriers.data( )
        } );
    }

    // 2. Now we specify the dynamic rendering information, which allows us to render directly to the swapchain images without
    // setting up a pipeline with a render pass.
    {
        VkRenderingAttachmentInfo const color_attachment_info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = image.view( ).handle( ),
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue.color = VkClearColorValue{ { 0.0f, 0.0f, 0.0f, 1.0f } }
        };

        VkRenderingAttachmentInfo const depth_attachment_info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = swapchain_->depth_image( ).view( ).handle( ),
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .clearValue.depthStencil.depth = 1.0f
        };

        VkRenderingInfo const render_info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea.extent = image.extent( ),
            .renderArea.offset = { 0, 0 },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attachment_info,
            .pDepthAttachment = &depth_attachment_info
        };

        // We begin the rendering process by calling vkCmdBeginRendering, which starts recording commands
        command_op.begin_rendering( render_info );
    }

    // 3. We can now bind the graphics pipeline.
    command_op.bind_pipeline( VK_PIPELINE_BIND_POINT_GRAPHICS, *graphics_pipeline_ );

    // 4. We did specify viewport and scissor state for this pipeline to be dynamic. So we need to set them in the command
    // buffer before issuing our draw command.
    command_op.set_viewport( VkViewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>( image.extent( ).width ),
        .height = static_cast<float>( image.extent( ).height ),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    } );
    command_op.set_scissor( VkRect2D{
        .offset = { 0, 0 },
        .extent = image.extent( )
    } );

    // 5. We attach the vertex and index buffers.
    command_op.bind_vertex_buffers( *vertex_buffer_, 0 );
    command_op.bind_index_buffer( *index_buffer_, 0 );

    // 6. We bind the right descriptor set for each frame to the descriptors in the shader.
    command_op.bind_descriptor_set( VK_PIPELINE_BIND_POINT_GRAPHICS, *graphics_pipeline_, desc_set );

    // 7. We can finally issue the draw command.
    command_op.draw_indexed( static_cast<uint32_t>( model_->indices( ).size( ) ), 1 );

    // 8. Now that we've finished recording the command buffer, we end the rendering process.
    command_op.end_rendering( );
}


void MyApplication::update_uniform_buffer( uint32_t const current_image ) const
{
    static auto start_time = std::chrono::high_resolution_clock::now( );

    auto const current_time = std::chrono::high_resolution_clock::now( );
    float const time        = std::chrono::duration<float>( current_time - start_time ).count( );

    UniformBufferObject ubo{};
    ubo.model = rotate( glm::mat4( 1.0f ), time * glm::radians( 90.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    ubo.view  = lookAt( glm::vec3( 2.0f, 2.0f, 2.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    ubo.proj  = glm::perspective( glm::radians( 45.0f ),
                                  static_cast<float>( swapchain_->extent( ).width ) / swapchain_->extent( ).height,
                                  0.1f,
                                  10.0f );
    ubo.proj[1][1] *= -1;

    memcpy( uniform_buffers_[current_image]->data( ), &ubo, sizeof( ubo ) );
}


void MyApplication::configure_relative_path( )
{
    xos::info::log_info( std::clog );
    xos::filesystem::configure_relative_path( );
}
