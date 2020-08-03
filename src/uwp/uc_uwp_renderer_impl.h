#pragma once

#include <concurrent_queue.h>
#include <winrt/windows.ui.xaml.h>
#include <winrt/windows.ui.xaml.controls.h>

#include <uc/sys/profile_timer.h>
#include <uc/lip/lip.h>
#include <uc/gx/lip/animation.h>

#include <uc/gx/dx12/dx12.h>
#include <uc/sys/profile_timer.h>

#include <uc/gx/geo/geometry_allocator.h>
#include <uc/gx/geo/indexed_geometry.h>
#include <uc/gxu/pinhole_camera_dispatcher.h>

#include <uc/io/pad.h>
#include <uc/io/mouse.h>
#include <uc/io/keyboard.h>

#include <soldier_graphics.h>
#include <soldier_graphics_depth.h>
#include <soldier_graphics_simple.h>
#include <shaders/interop.h>


#include "uc_uwp_device_resources.h"
#include "uc_uwp_renderer_impl_command.h"

#include "skeleton_instance.h"
#include "animation_instance.h"

struct ISwapChainPanelNative;

namespace uc
{
    namespace gx
    {
        namespace anm
        {
            class skeleton_instance;
            class animation_instance;
        }
    }

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
                interop::offset		m_blend_weights;
                interop::offset		m_blend_indices;

                uint32_t			m_indices_size;
                uint32_t			m_vertex_count;
            };

            struct gpu_primitive_range
            {
                uint32_t m_begin;
                uint32_t m_end;

                uint32_t index_count() const { return m_end - m_begin; }
            };

            struct gpu_mesh_textured_opaque 
            {
                std::vector<gx::dx12::managed_gpu_texture_2d>	                                m_opaque_textures;
                std::vector< gpu_primitive_range >				                                m_opaque_ranges;
            };

            struct gpu_multi_material_mesh
            {
                std::vector< gpu_primitive_range >				                                m_opaque_ranges;
                std::vector< math::float4 >				                                        m_opaque_colors;
            };

            device_resources                                                                    m_resources;

            uint32_t                                                                            m_frame_index = 2;

            std::unique_ptr<gx::dx12::gpu_frame_depth_buffer>                                   m_frame_depth_buffer[3];
            std::unique_ptr<gx::dx12::gpu_frame_msaa_depth_buffer>                              m_frame_shadow_buffer[3];
            std::unique_ptr<gx::dx12::gpu_frame_color_buffer>                                   m_frame_shadow_map[3];

            window_environment                                                                  m_window_enviroment;
            winrt::Windows::UI::Core::CoreWindow                                                m_window = nullptr;
            winrt::Windows::Graphics::Display::DisplayInformation                               m_display_information = nullptr;
            
            gx::dx12::managed_gpu_byteaddress_buffer                                            m_geometry;

            gpu_mesh                                                                            m_mesh;
            gpu_mesh_textured_opaque                                                            m_mesh_opaque;

            gx::dx12::managed_gpu_byteaddress_buffer                                            m_geometry2;

            gpu_mesh                                                                            m_mesh2;
            gpu_multi_material_mesh                                                             m_mesh2_opaque;
            
            io::pad                                                                             m_pad;
            io::pad_state                                                                       m_pad_state;

            io::mouse                                                                           m_mouse;
            io::mouse_state                                                                     m_mouse_state;
            io::keyboard                                                                        m_keyboard;
            io::keyboard_state                                                                  m_keyboard_state;

            gx::dx12::soldier_graphics::graphics_pipeline_state*								m_soldier       		= nullptr;
            gx::dx12::soldier_graphics_depth::graphics_pipeline_state*							m_soldier_depth	        = nullptr;
            gx::dx12::soldier_graphics_simple::graphics_pipeline_state*                         m_soldier_simple        = nullptr;

            float                                                                               m_scale_render = 1.0f;

            sys::profile_timer                                                                  m_frame_timer;
            double                                                                              m_frame_time;

            bool*                                                                               m_main_window;
            Concurrency::concurrent_queue < resize_window_command >                             m_prerender_queue;
            void                                                                                flush_prerender_queue();


            lip::unique_lip_pointer<lip::skeleton>                                              m_military_mechanic_skeleton;
            lip::unique_lip_pointer<lip::joint_animations>                                      m_military_mechanic_animations;

            std::unique_ptr< gx::anm::skeleton_instance >                                       m_skeleton_instance;
            std::unique_ptr< gx::anm::animation_instance>                                       m_animation_instance;


            //update state
            math::managed_float4x4                                                              m_military_mechanic_transform = math::make_float4x4();

        };
    }

}