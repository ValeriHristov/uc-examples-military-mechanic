#include "pch.h"
#include "uc_uwp_renderer_impl.h"

#include <array>

#include "uc_uwp_renderer_impl_window.h"

#include <ppl.h>
#include "uc_uwp_ui_helper.h"

namespace uc
{
    namespace uwp
    {
        renderer_impl::renderer_impl( bool* window_close, const winrt::Windows::UI::Core::CoreWindow& window, const winrt::Windows::Graphics::Display::DisplayInformation& display_information) : m_main_window(window_close)
        {
            set_window(window);
            set_display_info(display_information);
            
            m_resources.add_swapchain(swap_chain::swap_chain_type::background_core_window);
            m_resources.add_swapchain(swap_chain::swap_chain_type::foreground_core_window);
            

            gx::geo::geometry_allocator_options o = {};
            o.m_index_count = 1000000;
            o.m_skinned_mesh_vertex_count = 1000000;
            o.m_static_mesh_vertex_count = 1000000;
            o.m_normal_mesh_vertex_count = 1000000;

            m_geometry_allocator            = std::make_unique<gx::geo::geometry_allocator>(m_resources.resource_create_context(), o);
        }

        void renderer_impl::initialize_resources()
        {
            using namespace gx::dx12;
            

            //flush all uploaded resources previous frame
            m_resources.direct_queue(device_resources::swap_chains::background )->insert_wait_on(m_resources.upload_queue()->flush());
            m_resources.direct_queue(device_resources::swap_chains::background )->insert_wait_on(m_resources.compute_queue()->signal_fence());
        }

        static inline uint64_t next_frame(uint64_t frame)
        {
            uint64_t r = frame + 1;
            return r % 3;
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
            using namespace gx::dx12;

            concurrency::task_group g;

            //flush all uploaded resources previous frame
            //make sure the gpu waits for the newly uploaded resources if any
            //flush the previous
            m_resources.direct_queue(device_resources::swap_chains::background)->pix_begin_event(L"Frame");
            m_resources.direct_queue(device_resources::swap_chains::background)->insert_wait_on(m_resources.upload_queue()->flush());
            m_resources.direct_queue(device_resources::swap_chains::background)->insert_wait_on(m_resources.compute_queue()->signal_fence());

            m_resources.direct_queue(device_resources::swap_chains::overlay)->insert_wait_on(m_resources.upload_queue()->flush());
            m_resources.direct_queue(device_resources::swap_chains::background)->insert_wait_on(m_resources.compute_queue()->signal_fence());

            m_frame_index += 1;
            m_frame_index %= 3;

            auto&& back_buffer = m_resources.back_buffer(device_resources::swap_chains::background);
            auto graphics      = create_graphics_command_context(m_resources.direct_command_context_allocator(device_resources::swap_chains::background));

            graphics->clear(back_buffer);


            graphics->submit();

            //if we did upload through the pci bus, insert waits
            m_resources.direct_queue(device_resources::swap_chains::background)->insert_wait_on(m_resources.upload_queue()->flush());
            m_resources.direct_queue(device_resources::swap_chains::background)->insert_wait_on(m_resources.compute_queue()->signal_fence());

            m_resources.direct_queue(device_resources::swap_chains::overlay)->insert_wait_on(m_resources.upload_queue()->flush());
            m_resources.direct_queue(device_resources::swap_chains::background)->insert_wait_on(m_resources.compute_queue()->signal_fence());

            m_resources.direct_queue(device_resources::swap_chains::background)->pix_end_event();
        }

        void renderer_impl::present()
        {
            m_resources.present();
            m_resources.move_to_next_frame();
            m_resources.sync();
            m_geometry_allocator->sync();
        }

        void renderer_impl::resize()
        {

        }

    }
}