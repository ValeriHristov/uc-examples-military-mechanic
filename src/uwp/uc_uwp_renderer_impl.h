#pragma once

#include <concurrent_queue.h>
#include <winrt/windows.ui.xaml.h>
#include <winrt/windows.ui.xaml.controls.h>

#include <uc/gx/dx12/dx12.h>
#include <uc/sys/profile_timer.h>

#include <uc/gx/geo/geometry_allocator.h>
#include <uc/gx/geo/indexed_geometry.h>
#include <uc/gxu/pinhole_camera_dispatcher.h>

#include <uc/io/pad.h>
#include <uc/io/mouse.h>
#include <uc/io/keyboard.h>

#include "uc_uwp_device_resources.h"
#include "uc_uwp_renderer_impl_command.h"

#include <uc/gx/geo/geometry_allocator.h>


#include <solid_graphics.h>
#include <solid_graphics_depth.h>

#include <shaders/interop.h>


struct ISwapChainPanelNative;

namespace uc
{
    namespace uwp
    {
        class renderer_impl : public util::noncopyable
        {
        public:

            renderer_impl(bool* window_close, const winrt::Windows::UI::Core::CoreWindow& window, const winrt::Windows::Graphics::Display::DisplayInformation& display_information);
            ~renderer_impl();

            void update();
            void pre_render();
            void render();
            void process_user_input();

            void present();
            void resize();

            void set_display_info(const winrt::Windows::Graphics::Display::DisplayInformation& display_information);
            void set_window(const winrt::Windows::UI::Core::CoreWindow& window);
            void set_swapchainpanel(const winrt::Windows::UI::Xaml::Controls::SwapChainPanel&  swapchainpanel);

            void refresh_display_layout();
            
            void resize_buffers(const window_environment* environment);

            void initialize_resources();

        private:

			struct gpu_mesh
			{
				interop::offset		m_pos;
				interop::offset		m_uv;
				interop::offset		m_normals;
				interop::offset		m_tangents;
				interop::offset		m_indices;
				uint32_t			m_indices_size;
				uint32_t			m_vertex_count;
			};

			struct gpu_primitive_range
			{
				uint32_t m_begin;
				uint32_t m_end;

				uint32_t index_count() const { return m_end - m_begin; }
			};

			struct gpu_mesh_opaque
			{
				std::vector<gx::dx12::managed_gpu_texture_2d>	m_opaque_textures;
				std::vector< gpu_primitive_range >				m_opaque_ranges;
			};

			device_resources                                                                    m_resources;

            uint32_t                                                                            m_frame_index = 2;

            std::unique_ptr<gx::dx12::gpu_frame_depth_buffer>                                   m_frame_depth_buffer[3];
            std::unique_ptr<gx::dx12::gpu_frame_msaa_depth_buffer>                              m_frame_shadow_buffer[3];
            std::unique_ptr<gx::dx12::gpu_frame_color_buffer>                                   m_frame_shadow_map[3];

            window_environment                                                                  m_window_enviroment;
            winrt::Windows::UI::Core::CoreWindow                                                m_window = nullptr;
            winrt::Windows::Graphics::Display::DisplayInformation                               m_display_information = nullptr;
			
			gx::dx12::managed_byteaddress_gpu_buffer											m_geometry;

			gpu_mesh																			m_mesh;
			gpu_mesh_opaque																		m_mesh_opaque;
			

            io::pad                                                                             m_pad;
            io::pad_state                                                                       m_pad_state;

            io::mouse                                                                           m_mouse;
            io::mouse_state                                                                     m_mouse_state;
            io::keyboard                                                                        m_keyboard;
            io::keyboard_state                                                                  m_keyboard_state;

			gx::dx12::solid_graphics::graphics_pipeline_state*									m_solid_graphics		= nullptr;
			gx::dx12::solid_graphics_depth::graphics_pipeline_state*							m_solid_graphics_depth	= nullptr;

            float                                                                               m_scale_render = 1.0f;

            sys::profile_timer                                                                  m_frame_timer;
            double                                                                              m_frame_time;

            bool*                                                                               m_main_window;
            Concurrency::concurrent_queue < resize_window_command >                             m_prerender_queue;
            void                                                                                flush_prerender_queue();
        };
    }

}