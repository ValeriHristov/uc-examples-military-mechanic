#pragma once

#include <cstdint>

#include <uc/io/pad.h>
#include <uc/io/mouse.h>
#include <uc/io/keyboard.h>



namespace uc
{
    namespace gx
    {
        namespace dx12
        {
            class gpu_depth_buffer;
            class gpu_msaa_depth_buffer;
            class gpu_color_buffer;
            struct gpu_graphics_command_context;
        }

        namespace geo
        {
            class geometry_allocator;
        }
    }

    namespace uwp
    {
        class device_resources;

        namespace gxu
        {
            struct size
            {
                uint16_t m_width;
                uint16_t m_height;
            };

            struct render_context
            {
                device_resources*                   m_resources;

                size                                m_back_buffer_size;
                size                                m_back_buffer_scaled_size;

                size                                m_front_buffer_size;
                double                              m_frame_time;
            };

            struct shadow_render_context : public render_context
            {
            
            };

            struct update_context
            {
                double              m_frame_time;
                
                device_resources*   m_resources;

                io::pad_state       m_pad_state;
                io::mouse_state     m_mouse_state;
                io::keyboard_state  m_keyboard_state;

                size                m_back_buffer_size;
                size                m_front_buffer_size;

                bool*               m_window_close;
            };

            struct initialize_context
            {
                device_resources*                        m_resources;
                gx::dx12::gpu_graphics_command_context * m_upload_ctx;
            };
        }
    }

}