#pragma once

#include <uc/gx/dx12/dx12.h>

#include "uc_uwp_renderer_overlay_page.h"

namespace uc
{
    namespace uwp
    {
        namespace imgui
        {
            class options_page : public overlay::page
            {
                using super = overlay::page;

                public:

                options_page(gxu::initialize_context* ctx);
                ~options_page();

                private:

                void do_update(gxu::update_context* ctx) override;

                std::unique_ptr< submitable > do_render(gxu::render_context* ctx) override;

                gx::dx12::managed_gpu_texture_2d        m_font;
                gx::dx12::graphics_pipeline_state*      m_imgui_pso;
            };
        }
    }
    
}