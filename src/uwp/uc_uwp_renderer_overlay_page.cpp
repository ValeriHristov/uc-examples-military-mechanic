#include "pch.h"
#include "uc_uwp_renderer_overlay_page.h"

namespace uc
{
    namespace uwp
    {
        namespace overlay
        {
            page::page(gxu::initialize_context* )
            {

            }

            page::~page()
            {

            }

            void page::update(gxu::update_context* ctx)
            {
                do_update(ctx);
            }
            
            std::unique_ptr< submitable >  page::render(gxu::render_context* ctx )
            {
                return do_render(ctx);
            }

        }
    }
}