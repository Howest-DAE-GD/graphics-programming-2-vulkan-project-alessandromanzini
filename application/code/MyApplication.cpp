#include "MyApplication.h"

#include "Camera.h"
#include "debug_callback.h"
#include "Timer.h"
#include "UniformBufferObject.h"

#include <CobaltVK.h>
#include <__buffer/Buffer.h>
#include <__buffer/CommandPool.h>
#include <__context/VkContext.h>
#include <__enum/ValidationFlags.h>
#include <__image/ImageSampler.h>
#include <__model/AssimpModelLoader.h>
#include <__model/Model.h>
#include <__pipeline/Pipeline.h>
#include <__render/DescriptorAllocator.h>
#include <__render/Renderer.h>
#include <__render/Swapchain.h>
#include <__shader/ShaderModule.h>

#include <xos/filesystem.h>
#include <xos/info.h>

#include <filesystem>
#include <iostream>
#include <__pipeline/GraphicsPipelineBuilder.h>


using namespace cobalt;


// +---------------------------+
// | PUBLIC                    |
// +---------------------------+
MyApplication::MyApplication( )
{
    // 1. Create Window and Camera
    window_ = CVK.create_resource<Window>( WIDTH_, HEIGHT_, "Vulkan App" );

    camera_ptr_ = std::make_unique<Camera>( window_->handle( ), window_->extent( ), glm::radians( 45.f ), 0.1f, 20.f );
    window_->on_framebuffer_resize.bind( camera_ptr_.get( ), &Camera::set_viewport );

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
            DeviceFeatureFlags::DYNAMIC_RENDERING_EXT | DeviceFeatureFlags::SYNCHRONIZATION_2_EXT |
            DeviceFeatureFlags::INDEPENDENT_BLEND )
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
                .surface_format = { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }
            }
        } );
    command_pool_ = CVK.create_resource<CommandPool>( *context_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT );
    swapchain_->depth_image( ).transition_layout( { VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL }, *command_pool_ );

    // 5. Descriptors
    descriptor_allocator_ = CVK.create_resource<DescriptorAllocator>(
        context_->device( ), MAX_FRAMES_IN_FLIGHT_,
        std::array{
            // Camera uniform buffer
            LayoutBindingDescription{ VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },

            // Sampler
            LayoutBindingDescription{ VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_SAMPLER },

            // Textures
            LayoutBindingDescription{ VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, TEXTURES_COUNT_ },

            // Texture Indices Buffer
            LayoutBindingDescription{ VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER },

            // Albedo Image Buffer
            LayoutBindingDescription{ VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE },

            // Normal Image Buffer
            LayoutBindingDescription{ VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE }
        }
    );

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

    // 8. Sampler and GBuffers
    texture_sampler_ = CVK.create_resource<ImageSampler>(
        context_->device( ),
        ImageSamplerCreateInfo{
            .filter = VK_FILTER_LINEAR,
            .address_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .mipmap_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .border_color = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK,
            .unnormalized_coordinates = false,
            .compare_enable = false,
            .compare_op = VK_COMPARE_OP_ALWAYS
        } );

    albedo_image_ = CVK.create_resource<Image>(
        context_->device( ), ImageCreateInfo{
            .extent = swapchain_->extent( ),
            .format = VK_FORMAT_R8G8B8A8_SRGB,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT
        }
    );
    normal_image_ = CVK.create_resource<Image>(
        context_->device( ), ImageCreateInfo{
            .extent = swapchain_->extent( ),
            .format = VK_FORMAT_R32G32_SFLOAT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT
        }
    );

    // 9. Graphic pipelines
    create_pipelines( );

    // 10. Model
    model_ = CVK.create_resource<Model>( context_->device( ), *command_pool_, loader::AssimpModelLoader{ MODEL_PATH_ } );

    // 11. Uniform buffers
    for ( uint32_t i{}; i < MAX_FRAMES_IN_FLIGHT_; i++ )
    {
        uniform_buffers_.emplace_back(
            CVK.create_resource<Buffer>( buffer::make_uniform_buffer( context_->device( ), sizeof( UniformBufferObject ) ) ) );
    }

    // 12. Update descriptor sets
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
            VK_DESCRIPTOR_TYPE_SAMPLER,
            [this]( uint32_t const ) -> VkDescriptorImageInfo
                {
                    return {
                        .sampler = texture_sampler_->handle( )
                    };
                },
        },
        WriteDescription{
            VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            [this]( uint32_t ) -> std::vector<VkDescriptorImageInfo>
                {
                    auto const texture_images = model_->textures( );
                    std::vector<VkDescriptorImageInfo> infos;
                    infos.reserve( texture_images.size( ) );
                    for ( auto const& tex : texture_images )
                    {
                        infos.push_back( {
                            .imageView = tex.image( ).view( ).handle( ),
                            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                        } );
                    }
                    return infos;
                }
        },
        WriteDescription{
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            [this]( uint32_t ) -> VkDescriptorBufferInfo
                {
                    return {
                        .buffer = model_->surface_buffer( ).handle( ),
                        .offset = 0,
                        .range = model_->surface_buffer( ).memory_size( )
                    };
                }
        },
        WriteDescription{
            VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            [this]( uint32_t ) -> VkDescriptorImageInfo
                {
                    return {
                        .imageView = albedo_image_->view( ).handle( ),
                        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                    };
                }
        },
        WriteDescription{
            VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            [this]( uint32_t ) -> VkDescriptorImageInfo
                {
                    return {
                        .imageView = normal_image_->view( ).handle( ),
                        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                    };
                }
        },
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
    Timer timer{};

    timer.start( );
    running_ = true;
    while ( running_ )
    {
        glfwPollEvents( );

        timer.update( );
        camera_ptr_->update( &timer );

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
void MyApplication::create_pipelines( )
{
    textures_pipeline_layout_ = CVK.create_resource<PipelineLayout>(
        context_->device( ), std::array{ descriptor_allocator_->layout( ).handle( ) },
        std::array{
            VkPushConstantRange{
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                .offset = 0,
                .size = sizeof( uint32_t )
            }
        } );
    quad_pipeline_layout_ = CVK.create_resource<PipelineLayout>( context_->device( ),
                                                                 std::array{ descriptor_allocator_->layout( ).handle( ) } );

    // Depth pre-pass pipeline
    {
        constexpr VkSpecializationMapEntry specialization_entry{
            .constantID = 0,
            .offset = 0,
            .size = sizeof( TEXTURES_COUNT_ ),
        };

        VkSpecializationInfo const specialization_info{
            .mapEntryCount = 1,
            .pMapEntries = &specialization_entry,
            .dataSize = sizeof( TEXTURES_COUNT_ ),
            .pData = &TEXTURES_COUNT_
        };

        depth_prepass_pipeline_ = CVK.create_resource<Pipeline>(
            builder::GraphicsPipelineBuilder{}
            .add_shader_module( { context_->device( ), "shaders/transform.vert.spv", VK_SHADER_STAGE_VERTEX_BIT } )
            .add_shader_module( { context_->device( ), "shaders/depth.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT },
                                &specialization_info )
            .set_dynamic_state( std::array{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR } )
            .set_binding_description( Vertex::get_binding_description( ), Vertex::get_attribute_descriptions( ) )
            .set_depth_stencil_mode( VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS )
            .set_depth_image_description( swapchain_->depth_image( ).format( ) )
            .build( context_->device( ), *textures_pipeline_layout_ ) );
    }

    // G-Buffer generation pipeline
    {
        constexpr VkSpecializationMapEntry specialization_entry{
            .constantID = 0,
            .offset = 0,
            .size = sizeof( TEXTURES_COUNT_ ),
        };

        VkSpecializationInfo const specialization_info{
            .mapEntryCount = 1,
            .pMapEntries = &specialization_entry,
            .dataSize = sizeof( TEXTURES_COUNT_ ),
            .pData = &TEXTURES_COUNT_
        };

        gbuffer_gen_pipeline_ = CVK.create_resource<Pipeline>(
            builder::GraphicsPipelineBuilder{}
            .add_shader_module( { context_->device( ), "shaders/transform.vert.spv", VK_SHADER_STAGE_VERTEX_BIT } )
            .add_shader_module( { context_->device( ), "shaders/gbuffer_gen.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT },
                                &specialization_info )
            .set_dynamic_state( std::array{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR } )
            .set_binding_description( Vertex::get_binding_description( ), Vertex::get_attribute_descriptions( ) )
            .set_depth_stencil_mode( VK_TRUE, VK_FALSE, VK_COMPARE_OP_EQUAL )
            .set_depth_image_description( swapchain_->depth_image( ).format( ) )
            .add_color_attachment_description(
                VkPipelineColorBlendAttachmentState{
                    .blendEnable = VK_FALSE,
                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                      VK_COLOR_COMPONENT_A_BIT,
                }, albedo_image_->format( ) )
            .add_color_attachment_description(
                VkPipelineColorBlendAttachmentState{
                    .blendEnable = VK_FALSE,
                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT,
                }, normal_image_->format( ) )
            .build( context_->device( ), *textures_pipeline_layout_ ) );
    }

    // Color pass pipeline
    {
        color_pass_pipeline_ = CVK.create_resource<Pipeline>(
            builder::GraphicsPipelineBuilder{}
            .add_shader_module( { context_->device( ), "shaders/quad.vert.spv", VK_SHADER_STAGE_VERTEX_BIT } )
            .add_shader_module( { context_->device( ), "shaders/lighting.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT } )
            .set_dynamic_state( std::array{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR } )
            .set_depth_stencil_mode( VK_FALSE, VK_FALSE )
            .set_depth_image_description( swapchain_->depth_image( ).format( ) )
            .set_cull_mode( VK_CULL_MODE_NONE )
            .add_color_attachment_description(
                VkPipelineColorBlendAttachmentState{
                    .blendEnable = VK_FALSE,
                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                      VK_COLOR_COMPONENT_A_BIT,
                }, swapchain_->image_format( ) )
            .build( context_->device( ), *textures_pipeline_layout_ ) );
    }
}


void MyApplication::record_command_buffer( CommandBuffer const& buffer, Image& image, VkDescriptorSet const desc_set )
{
    buffer.reset( );
    auto const command_op = buffer.command_operator( 0 );

    // 1. Depth Pre-Pass: render geometry to depth only, no color attachment
    {
        // Pre depth pass barrier
        swapchain_->depth_image( ).transition_layout(
            ImageLayoutTransition{ VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL }
            .from_stage( VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT )
            .to_stage( VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT )
            .from_access( VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT )
            .to_access( VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT ), command_op );

        VkRenderingAttachmentInfo const depth_prepass_attachment_info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = swapchain_->depth_image( ).view( ).handle( ),
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = {
                .depthStencil = {
                    .depth = 1.0f
                }
            }
        };

        VkRenderingInfo const depth_prepass_render_info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = {
                .offset = { 0, 0 },
                .extent = image.extent( ),
            },
            .layerCount = 1,
            .colorAttachmentCount = 0,
            .pColorAttachments = nullptr,
            .pDepthAttachment = &depth_prepass_attachment_info
        };

        command_op.begin_rendering( depth_prepass_render_info );
        command_op.bind_pipeline( VK_PIPELINE_BIND_POINT_GRAPHICS, *depth_prepass_pipeline_ );

        command_op.set_viewport( VkViewport{
            .x = 0.f, .y = 0.f,
            .width = static_cast<float>( image.extent( ).width ),
            .height = static_cast<float>( image.extent( ).height ),
            .minDepth = 0.f, .maxDepth = 1.f
        } );
        command_op.set_scissor( VkRect2D{ .offset = { 0, 0 }, .extent = image.extent( ) } );

        command_op.bind_vertex_buffers( model_->vertex_buffer( ), 0 );
        command_op.bind_index_buffer( model_->index_buffer( ), 0 );

        command_op.bind_descriptor_set( VK_PIPELINE_BIND_POINT_GRAPHICS, *depth_prepass_pipeline_, desc_set );

        for ( auto const& [index_count, index_offset, vertex_offset, material_index] : model_->meshes( ) )
        {
            command_op.push_constants( depth_prepass_pipeline_->layout( ), VK_SHADER_STAGE_FRAGMENT_BIT,
                                       0, sizeof( uint32_t ), &material_index );
            command_op.draw_indexed( index_count, 1, index_offset, vertex_offset );
        }

        command_op.end_rendering( );

        // Post depth pass barrier
        swapchain_->depth_image( ).transition_layout(
            ImageLayoutTransition{ VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL }
            .from_stage( VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT )
            .to_stage( VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT )
            .from_access( VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT )
            .to_access( VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT ), command_op );
    }

    // 2. G-Buffer generation pass: color on g-buffer images
    {
        // Pre gbuffer generation barrier
        albedo_image_->transition_layout(
            ImageLayoutTransition{ VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }
            .from_stage( VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT )
            .to_stage( VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT )
            .from_access( VK_ACCESS_2_SHADER_SAMPLED_READ_BIT )
            .to_access( VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT ),
            command_op );
        normal_image_->transition_layout(
            ImageLayoutTransition{ VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }
            .from_stage( VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT )
            .to_stage( VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT )
            .from_access( VK_ACCESS_2_SHADER_SAMPLED_READ_BIT )
            .to_access( VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT ),
            command_op );

        std::array const color_attachments_info{
            VkRenderingAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = albedo_image_->view( ).handle( ),
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = {
                    .color = VkClearColorValue{ { 0.0f, 0.0f, 0.0f, 1.0f } }
                }
            },
            VkRenderingAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = normal_image_->view( ).handle( ),
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = {
                    .color = VkClearColorValue{ { 0.0f, 0.0f, 0.0f, 1.0f } }
                }
            }
        };

        VkRenderingAttachmentInfo const depth_attachment_info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = swapchain_->depth_image( ).view( ).handle( ),
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .clearValue = {
                .depthStencil = {
                    .depth = 1.0f
                }
            }
        };

        command_op.begin_rendering( VkRenderingInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = {
                .offset = { 0, 0 },
                .extent = albedo_image_->extent( ),
            },
            .layerCount = 1,
            .colorAttachmentCount = color_attachments_info.size( ),
            .pColorAttachments = color_attachments_info.data( ),
            .pDepthAttachment = &depth_attachment_info
        } );

        command_op.bind_pipeline( VK_PIPELINE_BIND_POINT_GRAPHICS, *gbuffer_gen_pipeline_ );

        command_op.set_viewport( VkViewport{
            .x = 0.f, .y = 0.f,
            .width = static_cast<float>( albedo_image_->extent( ).width ),
            .height = static_cast<float>( albedo_image_->extent( ).height ),
            .minDepth = 0.f, .maxDepth = 1.f
        } );
        command_op.set_scissor( VkRect2D{ .offset = { 0, 0 }, .extent = albedo_image_->extent( ) } );

        command_op.bind_vertex_buffers( model_->vertex_buffer( ), 0 );
        command_op.bind_index_buffer( model_->index_buffer( ), 0 );

        command_op.bind_descriptor_set( VK_PIPELINE_BIND_POINT_GRAPHICS, *gbuffer_gen_pipeline_, desc_set );

        for ( auto const& [index_count, index_offset, vertex_offset, material_index] : model_->meshes( ) )
        {
            command_op.push_constants( gbuffer_gen_pipeline_->layout( ), VK_SHADER_STAGE_FRAGMENT_BIT,
                                       0, sizeof( uint32_t ), &material_index );
            command_op.draw_indexed( index_count, 1, index_offset, vertex_offset );
        }

        command_op.end_rendering( );

        // Post gbuffer generation barrier
        albedo_image_->transition_layout(
            ImageLayoutTransition{ VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }
            .from_stage( VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT )
            .to_stage( VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT )
            .from_access( VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT )
            .to_access( VK_ACCESS_2_SHADER_SAMPLED_READ_BIT ),
            command_op );
        normal_image_->transition_layout(
            ImageLayoutTransition{ VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }
            .from_stage( VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT )
            .to_stage( VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT )
            .from_access( VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT )
            .to_access( VK_ACCESS_2_SHADER_SAMPLED_READ_BIT ),
            command_op );
    }

    // 3. Color pass: color + depth read-only
    {
        // Pre color pass barrier
        image.transition_layout(
            ImageLayoutTransition{ VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }
            .from_stage( VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT )
            .to_stage( VK_PIPELINE_STAGE_2_TRANSFER_BIT )
            .from_access( VK_ACCESS_2_NONE )
            .to_access( VK_ACCESS_2_TRANSFER_WRITE_BIT ), command_op );

        VkRenderingAttachmentInfo const color_attachment_info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = image.view( ).handle( ),
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = {
                .color = VkClearColorValue{ { 0.0f, 0.0f, 0.0f, 1.0f } }
            }
        };

        command_op.begin_rendering( VkRenderingInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = {
                .offset = { 0, 0 },
                .extent = image.extent( ),
            },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attachment_info,
            .pDepthAttachment = nullptr
        } );

        command_op.bind_pipeline( VK_PIPELINE_BIND_POINT_GRAPHICS, *color_pass_pipeline_ );

        command_op.set_viewport( VkViewport{
            .x = 0.f, .y = 0.f,
            .width = static_cast<float>( image.extent( ).width ),
            .height = static_cast<float>( image.extent( ).height ),
            .minDepth = 0.f, .maxDepth = 1.f
        } );
        command_op.set_scissor( VkRect2D{ .offset = { 0, 0 }, .extent = image.extent( ) } );

        command_op.bind_descriptor_set( VK_PIPELINE_BIND_POINT_GRAPHICS, *color_pass_pipeline_, desc_set );

        command_op.draw( 4, 1 );

        command_op.end_rendering( );

        // Post color pass barrier
        image.transition_layout(
            ImageLayoutTransition{ VK_IMAGE_LAYOUT_PRESENT_SRC_KHR }
            .from_stage( VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT )
            .to_stage( VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT )
            .from_access( VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT )
            .to_access( VK_ACCESS_2_NONE ), command_op );
    }
}


void MyApplication::update_uniform_buffer( uint32_t const current_image ) const
{
    UniformBufferObject const ubo{
        .model = glm::mat4( 1.0f ), //rotate( glm::mat4( 1.0f ), glm::radians( 90.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
        .view = camera_ptr_->camera_to_world( ),
        .proj = camera_ptr_->projection( )
    };
    uniform_buffers_[current_image]->write( &ubo, sizeof( UniformBufferObject ) );
}


void MyApplication::configure_relative_path( )
{
    xos::info::log_info( std::clog );
    xos::filesystem::configure_relative_path( );
}
