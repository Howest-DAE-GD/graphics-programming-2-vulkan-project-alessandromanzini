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
#include <__pipeline/GraphicsPipeline.h>
#include <__render/DescriptorAllocator.h>
#include <__render/Renderer.h>
#include <__render/Swapchain.h>
#include <__shader/ShaderModule.h>

#include <xos/filesystem.h>
#include <xos/info.h>

#include <filesystem>
#include <iostream>


using namespace cobalt;


GraphicsPipelineCreateInfo make_graphics_pipeline_create_info( std::span<VkPipelineShaderStageCreateInfo const> shaders );


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
                .surface_format = { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }
            }
        } );
    command_pool_ = CVK.create_resource<CommandPool>( *context_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT );
    swapchain_->depth_image( ).transition_layout( { VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL }, *command_pool_ );

    // 5. Render pipeline
    descriptor_allocator_ = CVK.create_resource<DescriptorAllocator>(
        context_->device( ), MAX_FRAMES_IN_FLIGHT_,
        std::array{
            LayoutBindingDescription{ VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },
            LayoutBindingDescription{ VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_SAMPLER },
            LayoutBindingDescription{ VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 24 },
            LayoutBindingDescription{ VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER }
        }
    );
    create_depth_prepass_pipeline( );
    create_main_render_pipeline( );

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

    // 9. Model
    model_ = CVK.create_resource<Model>( context_->device( ), *command_pool_, loader::AssimpModelLoader{ MODEL_PATH_ } );

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
                        .buffer = model_->materials_buffer( ).handle( ),
                        .offset = 0,
                        .range = model_->materials_buffer( ).memory_size( )
                    };
                }
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
void MyApplication::create_depth_prepass_pipeline( )
{
    // Create shader modules for depth prepass
    shader::ShaderModule const vert_shader{ context_->device( ), "shaders/geometry.vert.spv" };
    shader::ShaderModule const frag_shader{ context_->device( ), "shaders/depth.frag.spv" };

    constexpr VkSpecializationMapEntry specialization_entry{
        .constantID = 0,
        .offset = 0,
        .size = sizeof( uint32_t ),
    };

    constexpr uint32_t texture_count{ 24u };

    VkSpecializationInfo specialization_info{
        .mapEntryCount = 1,
        .pMapEntries   = &specialization_entry,
        .dataSize      = sizeof( uint32_t ),
        .pData         = &texture_count
    };

    GraphicsPipelineCreateInfo info = make_graphics_pipeline_create_info(
        std::array<VkPipelineShaderStageCreateInfo, 2>{
            {
                {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .stage = VK_SHADER_STAGE_VERTEX_BIT,
                    .module = vert_shader.handle( ),
                    .pName = "main"
                },
                {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .module = frag_shader.handle( ),
                    .pName = "main",
                    .pSpecializationInfo = &specialization_info
                }
            }
        } );

    // Disable color writes completely for depth prepass
    info.color_blend_attachment = std::nullopt;

    // Set depth test and write enable
    info.depth_stencil.depthTestEnable  = VK_TRUE;
    info.depth_stencil.depthWriteEnable = VK_TRUE;
    info.depth_stencil.depthCompareOp   = VK_COMPARE_OP_LESS,

            // No color attachment in depth prepass
            info.descriptor_set_layouts = { descriptor_allocator_->layout( ).handle( ) };
    info.swapchain_image_format         = VK_FORMAT_UNDEFINED;
    info.depth_image_format             = swapchain_->depth_image( ).format( );

    depth_prepass_pipeline_ = CVK.create_resource<GraphicsPipeline>( context_->device( ), info );
}


void MyApplication::create_main_render_pipeline( )
{
    // The compilation and linking of the SPIR-V bytecode to machine code for execution by the GPU doesn't happen until the
    // graphics pipeline is created. That means that we're allowed to destroy the shader modules again as soon as pipeline
    // creation is finished. Therefore, we create them as local temporary variables.
    shader::ShaderModule const vert_shader{ context_->device( ), "shaders/geometry.vert.spv" };
    shader::ShaderModule const frag_shader{ context_->device( ), "shaders/geometry.frag.spv" };

    constexpr VkSpecializationMapEntry specialization_entry{
        .constantID = 0,
        .offset = 0,
        .size = sizeof( uint32_t ),
    };

    constexpr uint32_t texture_count{ 24u };

    VkSpecializationInfo specialization_info{
        .mapEntryCount = 1,
        .pMapEntries   = &specialization_entry,
        .dataSize      = sizeof( uint32_t ),
        .pData         = &texture_count
    };

    GraphicsPipelineCreateInfo info = make_graphics_pipeline_create_info(
        std::array<VkPipelineShaderStageCreateInfo, 2>{
            {
                {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .stage = VK_SHADER_STAGE_VERTEX_BIT,
                    .module = vert_shader.handle( ),
                    .pName = "main",
                },
                {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .module = frag_shader.handle( ),
                    .pName = "main",
                    .pSpecializationInfo = &specialization_info,
                }
            }
        } );

    // Set only depth test enable
    info.depth_stencil.depthTestEnable  = VK_TRUE;
    info.depth_stencil.depthWriteEnable = VK_FALSE;
    info.depth_stencil.depthCompareOp   = VK_COMPARE_OP_EQUAL,

            info.descriptor_set_layouts = { descriptor_allocator_->layout( ).handle( ) };
    info.swapchain_image_format         = swapchain_->image_format( );
    info.depth_image_format             = swapchain_->depth_image( ).format( );

    main_render_pipeline_ = CVK.create_resource<GraphicsPipeline>( context_->device( ), info );
}


