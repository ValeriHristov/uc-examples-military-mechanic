#include "pch.h"
#pragma optimize("",off)
#include "uc_uwp_renderer_impl.h"

#include <array>

#include "uc_uwp_renderer_impl_window.h"

#include <ppl.h>
#include "uc_uwp_ui_helper.h"

#define UWP

#include <uc/gx/lip/lip.h>
#include <uc/gx/lip_utils.h>

#include "animation_instance.h"
#include "transforms.h"

#include "uwp/file.h"

#include "uc_uwp_gx_render_context.h"

namespace
{
    size_t align(size_t v, size_t a)
    {
        return (v + a - 1) & (~(a - 1));
    }
}

namespace uc
{
    namespace uwp
    {
        using namespace winrt::Windows::ApplicationModel;
        using namespace winrt::Windows::ApplicationModel::Core;
        using namespace winrt::Windows::ApplicationModel::Activation;
        using namespace winrt::Windows::UI::Core;
        using namespace winrt::Windows::UI::ViewManagement;
        using namespace winrt::Windows::Graphics::Display;

        struct skinned_draw_constants
        {
            std::array<math::float4x4, 127> m_joints_palette;

            skinned_draw_constants()
            {
                for (auto&& i : m_joints_palette)
                {
                    i = math::identity_matrix();
                }
            }
        };

        renderer_impl::renderer_impl( bool* window_close, const winrt::Windows::UI::Core::CoreWindow& window, const winrt::Windows::Graphics::Display::DisplayInformation& display_information) : m_main_window(window_close)
        {
            set_window(window);
            set_display_info(display_information);
            
            m_resources.add_swapchain(swap_chain::swap_chain_type::background_core_window);
            m_resources.add_swapchain(swap_chain::swap_chain_type::foreground_core_window);
        }

        void renderer_impl::initialize_resources()
        {
            concurrency::task_group g;

            g.run([this]()
            {
                using namespace gx::dx12;

                gpu_resource_create_context* rc = m_resources.resource_create_context();
                gx::dx12::managed_graphics_command_context ctx = create_graphics_command_context(m_resources.direct_command_context_allocator(device_resources::swap_chains::background));

                {
                    const auto mesh = lip::create_from_compressed_lip_file<lip::derivatives_skinned_model>(L"Assets\\models\\military_mechanic.derivatives_skinned_model.model");
                    const auto pos = static_cast<uint32_t>(align(size(mesh->m_positions), 256U));
                    const auto uv = static_cast<uint32_t>(align(size(mesh->m_uv), 256U));
                    const auto normals = static_cast<uint32_t>(align(size(mesh->m_normals), 256UL));
                    const auto tangents = static_cast<uint32_t>(align(size(mesh->m_tangents), 256UL));
                    const auto indices = static_cast<uint32_t>(align(size(mesh->m_indices), 256UL));
                    const auto blend_weight_size = mesh->m_blend_weights.size() * sizeof(lip::float4);
                    const auto blend_indices_size = mesh->m_blend_indices.size() * sizeof(lip::ubyte4);

                    const auto blend_weights = static_cast<uint32_t>(align(blend_weight_size, 256UL));
                    const auto blend_indices = static_cast<uint32_t>(align(blend_indices_size, 256UL));

                    m_mesh_opaque.m_opaque_textures.resize(mesh->m_textures.size());
                    m_mesh_opaque.m_opaque_ranges.resize(mesh->m_primitive_ranges.size());

                    for (auto i = 0U; i < mesh->m_textures.size(); ++i)
                    {
                        const auto& texture = mesh->m_textures[i];

                        auto w = texture.m_levels[0].m_width;
                        auto h = texture.m_levels[0].m_height;

                        m_mesh_opaque.m_opaque_textures[i] = gx::dx12::create_texture_2d(rc, w, h, static_cast<DXGI_FORMAT>(texture.m_levels[0].view()), D3D12_RESOURCE_STATE_COPY_DEST);
                        D3D12_SUBRESOURCE_DATA s[1];
                        s[0] = gx::sub_resource_data(&texture.m_levels[0]);
                        ctx->upload_resource(m_mesh_opaque.m_opaque_textures[i].get(), 0, 1, &s[0]);
                    }

                    for (auto i = 0U; i < mesh->m_primitive_ranges.size(); ++i)
                    {
                        const auto& r = mesh->m_primitive_ranges[i];
                        m_mesh_opaque.m_opaque_ranges[i].m_begin = r.m_begin;
                        m_mesh_opaque.m_opaque_ranges[i].m_end = r.m_end;
                    }

                    auto s = static_cast<uint32_t> (pos + uv + normals + tangents + blend_weights + blend_indices + indices);
                    m_geometry = gx::dx12::create_byteaddress_buffer(rc, s, D3D12_RESOURCE_STATE_COPY_DEST);

                    //allocation
                    m_mesh.m_pos = 0;
                    m_mesh.m_uv = pos;
                    m_mesh.m_normals = pos + uv;
                    m_mesh.m_tangents = pos + uv + normals;
                    m_mesh.m_blend_weights = pos + uv + normals + tangents;
                    m_mesh.m_blend_indices = pos + uv + normals + tangents + blend_weights;
                    m_mesh.m_indices = pos + uv + normals + tangents + blend_weights + blend_indices;
                    m_mesh.m_indices_size = static_cast<uint32_t>(size(mesh->m_indices));
                    m_mesh.m_vertex_count = static_cast<uint32_t>(mesh->m_positions.size());

                    ctx->upload_buffer(m_geometry.get(), m_mesh.m_pos, mesh->m_positions.data(), size(mesh->m_positions));
                    ctx->upload_buffer(m_geometry.get(), m_mesh.m_uv, mesh->m_uv.data(), size(mesh->m_uv));
                    ctx->upload_buffer(m_geometry.get(), m_mesh.m_normals, mesh->m_normals.data(), size(mesh->m_normals));
                    ctx->upload_buffer(m_geometry.get(), m_mesh.m_tangents, mesh->m_tangents.data(), size(mesh->m_tangents));
                    ctx->upload_buffer(m_geometry.get(), m_mesh.m_blend_weights, mesh->m_blend_weights.data(), blend_weight_size);
                    ctx->upload_buffer(m_geometry.get(), m_mesh.m_blend_indices, mesh->m_blend_indices.data(), blend_indices_size);
                    ctx->upload_buffer(m_geometry.get(), m_mesh.m_indices, mesh->m_indices.data(), size(mesh->m_indices));

                    ctx->transition_resource(m_geometry.get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

                    for (auto&& t : m_mesh_opaque.m_opaque_textures)
                    {
                        ctx->transition_resource(t.get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                    }
                }

                {
                    const auto mesh = lip::create_from_compressed_lip_file<lip::derivatives_skinned_multi_material_model>(L"Assets\\models\\military_mechanic.derivatives_skinned_multi_material_model.model");
                    const auto pos = static_cast<uint32_t>(align(size(mesh->m_positions), 256U));
                    const auto uv = static_cast<uint32_t>(align(size(mesh->m_uv), 256U));
                    const auto normals = static_cast<uint32_t>(align(size(mesh->m_normals), 256UL));
                    const auto tangents = static_cast<uint32_t>(align(size(mesh->m_tangents), 256UL));
                    const auto indices = static_cast<uint32_t>(align(size(mesh->m_indices), 256UL));
                    const auto blend_weight_size = mesh->m_blend_weights.size() * sizeof(lip::float4);
                    const auto blend_indices_size = mesh->m_blend_indices.size() * sizeof(lip::ubyte4);

                    const auto blend_weights = static_cast<uint32_t>(align(blend_weight_size, 256UL));
                    const auto blend_indices = static_cast<uint32_t>(align(blend_indices_size, 256UL));

                    m_mesh2_opaque.m_opaque_ranges.resize(mesh->m_primitive_ranges.size());
                    m_mesh2_opaque.m_opaque_colors.resize(mesh->m_primitive_ranges.size());

                    for (auto i = 0U; i < mesh->m_primitive_ranges.size(); ++i)
                    {
                        const auto& r = mesh->m_primitive_ranges[i];
                        m_mesh2_opaque.m_opaque_ranges[i].m_begin = r.m_begin;
                        m_mesh2_opaque.m_opaque_ranges[i].m_end = r.m_end;
                    }

                    std::srand(35);

                    for (auto i = 0U; i < mesh->m_primitive_ranges.size(); ++i)
                    {
                        size_t r = std::rand() % 255;
                        size_t g = std::rand() % 255;
                        size_t b = 0;// std::rand() % 255;
                        m_mesh2_opaque.m_opaque_colors[i] = { r / 255.0f, g / 255.0f, b / 255.0f, 1.0f };
                    }

                    auto s = static_cast<uint32_t> (pos + uv + normals + tangents + blend_weights + blend_indices + indices);
                    m_geometry2 = gx::dx12::create_byteaddress_buffer(rc, s, D3D12_RESOURCE_STATE_COPY_DEST);

                    //allocation
                    m_mesh2.m_pos = 0;
                    m_mesh2.m_uv = pos;
                    m_mesh2.m_normals = pos + uv;
                    m_mesh2.m_tangents = pos + uv + normals;
                    m_mesh2.m_blend_weights = pos + uv + normals + tangents;
                    m_mesh2.m_blend_indices = pos + uv + normals + tangents + blend_weights;
                    m_mesh2.m_indices = pos + uv + normals + tangents + blend_weights + blend_indices;
                    m_mesh2.m_indices_size = static_cast<uint32_t>(size(mesh->m_indices));
                    m_mesh2.m_vertex_count = static_cast<uint32_t>(mesh->m_positions.size());

                    ctx->upload_buffer(m_geometry2.get(), m_mesh2.m_pos, mesh->m_positions.data(), size(mesh->m_positions));
                    ctx->upload_buffer(m_geometry2.get(), m_mesh2.m_uv, mesh->m_uv.data(), size(mesh->m_uv));
                    ctx->upload_buffer(m_geometry2.get(), m_mesh2.m_normals, mesh->m_normals.data(), size(mesh->m_normals));
                    ctx->upload_buffer(m_geometry2.get(), m_mesh2.m_tangents, mesh->m_tangents.data(), size(mesh->m_tangents));
                    ctx->upload_buffer(m_geometry2.get(), m_mesh2.m_blend_weights, mesh->m_blend_weights.data(), blend_weight_size);
                    ctx->upload_buffer(m_geometry2.get(), m_mesh2.m_blend_indices, mesh->m_blend_indices.data(), blend_indices_size);
                    ctx->upload_buffer(m_geometry2.get(), m_mesh2.m_indices, mesh->m_indices.data(), size(mesh->m_indices));

                    ctx->transition_resource(m_geometry2.get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
                }


                {
                    gxu::initialize_context ictx = { &m_resources, ctx.get() };

                    m_imgui_page = std::make_unique<imgui::options_page>(&ictx);

                }

                ctx->submit();

                //flush all uploaded resources previous frame
                m_resources.direct_queue(device_resources::swap_chains::background)->insert_wait_on(m_resources.upload_queue()->flush());
                m_resources.direct_queue(device_resources::swap_chains::background)->insert_wait_on(m_resources.compute_queue()->signal_fence());

                m_resources.wait_for_gpu();
            });

            g.run([this]()
            {
                using namespace gx::dx12;
                gpu_resource_create_context* rc = m_resources.resource_create_context();
                m_soldier = gx::dx12::soldier_graphics::create_pso(m_resources.device_d2d12(), rc->null_cbv(), rc->null_srv(), rc->null_uav(), rc->null_sampler());
            });

            g.run([this]()
            {
                using namespace gx::dx12;
                gpu_resource_create_context* rc = m_resources.resource_create_context();

                m_soldier_depth = gx::dx12::soldier_graphics_depth::create_pso(m_resources.device_d2d12(), rc->null_cbv(), rc->null_srv(), rc->null_uav(), rc->null_sampler());
            });

            g.run([this]()
            {
                using namespace gx::dx12;
                gpu_resource_create_context* rc = m_resources.resource_create_context();
                m_soldier_simple = gx::dx12::soldier_graphics_simple::create_pso(m_resources.device_d2d12(), rc->null_cbv(), rc->null_srv(), rc->null_uav(), rc->null_sampler());
            });


            //load preprocessed textured model
            g.run([this]()
            {
                m_military_mechanic_skeleton = lip::create_from_compressed_lip_file<lip::skeleton>(L"Assets\\skeletons\\military_mechanic.skeleton");
                m_skeleton_instance = std::make_unique<gx::anm::skeleton_instance>(m_military_mechanic_skeleton.get());
            });

            //load preprocessed textured model
            g.run_and_wait([this]()
            {
                m_military_mechanic_animations = lip::create_from_compressed_lip_file<lip::joint_animations>(L"Assets\\animations\\military_mechanic.animation");
            });


            m_animation_instance = std::make_unique<gx::anm::animation_instance>(m_military_mechanic_animations.get(), m_military_mechanic_skeleton.get());

            gx::pinhole_camera_helper::set_look_at(&m_camera, math::point3(10, 0, 10), math::vector3(0, 0, -5), math::vector3(0, 1, 0));
        }

        static inline uint64_t next_frame(uint64_t frame)
        {
            uint64_t r = frame + 1;
            return r % 2;
        }

        void renderer_impl::set_display_info(const winrt::Windows::Graphics::Display::DisplayInformation& display_information)
        {
            m_display_information = display_information;
        }

        void renderer_impl::set_window( const winrt::Windows::UI::Core::CoreWindow& window)
        {
            m_window = window;
        }

        void renderer_impl::set_swapchainpanel(const winrt::Windows::UI::Xaml::Controls::SwapChainPanel&)
        {

        }

        void renderer_impl::refresh_display_layout()
        {
            resize_window_command c;
            c.m_window_environment = build_environment(m_window, m_display_information);
            m_prerender_queue.push(std::move(c));
            //ui stuff
            m_mouse.set_window(m_window, c.m_window_environment.m_effective_dpi);
            m_keyboard.set_window(m_window);
        }

        renderer_impl::~renderer_impl()
        {
            ui_run_sync(m_window, [this]
            {
                m_mouse.release();
                m_keyboard.release();
            });

            m_resources.wait_for_gpu();
        }

        void renderer_impl::process_user_input()
        {
            m_pad_state         = m_pad.update(m_pad_state);
            m_mouse_state       = m_mouse.update(m_mouse_state);
            m_keyboard_state    = m_keyboard.update(m_keyboard_state);

            io::keyboard_state s = m_keyboard_state;
        }

        void renderer_impl::update()
        {
            m_frame_time = m_frame_timer.seconds();
            m_frame_timer.reset();

            process_user_input();
            update_camera();

            gxu::update_context uctx = {};

            uctx.m_frame_time = m_frame_time;
            uctx.m_resources  = &m_resources;
            uctx.m_pad_state   = m_pad_state;
            uctx.m_mouse_state = m_mouse_state;
            uctx.m_keyboard_state = m_keyboard_state;

			uctx.m_back_buffer_size.m_width = static_cast<uint16_t>(m_resources.back_buffer(device_resources::swap_chains::background)->width() * m_scale_render);
            uctx.m_back_buffer_size.m_height = static_cast<uint16_t>(m_resources.back_buffer(device_resources::swap_chains::background)->height() * m_scale_render);

            uctx.m_front_buffer_size.m_width = static_cast<uint16_t>(m_resources.back_buffer(device_resources::swap_chains::overlay)->width());
            uctx.m_front_buffer_size.m_height = static_cast<uint16_t>(m_resources.back_buffer(device_resources::swap_chains::overlay)->height());

            m_imgui_page->update(&uctx);
        }

        void renderer_impl::update_camera()
        {
            using namespace gx;
            using namespace uc::math;

            if (m_keyboard_state.is_button_down(io::keyboard_state::virtual_key_w))
            {
                pinhole_camera_helper::walk(&m_camera, (float)m_frame_time);
            }
            else if (m_keyboard_state.is_button_down(io::keyboard_state::virtual_key_s))
            {
                pinhole_camera_helper::walk(&m_camera, (float)-m_frame_time);
            }

            if (m_keyboard_state.is_button_down(io::keyboard_state::virtual_key_a))
            {
                pinhole_camera_helper::strafe(&m_camera, (float)m_frame_time);
            }
            else if (m_keyboard_state.is_button_down(io::keyboard_state::virtual_key_d))
            {
                pinhole_camera_helper::strafe(&m_camera, (float)-m_frame_time);
            }

            if (m_mouse_state.is_button_down(io::mouse_state::mouse_mask::middle))
            {
                 pinhole_camera_helper::pitch(&m_camera, -m_mouse_state.dy() / 2050.0f);
                 pinhole_camera_helper::yaw(&m_camera, m_mouse_state.dx() / 2050.0f);
            }
        }

        void renderer_impl::flush_prerender_queue()
        {
            resize_window_command c;
            while (m_prerender_queue.try_pop(c))
            {
                resize_buffers(&c.m_window_environment);
            }
        }

        void renderer_impl::resize_buffers( const window_environment* environment )
        {
            m_resources.wait_for_gpu();
            m_resources.set_window(environment);
            m_window = environment->m_window;
        }

        void renderer_impl::pre_render()
        {
            //skeleton of a render phases, which will get complicated over time
            flush_prerender_queue();
        }

        void renderer_impl::render()
        {
            using namespace gx;
            using namespace gx::dx12;

            m_frame_time = m_frame_timer.milliseconds();
            m_frame_timer.reset();

            m_skeleton_instance->reset();

            auto step = m_frame_time;

            if (m_keyboard_state.is_button_down(io::keyboard_state::virtual_key_x))
            {
                step = 0;
            }

            m_animation_instance->accumulate(m_skeleton_instance.get(), step);

            interop::skinned_draw_constants       constants_pass;
            interop::skinned_draw_pixel_constants material;

            {
                interop::skinned_draw_constants& draw = constants_pass;

                {
                    auto skeleton = m_military_mechanic_skeleton.get();
                    auto joints = gx::anm::local_to_world_joints2(skeleton, m_skeleton_instance->local_transforms());

                    //todo: avx2
                    for (auto i = 0U; i < joints.size(); ++i)
                    {
                        math::float4x4 bind_pose    = math::load44(&skeleton->m_joint_inverse_bind_pose2[i].m_a0);
                        math::float4x4 palette      = math::mul(bind_pose, joints[i]);
                        draw.m_joints_palette[i]    = math::transpose(palette);
                    }
                }
            }

            //flush all uploaded resources previous frame
            //make sure the gpu waits for the newly uploaded resources if any
            //flush the previous
            m_resources.direct_queue(device_resources::swap_chains::background)->pix_begin_event(L"Frame");

            m_resources.direct_queue(device_resources::swap_chains::background)->insert_wait_on(m_resources.upload_queue()->flush());
            m_resources.direct_queue(device_resources::swap_chains::background)->insert_wait_on(m_resources.compute_queue()->signal_fence());

            m_resources.direct_queue(device_resources::swap_chains::overlay)->insert_wait_on(m_resources.upload_queue()->flush());
            m_resources.direct_queue(device_resources::swap_chains::background)->insert_wait_on(m_resources.compute_queue()->signal_fence());

            m_frame_index += 1;
            m_frame_index %= 2;

            auto&& back_buffer  = m_resources.back_buffer(device_resources::swap_chains::background);
            auto w = back_buffer->width();
            auto h = back_buffer->height();

            auto&& depth_buffer = m_resources.resource_create_context()->create_frame_depth_buffer(w, h, DXGI_FORMAT_D32_FLOAT);
            auto graphics       = create_graphics_command_context(m_resources.direct_command_context_allocator(device_resources::swap_chains::background));


            auto perspective = perspective_matrix(m_camera);
            auto view		 = view_matrix(m_camera);
            auto world0		 = math::identity_matrix();
            auto world1      = math::translation(0, 0, 5);

            interop::frame f;

            f.m_perspective.m_value = transpose(perspective);
            f.m_view.m_value = transpose(view);
            
            graphics->transition_resource(back_buffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

            graphics->clear(back_buffer);
            graphics->clear(depth_buffer);

            graphics->set_render_target(depth_buffer);
            graphics->set_descriptor_heaps();

            graphics->set_view_port({ 0.0,0.0,static_cast<float>(w),static_cast<float>(h),0.0,1.0 });
            graphics->set_scissor_rectangle({ 0,0,(int32_t)w,(int32_t)(h) });
            graphics->set_primitive_topology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            //depth
            {
                graphics->set_pso(m_soldier_depth);
                graphics->set_graphics_root_constant(0, 1, offsetof(interop::draw_call, m_batch) / sizeof(uint32_t));
                graphics->set_graphics_root_constant(0, 0, offsetof(interop::draw_call, m_start_vertex) / sizeof(uint32_t));
                graphics->set_graphics_root_constant(0, m_mesh.m_blend_weights, offsetof(interop::draw_call, m_blend_weights) / sizeof(uint32_t));
                graphics->set_graphics_root_constant(0, m_mesh.m_blend_indices, offsetof(interop::draw_call, m_blend_indices) / sizeof(uint32_t));

                graphics->set_graphics_root_constant(0, m_mesh.m_pos, offsetof(interop::draw_call, m_position) / sizeof(uint32_t));
                

                graphics->set_graphics_constant_buffer(1, f);
                graphics->set_graphics_srv_buffer(2, m_geometry.get());
                graphics->set_graphics_constant_buffer(5, constants_pass);

                graphics->set_index_buffer({ m_geometry->virtual_address() + m_mesh.m_indices, m_mesh.m_indices_size, DXGI_FORMAT_R32_UINT });

                {
                    auto m = transpose(world0);
                    graphics->set_graphics_root_constants(0, sizeof(m) / sizeof(uint32_t), &m, offsetof(interop::draw_call, m_world) / sizeof(uint32_t));
                }
                graphics->draw_indexed(m_mesh.m_indices_size / 4);


                {
                    auto m = transpose(world1);
                    graphics->set_graphics_root_constants(0, sizeof(m) / sizeof(uint32_t), &m, offsetof(interop::draw_call, m_world) / sizeof(uint32_t));
                }
                graphics->draw_indexed(m_mesh.m_indices_size / 4);
            }

            //opaque
            {
                graphics->set_render_target(back_buffer, depth_buffer);
                graphics->set_pso(m_soldier);

                graphics->set_view_port({ 0.0,0.0,static_cast<float>(w),static_cast<float>(h),0.0,1.0 });
                graphics->set_scissor_rectangle({ 0,0,(int32_t)w,(int32_t)(h) });
                graphics->set_primitive_topology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

                graphics->set_graphics_root_constant(0, 1, offsetof(interop::draw_call, m_batch) / sizeof(uint32_t));
                graphics->set_graphics_root_constant(0, 0, offsetof(interop::draw_call, m_start_vertex) / sizeof(uint32_t));
                graphics->set_graphics_root_constant(0, m_mesh.m_blend_weights, offsetof(interop::draw_call, m_blend_weights) / sizeof(uint32_t));
                graphics->set_graphics_root_constant(0, m_mesh.m_blend_indices, offsetof(interop::draw_call, m_blend_indices) / sizeof(uint32_t));

                graphics->set_graphics_root_constant(0, m_mesh.m_pos, offsetof(interop::draw_call, m_position) / sizeof(uint32_t));
                graphics->set_graphics_root_constant(0, m_mesh.m_uv, offsetof(interop::draw_call, m_uv) / sizeof(uint32_t));
                graphics->set_graphics_root_constant(0, m_mesh.m_normals, offsetof(interop::draw_call, m_normal) / sizeof(uint32_t));
                graphics->set_graphics_root_constant(0, m_mesh.m_tangents, offsetof(interop::draw_call, m_tangent) / sizeof(uint32_t));
                
                graphics->set_graphics_constant_buffer(1, f);
                graphics->set_graphics_srv_buffer(2, m_geometry.get());
                graphics->set_graphics_constant_buffer(5, constants_pass);

                graphics->set_index_buffer({ m_geometry->virtual_address() + m_mesh.m_indices, m_mesh.m_indices_size, DXGI_FORMAT_R32_UINT });

                {
                    auto m = transpose(world0);
                    graphics->set_graphics_root_constants(0, sizeof(m) / sizeof(uint32_t), &m, offsetof(interop::draw_call, m_world) / sizeof(uint32_t));
                }

                for (auto i = 0U; i < m_mesh_opaque.m_opaque_textures.size(); ++i)
                {
                    graphics->set_graphics_dynamic_descriptor(4, m_mesh_opaque.m_opaque_textures[i]->srv());
                    graphics->draw_indexed(m_mesh_opaque.m_opaque_ranges[i].index_count(), m_mesh_opaque.m_opaque_ranges[i].m_begin);
                }
            }

            //opaque2
            {
                graphics->set_render_target(back_buffer, depth_buffer);
                graphics->set_pso(m_soldier_simple);

                graphics->set_view_port({ 0.0,0.0,static_cast<float>(w),static_cast<float>(h),0.0,1.0 });
                graphics->set_scissor_rectangle({ 0,0,(int32_t)w,(int32_t)(h) });
                graphics->set_primitive_topology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

                graphics->set_graphics_root_constant(0, 1, offsetof(interop::draw_call, m_batch) / sizeof(uint32_t));
                graphics->set_graphics_root_constant(0, 0, offsetof(interop::draw_call, m_start_vertex) / sizeof(uint32_t));
                graphics->set_graphics_root_constant(0, m_mesh2.m_blend_weights, offsetof(interop::draw_call, m_blend_weights) / sizeof(uint32_t));
                graphics->set_graphics_root_constant(0, m_mesh2.m_blend_indices, offsetof(interop::draw_call, m_blend_indices) / sizeof(uint32_t));

                graphics->set_graphics_root_constant(0, m_mesh2.m_pos, offsetof(interop::draw_call, m_position) / sizeof(uint32_t));
                graphics->set_graphics_root_constant(0, m_mesh2.m_uv, offsetof(interop::draw_call, m_uv) / sizeof(uint32_t));
                graphics->set_graphics_root_constant(0, m_mesh2.m_normals, offsetof(interop::draw_call, m_normal) / sizeof(uint32_t));
                graphics->set_graphics_root_constant(0, m_mesh2.m_tangents, offsetof(interop::draw_call, m_tangent) / sizeof(uint32_t));

                graphics->set_graphics_constant_buffer(1, f);
                graphics->set_graphics_srv_buffer(2, m_geometry2.get());
                graphics->set_graphics_constant_buffer(5, constants_pass);

                graphics->set_index_buffer({ m_geometry2->virtual_address() + m_mesh2.m_indices, m_mesh2.m_indices_size, DXGI_FORMAT_R32_UINT });

                {
                    auto m = transpose(world1);
                    graphics->set_graphics_root_constants(0, sizeof(m) / sizeof(uint32_t), &m, offsetof(interop::draw_call, m_world) / sizeof(uint32_t));
                }

                for (auto i = 0U; i < m_mesh_opaque.m_opaque_textures.size(); ++i)
                {
                    material.m_color = m_mesh2_opaque.m_opaque_colors[i];
                    graphics->set_graphics_dynamic_constant_buffer(3, 0, material);
                    graphics->draw_indexed(m_mesh2_opaque.m_opaque_ranges[i].index_count(), m_mesh2_opaque.m_opaque_ranges[i].m_begin);
                }
            }

            graphics->transition_resource(back_buffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);


            gxu::render_context rctx;

			rctx.m_back_buffer_size.m_width     = static_cast<uint16_t>(m_resources.back_buffer(device_resources::swap_chains::background)->width()  * m_scale_render);
			rctx.m_back_buffer_size.m_height    = static_cast<uint16_t>(m_resources.back_buffer(device_resources::swap_chains::background)->height() * m_scale_render);

			rctx.m_front_buffer_size.m_width    = static_cast<uint16_t>(m_resources.back_buffer(device_resources::swap_chains::overlay)->width());
			rctx.m_front_buffer_size.m_height   = static_cast<uint16_t>(m_resources.back_buffer(device_resources::swap_chains::overlay)->height());
            rctx.m_back_buffer_scaled_size      = rctx.m_back_buffer_size;
            rctx.m_frame_time = m_frame_time;
            rctx.m_resources = &m_resources;

            std::unique_ptr< submitable > submitable = m_imgui_page->render(&rctx);
            

			//if we did upload through the pci bus, insert waits
			m_resources.direct_queue(device_resources::swap_chains::background)->insert_wait_on(m_resources.upload_queue()->flush());
			m_resources.direct_queue(device_resources::swap_chains::background)->insert_wait_on(m_resources.compute_queue()->signal_fence());

			m_resources.direct_queue(device_resources::swap_chains::overlay)->insert_wait_on(m_resources.upload_queue()->flush());
			m_resources.direct_queue(device_resources::swap_chains::background)->insert_wait_on(m_resources.compute_queue()->signal_fence());

            graphics->submit();
            submitable->submit();

            m_resources.direct_queue(device_resources::swap_chains::background)->pix_end_event();
        }

        void renderer_impl::present()
        {
            m_resources.present(gx::dx12::gpu_command_queue::present_option::wait_for_vsync);
            m_resources.move_to_next_frame();
            m_resources.sync();
        }

        void renderer_impl::resize()
        {

        }
    }
}