void MyApplication::record_command_buffer( CommandBuffer const& buffer, Image& image, VkDescriptorSet const desc_set ) const
{
    buffer.reset( );
    auto const command_op = buffer.command_operator( 0 );

    // 1. We set the memory barriers for the swapchain image.
    {
        // Pre depth pass barrier
        swapchain_->depth_image( ).transition_layout(
            ImageLayoutTransition{ VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL }
            .from_stage( VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT )
            .to_stage( VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT )
            .from_access( VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT )
            .to_access( VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT ), command_op );

        // Post depth pass barrier
        swapchain_->depth_image( ).transition_layout(
            ImageLayoutTransition{ VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL }
            .from_stage( VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT )
            .to_stage( VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT )
            .from_access( VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT )
            .to_access( VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT ), command_op );

        // Pre color pass barrier
        image.transition_layout(
            ImageLayoutTransition{ VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }
            .from_stage( VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT )
            .to_stage( VK_PIPELINE_STAGE_2_TRANSFER_BIT )
            .from_access( VK_ACCESS_2_NONE )
            .to_access( VK_ACCESS_2_TRANSFER_WRITE_BIT ), command_op );

        // Post color pass barrier
        image.transition_layout(
            ImageLayoutTransition{ VK_IMAGE_LAYOUT_PRESENT_SRC_KHR }
            .from_stage( VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT )
            .to_stage( VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT )
            .from_access( VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT )
            .to_access( VK_ACCESS_2_NONE ), command_op );
    }

    // 2. Depth Pre-Pass: render geometry to depth only, no color attachment
    {
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
            command_op.push_constants( main_render_pipeline_->layout( ), VK_SHADER_STAGE_FRAGMENT_BIT,
                                       0, sizeof( uint32_t ), &material_index );
            command_op.draw_indexed( index_count, 1, index_offset, vertex_offset );
        }

        command_op.end_rendering( );
    }

    // 3. Main render pass: color + depth read-only
    {
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

        VkRenderingAttachmentInfo const depth_attachment_info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = swapchain_->depth_image( ).view( ).handle( ),
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .clearValue = {
                .depthStencil = {
                    .depth = 1.0f
                }
            }
        };

        VkRenderingInfo const render_info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = {
                .offset = { 0, 0 },
                .extent = image.extent( ),
            },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attachment_info,
            .pDepthAttachment = &depth_attachment_info
        };

        command_op.begin_rendering( render_info );

        command_op.bind_pipeline( VK_PIPELINE_BIND_POINT_GRAPHICS, *main_render_pipeline_ );

        command_op.set_viewport( VkViewport{
            .x = 0.f, .y = 0.f,
            .width = static_cast<float>( image.extent( ).width ),
            .height = static_cast<float>( image.extent( ).height ),
            .minDepth = 0.f, .maxDepth = 1.f
        } );
        command_op.set_scissor( VkRect2D{ .offset = { 0, 0 }, .extent = image.extent( ) } );

        command_op.bind_vertex_buffers( model_->vertex_buffer( ), 0 );
        command_op.bind_index_buffer( model_->index_buffer( ), 0 );

        command_op.bind_descriptor_set( VK_PIPELINE_BIND_POINT_GRAPHICS, *main_render_pipeline_, desc_set );

        for ( auto const& [index_count, index_offset, vertex_offset, material_index] : model_->meshes( ) )
        {
            command_op.push_constants( main_render_pipeline_->layout( ), VK_SHADER_STAGE_FRAGMENT_BIT,
                                       0, sizeof( uint32_t ), &material_index );
            command_op.draw_indexed( index_count, 1, index_offset, vertex_offset );
        }

        command_op.end_rendering( );
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


GraphicsPipelineCreateInfo make_graphics_pipeline_create_info( std::span<VkPipelineShaderStageCreateInfo const> shaders )
{
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
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.0f,
    };

    // The VkPipelineMultisampleStateCreateInfo struct configures multisampling, which is one of the ways to perform
    // antialiasing.
    constexpr VkPipelineMultisampleStateCreateInfo multisampling{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
    };

    // After a fragment shader has returned a color, it needs to be combined with the color that is already in the
    // framebuffer. This transformation is known as color blending and there are two ways to do it:
    // 1. Mix the old and new value to produce a final color.
    // 2. Combine the old and new value using a bitwise operation.
    constexpr VkPipelineColorBlendAttachmentState color_blend_attachment{
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                          VK_COLOR_COMPONENT_A_BIT,
    };

    // Depth stencil creation
    constexpr VkPipelineDepthStencilStateCreateInfo depth_stencil{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
    };

    return GraphicsPipelineCreateInfo{
        .shader_stages = { shaders.begin( ), shaders.end( ) },
        .push_constant_ranges = {
            VkPushConstantRange{
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                .offset = 0,
                .size = sizeof( uint32_t )
            }
        },
        .binding_description = { Vertex::get_binding_description( ), Vertex::get_attribute_descriptions( ) },
        .input_assembly = input_assembly,
        .rasterization = rasterization,
        .multisampling = multisampling,
        .color_blend_attachment = color_blend_attachment,
        .depth_stencil = depth_stencil
    };
}
